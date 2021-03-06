/*
 * Ramdisk module
 */
#include <linux/module.h>
#include <linux/proc_fs.h> /* We are making a procfs entry */
#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/sched.h> /* Get current */
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <asm/atomic.h>
#include <linux/errno.h> /* error codes */
#include <asm/uaccess.h> /* gives us get/put_user functions */
#include "ramdisk_module.h"
#include "data_structures.h"

MODULE_LICENSE("GPL");

// gobal declarations of ramdisk functions
static int ramdisk_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static int rd_init(void);
static bool rd_initialized(void);
static int create_file_descriptor_table(pid_t pid);
static file_descriptor_table_t *get_file_descriptor_table(pid_t pid);
static void delete_file_descriptor_table(pid_t pid);
static int create_file_descriptor_table_entry(file_descriptor_table_t *fdt, file_object_t fo);
static file_object_t get_file_descriptor_table_entry(file_descriptor_table_t *fdt, unsigned short fd);
static int set_file_descriptor_table_entry(file_descriptor_table_t *fdt, unsigned short fd, file_object_t fo);
static int delete_file_descriptor_table_entry(file_descriptor_table_t *fdt, unsigned short fd);
static size_t get_file_descriptor_table_size(file_descriptor_table_t *fdt, unsigned short fd);
static index_node_t *get_free_index_node(void);
static index_node_t *get_readlocked_parent_index_node(const char *pathname); // DOESNT TRASH PATHNAME
static index_node_t *get_readlocked_index_node(const char *pathname);
static index_node_t *get_inode(size_t no);
static void *extend_inode(index_node_t *inode);
static void *get_free_data_block(void);
static void release_data_block(void *data_block_ptr);
static directory_entry_t *get_directory_entry(index_node_t *inode, int index);
static void *get_byte_address(index_node_t *inode, int offset);
static int rd_creat(const char *usr_str);
static int rd_mkdir(const char *usr_str);
static int rd_open(const pid_t pid, const char *usr_str);
static int rd_close(const pid_t pid, const int fd);
static int rd_read(const pid_t pid, const rd_rwfile_arg_t *usr_arg);
static int rd_write(const pid_t pid, const rd_rwfile_arg_t *usr_arg);
static int rd_lseek(const pid_t pid, const rd_seek_arg_t *usr_arg);
static int rd_unlink(const char *usr_str);
static int rd_readdir(const pid_t pid, const rd_readdir_arg_t *usr_arg);
static int procfs_open(struct inode *inode, struct file *file);
static int procfs_close(struct inode *inode, struct file *file);

static struct file_operations ramdisk_file_ops = {
        .owner = THIS_MODULE,
        .read = NULL,
        .write = NULL,
        .open = procfs_open,
        .release = procfs_close,
};
static struct proc_dir_entry *proc_entry;

// declarations of ramdisk synchronization
// define locks to ensure consistency of ramdisk memory for multi processes access
DEFINE_RWLOCK(rd_init_rwlock);
DEFINE_SPINLOCK(super_block_spinlock);
DEFINE_SPINLOCK(block_bitmap_spinlock);
DEFINE_RWLOCK(index_nodes_rwlock);
DEFINE_RWLOCK(file_descriptor_tables_rwlock);

// declarations of ramdisk data structures
static bool rd_initialized_flag = false;
static super_block_t *super_block = NULL;
static index_node_t *index_nodes = NULL;    // 256 blocks/64 bytes per inode = 1024 inodes
static void *block_bitmap = NULL; // 4 blocks => block_bitmap is 1024 bytes long
static void *data_blocks = NULL; // len(data_blocks) == 7931 blocks
static int temp = 0;
static LIST_HEAD(file_descriptor_tables);

#define INODE_PTR(index) (index_node_t *) (((void *) index_nodes) + index * INDEX_NODE_SIZE)
#define BLOCK_START(byte_address) ((void *)byte_address - (((unsigned long) ((void *)byte_address - data_blocks)) % BLOCK_SIZE))
#define BLOCK_END(byte_address) (BLOCK_START(byte_address) + BLOCK_SIZE)

/*
 *
 *  setting up the /proc file system entry
 *
 *
 */
// increment usage count on /proc/ramdisk file open
static int procfs_open(struct inode *inode, struct file *file) {
    try_module_get(THIS_MODULE);
    return 0;
}

// decrement usage count on /proc/ramdisk file close
static int procfs_close(struct inode *inode, struct file *file) {
    int i = 0;
    file_descriptor_table_t *fdt = NULL;
    file_object_t fo;
    fdt = get_file_descriptor_table(current->pid);
    // assume that the no other thread will be accessing this fdt
    if (fdt != NULL) {
        for (i = 0; i < fdt->entries_length; i++) {
            fo = get_file_descriptor_table_entry(fdt, i);
            if (fo.index_node != NULL) {
                printk("Closing open file with fd %d on behalf of %d\n", i, current->pid);
                atomic_dec(&fo.index_node->open_count);
            }
        }
        delete_file_descriptor_table(current->pid);
    }
    printk("Num data_blocks remaining: %d\n", super_block->num_free_blocks);
    printk("Num inodes remaining: %d\n", super_block->num_free_inodes);
    module_put(THIS_MODULE);
    return 0;
}

static int __init initialization_routine(void) {
    printk(KERN_INFO "Loading ramdisk module\n");
    ramdisk_file_ops.ioctl = ramdisk_ioctl;
    // start create proc entry
    proc_entry = create_proc_entry("ramdisk", 0444, NULL);
    if (!proc_entry) {
        printk(KERN_ERR "Error creating /proc entry. \n");
        return 1;
    }
    proc_entry->proc_fops = &ramdisk_file_ops;
    return 0;
}

static void __exit cleanup_routine(void) {
    file_descriptor_table_t *p = NULL, *next = NULL;
    remove_proc_entry("ramdisk", NULL);
    printk(KERN_INFO "Cleaning up ramdisk module\n");
    list_for_each_entry_safe(p, next, &file_descriptor_tables, list){
        delete_file_descriptor_table(p->owner);
    }
    if (super_block != NULL) {
        printk(KERN_INFO "Freeing ramdisk memory\n");
        vfree(super_block);
    }
    return;
}


// ioctl() entry point
static int ramdisk_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg) {
    printk(KERN_INFO "Called ioctl\n");
    if (cmd != RD_INIT && !rd_initialized()) {
        printk(KERN_ERR "Ramdisk called before being initialized\n");
        return -1;
    }
    switch (cmd) {
        case RD_INIT:
            rd_init();
            break;
        case RD_CREAT:
            return rd_creat((char *) arg);
        case RD_MKDIR:
            return rd_mkdir((char *) arg);
        case RD_OPEN:
            return rd_open(current->pid, (char *) arg);
        case RD_CLOSE:
            return rd_close(current->pid, (int) arg);
        case RD_READ:
            return rd_read(current->pid, (rd_rwfile_arg_t *) arg);
        case RD_WRITE:
            return rd_write(current->pid, (rd_rwfile_arg_t *) arg);
        case RD_LSEEK:
            return rd_lseek(current->pid, (rd_seek_arg_t *) arg);
        case RD_UNLINK:
            return rd_unlink((char *) arg);
        case RD_READDIR:
            return rd_readdir(current->pid, (rd_readdir_arg_t *) arg);
        default:
            printk("Unrecognized cmd %u\n", cmd);
            return -EINVAL;
    }
    return 0;
}

/*
 *
 *  functions for working with ramdisk data structures
 *
 *
 */
// FDT functions
// Create a file descriptor table for the process identified by pid
static int create_file_descriptor_table(pid_t pid) {
    file_descriptor_table_t *fdt = NULL;
    file_object_t *entries = NULL;
    size_t init_num_entry_bytes = sizeof(file_object_t) * INIT_FDT_LEN;

    // Check if a file descriptor table for this process already exists
    if (get_file_descriptor_table(pid) != NULL) {
        printk(KERN_ERR "FDT for process %d already existed\n", pid);
        return -EEXIST;
    }

    // Allocate memory for the new file descriptor table, return on failure
    fdt = (file_descriptor_table_t *) kmalloc(sizeof(file_descriptor_table_t), GFP_KERNEL);
    if (fdt == NULL) {
        printk(KERN_ERR "failed to allocate FDT for process %d\n", pid);
        return -ENOMEM;
    }
    entries = (file_object_t *) kmalloc(init_num_entry_bytes, GFP_KERNEL);
    if (entries == NULL) {
        printk(KERN_ERR "failed to allocate entries array for FDT for process %d\n", pid);
        kfree(fdt);
        return -ENOMEM;
    }
    memset(entries, 0, init_num_entry_bytes);

    // Initialize new file descriptor table
    fdt->owner = pid;
    fdt->entries = entries;
    fdt->entries_length = INIT_FDT_LEN;
    fdt->num_free_entries = INIT_FDT_LEN;

    // Insert new FDT into file_descriptor_tables_list
    write_lock(&file_descriptor_tables_rwlock);
    list_add(&fdt->list, &file_descriptor_tables);
    write_unlock(&file_descriptor_tables_rwlock);
    return fdt;
}

// get a pointer to the file descriptor table owned associated with pid
static file_descriptor_table_t *get_file_descriptor_table(pid_t pid) {
    file_descriptor_table_t *p = NULL, *target = NULL;
    read_lock(&file_descriptor_tables_rwlock);
    //iterate the FDT list to find the process owning the current FDT
    list_for_each_entry(p, &file_descriptor_tables, list){
        if (p != NULL && p->owner == pid) {
            target = p;
            break;
        }
    }
    read_unlock(&file_descriptor_tables_rwlock);
    return target;
}


// removes the file descripor table associated with pid
static void delete_file_descriptor_table(pid_t pid) {
    file_descriptor_table_t *fdt = get_file_descriptor_table(pid);
    if (fdt == NULL) {
        printk(KERN_ERR "remove non-existant FDT for process %d\n", pid);
        return;
    }
    // remove fdt from list
    write_lock(&file_descriptor_tables_rwlock);
    list_del(&fdt->list);
    write_unlock(&file_descriptor_tables_rwlock);
    kfree(fdt->entries);
    kfree(fdt);
}


/*
 * Given a pointer to a process' file descriptor table, adds the given
 * file object to the table, and returns the file descriptor corresponding
 * to this new entry, or -errno on error
 */
static int create_file_descriptor_table_entry(file_descriptor_table_t *fdt, file_object_t fo) {
    int entry_index = 0;
    file_object_t *p = NULL, *dest = NULL;
    // Check if we need to allocate larger array/copy over current array
    if (fdt->num_free_entries <= 0)
        return -ENOMEM;
    // Search for empty entry in array, assumes that all empty entries are null
    for (entry_index = 0; entry_index < fdt->entries_length; entry_index++) {
        p = (fdt->entries) + entry_index;
        if (p->index_node == NULL) {
            dest = p;
            break;
        }
    }
    if (dest == NULL)
        return -ENOMEM;
    dest->index_node = fo.index_node;
    dest->file_position = fo.file_position;
    return entry_index;
}

/*
 * Returns the file_object associated with the given file descriptor in the given
 * file descriptor table. If the file descriptor is invalid, then a null file object is returned
 */
static file_object_t get_file_descriptor_table_entry(file_descriptor_table_t *fdt,
                                                     unsigned short fd) {
    file_object_t ret = {.index_node = NULL, .file_position = 0};
    if (fd >= (fdt->entries_length)) {
        return ret;
    }
    ret.index_node = fdt->entries[fd].index_node;
    ret.file_position = fdt->entries[fd].file_position;
    return ret;
}

/*
 * Sets the file descriptor table entry assocated with the given file descriptor
 * to the given file_object value.
 */
static int set_file_descriptor_table_entry(file_descriptor_table_t *fdt, unsigned short fd, file_object_t fo) {
    if (fdt->entries[fd].index_node == NULL) {
        return -EINVAL;
    }
    fdt->entries[fd] = fo;
    return 0;
}

// Deletes the file descriptor table entry assocated with the given file descriptor
static int delete_file_descriptor_table_entry(file_descriptor_table_t *fdt,
                                              unsigned short fd) {
    file_object_t null_file_object = {.index_node = NULL, .file_position = 0};
    return set_file_descriptor_table_entry(fdt, fd, null_file_object);
}

static size_t get_file_descriptor_table_size(file_descriptor_table_t *fdt, unsigned short fd) {
    size_t fdt_size;
    fdt_size = fdt->entries_length - fdt->entries_length;
    return fdt_size;
}

// Returns a pointer to a free index_node_t, if one exists, NULL on error
static index_node_t *get_free_index_node() {
    int i = 0, direct_ptr_index = 0;
    index_node_t *new_inode = NULL, *p = NULL;
    // make sure there is a free inode/ decrement inodes counter in superblock
    spin_lock(&super_block_spinlock);
    if (super_block->num_free_inodes == 0) {
        spin_unlock(&super_block_spinlock);
        return NULL;
    }
    super_block->num_free_inodes--;
    spin_unlock(&super_block_spinlock);
    // Look for an UNALLOCATED inode
    for (i = 0; i < INDEX_NODES; i++) {
        p = get_inode(i);
        if (write_trylock(&p->file_lock)) {
            if (p->type == UNALLOCATED) {
                new_inode = p;
                new_inode->type = ALLOCATED;
                new_inode->size = 0;
                atomic_set(&new_inode->open_count, 0);
                for (direct_ptr_index = 0; direct_ptr_index < DIRECT; direct_ptr_index++)
                    new_inode->direct[direct_ptr_index] = NULL;
                new_inode->single_indirect = NULL;
                new_inode->double_indirect = NULL;
                write_unlock(&new_inode->file_lock);
                break;
            } else {
                write_unlock(&p->file_lock);
            }
        }
    }
    return new_inode;
}


// Returns the index node of directory containing the file indicated by pathname, or NULL on error.
static index_node_t *get_readlocked_parent_index_node(const char *pathname) {
    char *pathname_copy = NULL, *filename = NULL;
    index_node_t *parent;
    filename = strrchr(pathname, '/');
    if (filename == NULL)
        return NULL;
    // if parent is root node
    if (strcmp(filename, pathname) == 0) {
        read_lock(&index_nodes->file_lock);
        return index_nodes;
    }
    pathname_copy = (char *) kcalloc(strlen(pathname) + 1, sizeof(char), GFP_KERNEL);
    strncpy(pathname_copy, pathname, strlen(pathname) - strlen(filename));
    parent = get_readlocked_index_node(pathname_copy);
    kfree(pathname_copy);
    return parent;
}

static index_node_t *get_readlocked_index_node(const char *pathname) {
    char *pathname_copy, *token, *tokenize;
    index_node_t *curr = index_nodes, *prev = NULL;
    directory_entry_t *dir_entry = NULL;
    int i = 0;
    bool found_prev_inode = true;
    // start with index_nodes
    if (strlen(pathname) == 1 && pathname[0] != '/') {
        return NULL;
    }
    if (strlen(pathname) == 1 && pathname[0] == '/') {
        read_lock(&index_nodes->file_lock);
        return index_nodes;         // points to root index node
    }

    pathname_copy = (char *) kcalloc(strlen(pathname) + 1, sizeof(char), GFP_KERNEL);
    strncpy(pathname_copy, pathname, strlen(pathname));
    tokenize = pathname_copy + 1;   // skip the first forward slash

    read_lock(&curr->file_lock);
    while ((token = strsep(&tokenize, "/")) != NULL) {
        if (curr->type != DIR || !found_prev_inode) {
            break;
        }
        found_prev_inode = false;
        for (i = 0; i < curr->size / sizeof(directory_entry_t); i++) {
            dir_entry = get_byte_address(curr, i * sizeof(directory_entry_t));
            if (strncmp(dir_entry->filename, token, MAX_FILE_NAME_LEN) == 0) {
                found_prev_inode = true;
                prev = curr;
                curr = INODE_PTR(dir_entry->index_node_number);
                read_lock(&curr->file_lock);
                read_unlock(&prev->file_lock);
                break;
            }
        }
    }
    kfree(pathname_copy);
    if (!(token == NULL && found_prev_inode)) {
        read_unlock(&curr->file_lock);
        return NULL;
    } else
        return curr;
}

/*
  To be called only on behalf of processes that have already opened
  the index node corresponding to the given index (that is, the returned
  index node does not come read-locked
 */
static index_node_t *get_inode(size_t index) {
    return (index_node_t *) (((void *) index_nodes) + INDEX_NODE_SIZE * index);
}


// to be called with write lock held!
static void *extend_inode(index_node_t *inode) {
    void *extending_block;
    if (inode->size >= MAX_FILE_SIZE - BLOCK_SIZE + 1) {
        // there's no room for another block for this file
        return NULL;
    }

    // Get new data block to extend inode with
    extending_block = get_free_data_block();
    if (inode->size < DIRECT * BLOCK_SIZE) {
        // Can link to new block from one of the DIRECT pointers
        inode->direct[inode->size / BLOCK_SIZE] = extending_block;
    } else if (inode->size < BLOCK_SIZE * (DIRECT + POINTER_PER_BLOCK)) {
        // Can link to new block from one of the INDIRECT pointers
        if (inode->size == DIRECT * BLOCK_SIZE) {
            // Need to make the INDIRECT block
            indirect_block_t *indirect_block = get_free_data_block();
            if (indirect_block == NULL) {
                return NULL;
            }
            inode->single_indirect = indirect_block;
            indirect_block->data[0] = (void *) extending_block;
        } else {
            // Indirect block already exists
            inode->single_indirect->data[(inode->size / BLOCK_SIZE) - DIRECT] = (void *) extending_block;
        }
    } else {
        // Need to link to new block from an INDIRECT block, that is pointed to from the DOUBLE_INDIRECT block
        if (inode->size == BLOCK_SIZE * (DIRECT + POINTER_PER_BLOCK)) {
            // Need to create the DOUBLE INDIRECT block
            double_indirect_block_t *double_indirect_block = get_free_data_block();
            indirect_block_t *indirect_block = get_free_data_block();
            if (indirect_block == NULL || double_indirect_block == NULL) {
                if (indirect_block != NULL)
                    release_data_block(indirect_block);
                if (double_indirect_block != NULL)
                    release_data_block(double_indirect_block);
                release_data_block(extending_block);
                return NULL;
            }
            inode->double_indirect = double_indirect_block;
            double_indirect_block->indirect_blocks[0] = indirect_block;
            indirect_block->data[0] = (void *) extending_block;
        } else if ((inode->size - BLOCK_SIZE * (DIRECT + POINTER_PER_BLOCK)) % (POINTER_PER_BLOCK * BLOCK_SIZE) != 0) {
            // Can point to the new block from  a prexisting indirect block
            int indirect_block_index =
                    (inode->size - BLOCK_SIZE * (DIRECT + POINTER_PER_BLOCK)) / (POINTER_PER_BLOCK * BLOCK_SIZE);
            int index_in_indirect_block =
                    ((inode->size - BLOCK_SIZE * (DIRECT + POINTER_PER_BLOCK)) % (POINTER_PER_BLOCK * BLOCK_SIZE)) / BLOCK_SIZE;
            inode->double_indirect->indirect_blocks[indirect_block_index]
                    ->data[index_in_indirect_block] = (void *) extending_block;
        } else if ((inode->size - BLOCK_SIZE * (DIRECT + POINTER_PER_BLOCK)) % (POINTER_PER_BLOCK * BLOCK_SIZE) == 0) {
            // Need to create a new indirect block to point to the new block
            indirect_block_t *indirect_block = get_free_data_block();
            if (indirect_block == NULL) {
                release_data_block(extending_block);
                return NULL;
            }
            int indirect_block_index =
                    (inode->size - BLOCK_SIZE * (DIRECT + POINTER_PER_BLOCK)) / (POINTER_PER_BLOCK * BLOCK_SIZE);
            int index_in_indirect_block = 0;
            inode->double_indirect->indirect_blocks[indirect_block_index]
                    = indirect_block;
            inode->double_indirect->indirect_blocks[indirect_block_index]
                    ->data[index_in_indirect_block] = (void *) extending_block;
        } else {
            printk(KERN_ERR "Encountered unexpected case in extend_inode\n");
            return NULL;
        }
    }
    return extending_block;
}

// to be called with readlock held
static directory_entry_t *get_directory_entry(index_node_t *inode, int index) {
    if (inode->type != DIR || inode->size / DIR_ENTRY_SIZE <= index) {
        return NULL;
    }
    return (directory_entry_t *) get_byte_address(inode, index * sizeof(directory_entry_t));
}


// returns a pointer to a free data block, or NULL if one is not available
static void *get_free_data_block() {
    unsigned long block_num = 0;
    void *block_address = NULL;
    spin_lock(&super_block_spinlock);
    if (super_block->num_free_blocks == 0) {
        spin_unlock(&super_block_spinlock);
        return NULL;
    }
    super_block->num_free_blocks--;
    spin_unlock(&super_block_spinlock);
    spin_lock(&block_bitmap_spinlock);
    block_num = find_first_zero_bit(block_bitmap, BLOCK_BITMAPS * BLOCK_SIZE * 8);
    if (block_num == BLOCK_BITMAPS * BLOCK_SIZE * 8) {
        spin_unlock(&block_bitmap_spinlock);
        return NULL;
    }
    set_bit(block_num, block_bitmap);
    spin_unlock(&block_bitmap_spinlock);
    block_address = data_blocks + block_num * BLOCK_SIZE;
    memset(block_address, 0, BLOCK_SIZE);
    return block_address;
}

/*
 *  Frees the data block pointed to by data_block_ptr to be
 *  re-allocated. NEVER CALL THIS FUNCTION while holding
 *  super_block_spinlock OR block_bitmap_spinlock!
 */
static void release_data_block(void *data_block_ptr) {
    int block_num;
    if (data_block_ptr == NULL) {
        return;
    }
    block_num = (data_block_ptr - data_blocks) / BLOCK_SIZE;
    spin_lock(&super_block_spinlock);
    super_block->num_free_blocks++;
    spin_unlock(&super_block_spinlock);
    spin_lock(&block_bitmap_spinlock);
    clear_bit(block_num, block_bitmap);
    spin_unlock(&block_bitmap_spinlock);
    return;
}


/*
 *
 * Functions for implementing the ramdisk API
 *
 */

// returns a boolean value indicating whether ramdisk is ready for use
bool rd_initialized() {
    bool ret;
    read_lock(&rd_init_rwlock);
    ret = rd_initialized_flag;
    read_unlock(&rd_init_rwlock);
    return ret;
}

// Initializaton routine must be called once to initialize ramdisk memory before other functions are called
int rd_init() {
    const super_block_t init_super_block = {.num_free_blocks = BLOCK_DATA,
            .num_free_inodes = BLOCK_INDEX_NODES * BLOCK_SIZE / INDEX_NODE_SIZE - 1};
    const index_node_t root_inode = {.type = DIR,
            .size = 0,
            .open_count = ATOMIC_INIT(0),
            .file_lock = RW_LOCK_UNLOCKED,
            .direct = {NULL},
            .single_indirect = NULL,
            .double_indirect = NULL};
    const index_node_t regular_inode = {.type = UNALLOCATED,
            .size = 0,
            .open_count = ATOMIC_INIT(0),
            .file_lock = RW_LOCK_UNLOCKED,
            .direct = {NULL},
            .single_indirect = NULL,
            .double_indirect = NULL};
    int i = 0;
    index_node_t *inode = NULL;
    if (rd_initialized()) {
        return -EALREADY;
    }
    write_lock(&rd_init_rwlock);
    printk(KERN_INFO "Initializing ramdisk\n");
    super_block = (super_block_t *) vmalloc(RD_SIZE);
    if (!super_block) {
        printk(KERN_ERR "vmalloc for ramdisk space failed\n");
        write_unlock(&rd_init_rwlock);
        return -ENOMEM;
    }
    memset((void *) super_block, 0, RD_SIZE);
    index_nodes = (index_node_t *) ((void *) super_block + BLOCK_SIZE);
    block_bitmap = ((void *) index_nodes + BLOCK_INDEX_NODES * BLOCK_SIZE);
    data_blocks = block_bitmap + BLOCK_BITMAPS * BLOCK_SIZE;
    *super_block = init_super_block;
    rd_initialized_flag = true;
    index_nodes[0] = root_inode;
    for (i = 1; i < INDEX_NODES; i++) {
        inode = get_inode(i);
        *inode = regular_inode;
    }
    write_unlock(&rd_init_rwlock);
    printk("Num data_block at init: %d\n", super_block->num_free_blocks);
    printk("Num inodes at init: %d\n", super_block->num_free_inodes);
    return 0;
}


// Returns the address of the offset bytes of the data associated with inode or NULL on error. To be called with readlock held
static void *get_byte_address(index_node_t *inode, int offset) {
    int data_block_num, indirect_block_num, dbl_indirect_block_num, offset_into_block;
    void *offset_address = NULL, *block_start_address;
    if (offset >= inode->size)
        return offset_address;

    data_block_num = offset / BLOCK_SIZE;       //integer division
    offset_into_block = offset % BLOCK_SIZE;

    if (data_block_num < DIRECT) {      //direct
        block_start_address = inode->direct[data_block_num];
    } else if (data_block_num < DIRECT + POINTER_PER_BLOCK) {   //single indirect
        indirect_block_num = data_block_num - DIRECT;
        block_start_address = inode->single_indirect->data[indirect_block_num];
    } else {                            //double indirect
        dbl_indirect_block_num = (data_block_num - (DIRECT + POINTER_PER_BLOCK)) / POINTER_PER_BLOCK;
        indirect_block_num = data_block_num - (DIRECT + POINTER_PER_BLOCK) - dbl_indirect_block_num * POINTER_PER_BLOCK;
        block_start_address = inode->double_indirect->indirect_blocks[dbl_indirect_block_num]->
                data[indirect_block_num];
    }
    offset_address = block_start_address + offset_into_block;
    return offset_address;
}


static int rd_creat(const char *usr_str) {
    directory_entry_t new_directory_entry = {
            .filename = {'\0'},
            .index_node_number = 0
    };
    //define file creating path
    char *pathname = NULL;
    size_t usr_str_len = strlen_user(usr_str);
    index_node_t *existing_node = NULL;

    if (usr_str_len <= 2 || !access_ok(VERIFY_READ, usr_str, MAX_FILE_NAME_LEN))
        return -EINVAL;
    pathname = kcalloc(usr_str_len, sizeof(char), GFP_KERNEL);
    if (pathname == NULL)
        return -1;
    strncpy_from_user(pathname, usr_str, usr_str_len);

    existing_node = get_readlocked_index_node(pathname);
    if (existing_node != NULL) {
        read_unlock(&existing_node->file_lock);
        return -EEXIST;
    }

    index_node_t *parent = get_readlocked_parent_index_node(pathname);
    if (parent == NULL) {
        kfree(pathname);
        return -EINVAL;
    } else if (parent->type != DIR || parent->size >= MAX_FILE_SIZE) {
        read_unlock(&parent->file_lock);
        kfree(pathname);
        return -EINVAL;
    }

    index_node_t *new_inode_ptr = get_free_index_node();
    if (new_inode_ptr == NULL) {
        read_unlock(&parent->file_lock);
        kfree(pathname);
        return -EFBIG;
    }
    // prevent others from unlinking file while we release readlock/obtain writelock
    atomic_inc(&parent->open_count);
    read_unlock(&parent->file_lock);
    write_lock(&new_inode_ptr->file_lock);
    new_inode_ptr->type = REG;
    write_lock(&parent->file_lock);
    atomic_dec(&parent->open_count);
    // link to new index node in parent
    directory_entry_t *entry = NULL;
    if (parent->size % BLOCK_SIZE == 0) {
        entry = (directory_entry_t *) extend_inode(parent);
    } else {
        entry = get_directory_entry(parent, parent->size / DIR_ENTRY_SIZE - 1) + 1;
    }
    if (entry == NULL) {
        write_unlock(&new_inode_ptr->file_lock);
        write_unlock(&parent->file_lock);
        kfree(pathname);
        return -EFBIG;
    }
    entry->index_node_number = ((void *) new_inode_ptr - (void *) index_nodes) / INDEX_NODE_SIZE;
    strncpy(entry->filename, strrchr(pathname, '/') + 1, MAX_FILE_NAME_LEN);
    parent->size += DIR_ENTRY_SIZE;
    write_unlock(&new_inode_ptr->file_lock);
    write_unlock(&parent->file_lock);
    kfree(pathname);
    return 0;
}

static int rd_mkdir(const char *usr_str) {
    directory_entry_t new_directory_entry = {
            .filename = {'\0'},
            .index_node_number = 0
    };
    char *pathname = NULL;
    size_t usr_str_len = strlen_user(usr_str);
    index_node_t *existing_node = NULL;
    // similar to rd_create
    if (usr_str_len <= 2 || !access_ok(VERIFY_READ, usr_str, MAX_FILE_NAME_LEN))
        return -EINVAL;
    pathname = kcalloc(usr_str_len, sizeof(char), GFP_KERNEL);
    if (pathname == NULL)
        return -1;
    strncpy_from_user(pathname, usr_str, usr_str_len);
    // if pathname is overflow, free it
    if (strlen(strrchr(pathname, '/') + 1) > MAX_FILE_NAME_LEN) {
        kfree(pathname);
        return -EINVAL;
    }

    existing_node = get_readlocked_index_node(pathname);
    if (existing_node != NULL) {
        read_unlock(&existing_node->file_lock);
        return -EEXIST;
    }

    index_node_t *parent = get_readlocked_parent_index_node(pathname);
    if (parent == NULL) {
        kfree(pathname);
        return -EINVAL;
    } else if (parent->type != DIR || parent->size >= MAX_FILE_SIZE) {
        read_unlock(&parent->file_lock);
        kfree(pathname);
        return -EINVAL;
    }

    index_node_t *new_inode_ptr = get_free_index_node();
    if (new_inode_ptr == NULL) {
        read_unlock(&parent->file_lock);
        kfree(pathname);
        return -EFBIG;
    }
    // prevent others from unlinking file while we release readlock/obtain writelock
    atomic_inc(&parent->open_count);
    read_unlock(&parent->file_lock);
    write_lock(&new_inode_ptr->file_lock);
    new_inode_ptr->type = DIR;
    write_lock(&parent->file_lock);
    atomic_dec(&parent->open_count);
    // link to new index node in parent
    directory_entry_t *entry = NULL;
    if (parent->size % BLOCK_SIZE == 0) {
        entry = (directory_entry_t *) extend_inode(parent);
    } else {
        entry = get_directory_entry(parent, parent->size / DIR_ENTRY_SIZE - 1) + 1;
    }
    if (entry == NULL) {
        write_unlock(&new_inode_ptr->file_lock);
        write_unlock(&parent->file_lock);
        kfree(pathname);
        return -EFBIG;
    }

    entry->index_node_number = ((void *) new_inode_ptr - (void *) index_nodes) / INDEX_NODE_SIZE;
    strncpy(entry->filename, strrchr(pathname, '/') + 1, MAX_FILE_NAME_LEN);
    parent->size += DIR_ENTRY_SIZE;
    write_unlock(&new_inode_ptr->file_lock);
    write_unlock(&parent->file_lock);
    kfree(pathname);
    return 0;
}

static int rd_unlink(const char *usr_str) {
    int i = 0, indirect_block_num = 0, dir_block_num = 0, open_count = 0;
    char *pathname = NULL;
    size_t usr_strlen = strlen_user(usr_str);
    index_node_t *node = NULL;
    printk("Starting unlink\n");
    if (usr_strlen <= 2)
        return -EINVAL;
    pathname = kcalloc(usr_strlen, sizeof(char), GFP_KERNEL);
    if (pathname == NULL)
        return -1;
    strncpy_from_user(pathname, usr_str, usr_strlen);
    // remove trailing forward slash, if it exists
    if (pathname[usr_strlen - 1] == '/')
        pathname[usr_strlen - 1] = '\0';

    index_node_t *parent_node = get_readlocked_parent_index_node(pathname);
    if (parent_node == NULL) {
        kfree(pathname);
        return -EINVAL;
    }
    atomic_inc(&parent_node->open_count);
    read_unlock(&parent_node->file_lock);
    write_lock(&parent_node->file_lock);
    atomic_dec(&parent_node->open_count);

    int last_entry_index = parent_node->size / DIR_ENTRY_SIZE - 1;
    const char *filename = strrchr(pathname, '/') + 1;
    for (i = 0; i <= last_entry_index; ++i) {
        directory_entry_t *entry = get_directory_entry(parent_node, i);
        if (strncmp(entry->filename, filename, MAX_FILE_NAME_LEN) == 0) {
            node = get_inode(entry->index_node_number);
            if (!write_trylock(&node->file_lock)) {
                printk("cannot unlink file\n");
                write_unlock(&parent_node->file_lock);
                kfree(pathname);
                return -EINVAL;
            } else if (atomic_read(&node->open_count) > 0) {
                printk("attempt to unlink a open file!\n");
                write_unlock(&parent_node->file_lock);
                write_unlock(&node->file_lock);
                kfree(pathname);
                return -EINVAL;
            }
            if (node->type == DIR) {
                if (node->size != 0) {
                    printk("attempt to unlink a non-empty directory file!\n");
                    write_unlock(&parent_node->file_lock);
                    write_unlock(&node->file_lock);
                    kfree(pathname);
                    return -EINVAL;
                }
            } else {
                // release all datablocks
                int num_blocks = node->size / BLOCK_SIZE;
                void *block_to_release = NULL;
                while (num_blocks != 0) {
                    block_to_release = get_byte_address(node, (num_blocks - 1) * BLOCK_SIZE);
                    release_data_block(block_to_release);
                    num_blocks--;
                }
                if (node->double_indirect != NULL) {
                    // need to release the double indirect block and the single indirect blocks pointed to from it
                    for (indirect_block_num = 0; indirect_block_num < POINTER_PER_BLOCK; indirect_block_num++) {
                        if (node->double_indirect->indirect_blocks[indirect_block_num] != NULL)
                            release_data_block(node->double_indirect->indirect_blocks[indirect_block_num]);
                    }
                    release_data_block(node->double_indirect);
                    node->double_indirect = NULL;
                }
                if (node->single_indirect != NULL)
                    release_data_block(node->single_indirect);
            }

            // delete entry in parent
            directory_entry_t *last_entry = get_directory_entry(parent_node, last_entry_index);
            if (entry != last_entry)
                *entry = *last_entry;
            parent_node->size -= DIR_ENTRY_SIZE;
            if (parent_node->size % BLOCK_SIZE == 0) {
                release_data_block(last_entry);
                // remove last location(DIRECT)
                if (parent_node->size / BLOCK_SIZE < DIRECT) {
                    parent_node->direct[parent_node->size / BLOCK_SIZE] = NULL;
                } else if (parent_node->size / BLOCK_SIZE < DIRECT + POINTER_PER_BLOCK) { // if it's single indirect
                    if (parent_node->size / BLOCK_SIZE == DIRECT) {
                        // need to also release the indirect block
                        release_data_block(parent_node->single_indirect);
                        parent_node->single_indirect = NULL;
                    } else {
                        // need to NULL out the entry in SINGLE_INDIRECT block that pointed to the directory entry block we just released
                        for (dir_block_num = 0; dir_block_num < POINTER_PER_BLOCK; dir_block_num++) {
                            if (parent_node->single_indirect->data[dir_block_num] == last_entry)
                                parent_node->single_indirect->data[dir_block_num] = NULL;
                        }
                    }
                } else if (parent_node->size / BLOCK_SIZE < DIRECT + POINTER_PER_BLOCK * (1 + POINTER_PER_BLOCK)) { //if it's double indirect
                    if (parent_node->size / BLOCK_SIZE == DIRECT + POINTER_PER_BLOCK) {
                        // need to release the single indirect block that pointed to the entry we just released and also the double indirect block
                        release_data_block(parent_node->double_indirect->indirect_blocks[0]);
                        release_data_block(parent_node->double_indirect);
                        parent_node->double_indirect = NULL;
                    } else if ((parent_node->size / BLOCK_SIZE) % (POINTER_PER_BLOCK * BLOCK_SIZE) != 0) {
                        // need to NULL out the entry in SINGLE_INDIRECT block that pointed to the directory entry block we just released
                        int indirect_block_index = (parent_node->size - BLOCK_SIZE * (DIRECT + POINTER_PER_BLOCK)) / (POINTER_PER_BLOCK * BLOCK_SIZE);
                        int index_in_indirect_block = ((parent_node->size - BLOCK_SIZE * (DIRECT + POINTER_PER_BLOCK)) % (POINTER_PER_BLOCK * BLOCK_SIZE)) / BLOCK_SIZE;
                        parent_node->double_indirect->indirect_blocks[indirect_block_index]->data[index_in_indirect_block] = NULL;
                    } else if ((parent_node->size - BLOCK_SIZE * (DIRECT + POINTER_PER_BLOCK)) % (POINTER_PER_BLOCK * BLOCK_SIZE) == 0) {
                        /* Need to free the SINGLE_INDIRECT block that pointed to the
                       directory entry block we just released, and
                       NULL out the entry in the DOUBLE_INDIRECT block
                       that pointed to this SINGLE_INDIRECT block
                        */
                        int indirect_block_index = (parent_node->size - BLOCK_SIZE * (DIRECT + POINTER_PER_BLOCK)) / (POINTER_PER_BLOCK * BLOCK_SIZE);
                        release_data_block(parent_node->double_indirect->indirect_blocks[indirect_block_index]);
                        parent_node->double_indirect->indirect_blocks[indirect_block_index] = NULL;
                    } else {
                        printk(KERN_ERR "Encountered unexpected case in rd_unlink\n");
                    }
                }
            }
            break;
        }
    }
    write_unlock(&parent_node->file_lock);
    if (node == NULL) {
        printk("the pathname does not exist,\n");
        return -EINVAL;
    }

    // init node
    node->type = UNALLOCATED;
    node->size = 0;
    atomic_set(&node->open_count, 0);
    for (i = 0; i < DIRECT; i++)
        node->direct[i] = NULL;
    node->single_indirect = NULL;
    node->double_indirect = NULL;
    write_unlock(&node->file_lock);
    kfree(pathname);
    spin_lock(&super_block_spinlock);
    super_block->num_free_inodes++;
    spin_unlock(&super_block_spinlock);
    return 0;
}

static int rd_open(const pid_t pid, const char *usr_str) {
    char *pathname = NULL;
    size_t usr_strlen = strlen_user(usr_str);
    int ret;
    pathname = kcalloc(usr_strlen, sizeof(char), GFP_KERNEL);
    strncpy_from_user(pathname, usr_str, usr_strlen);
    printk("Opening %s\n", pathname);

    // remove trailing forward slash, if it exists
    if (usr_strlen > 2 && pathname[usr_strlen - 1] == '/')
        pathname[usr_strlen - 1] = '\0';

    index_node_t *node = get_readlocked_index_node(pathname);
    kfree(pathname);
    if (node == NULL)
        return -EINVAL;
    atomic_inc(&node->open_count);
    file_object_t new_fo = {
            .index_node = node,
            .file_position = 0
    };
    // return a ﬁle descriptor value that will index into the process' ramdisk ﬁle descriptor table
    file_descriptor_table_t *fdt = get_file_descriptor_table(pid);
    if (fdt == NULL)
        fdt = create_file_descriptor_table(pid);
    if (fdt == NULL) {
        atomic_dec(&node->open_count);
        read_unlock(&node->file_lock);
        printk("Failed to create FDT for process %d\n", pid);
        return -1;
    }
    ret = create_file_descriptor_table_entry(fdt, new_fo);
    if (ret < 0) {
        atomic_dec(&node->open_count);
    }
    read_unlock(&node->file_lock);
    return ret;
}

static int rd_close(const pid_t pid, const int fd) {
    file_descriptor_table_t *fdt = get_file_descriptor_table(pid);
    if (fdt == NULL) {
        return -EINVAL;
    }
    file_object_t fo = get_file_descriptor_table_entry(fdt, fd);
    if (fo.index_node == NULL) {
        return -EINVAL;
    }
    atomic_dec(&fo.index_node->open_count);
    return delete_file_descriptor_table_entry(fdt, fd);
}

static int rd_read(const pid_t pid, const rd_rwfile_arg_t *usr_arg) {
    rd_rwfile_arg_t *read_arg = NULL;
    unsigned long data_requested = 0,
            data_fulfillable = 0,
            data_left_to_read = 0,
            bytes_until_end_of_block = 0,
            bytes_left_in_file = 0,
            data_to_be_read_at_address = 0,
            data_to_copy = 0,
            num_copied = 0,
            num_not_copied = 0;
    void *dest = NULL, *from = NULL, *data_buf = NULL;
    index_node_t *inode = NULL;
    // make sure the process has a file descriptor table
    file_descriptor_table_t *fdt = get_file_descriptor_table(pid);
    if (fdt == NULL)
        return -1;
    // copy argument from user space and check validity
    read_arg = kcalloc(1, sizeof(rd_rwfile_arg_t), GFP_KERNEL);
    if (read_arg == NULL)
        return -1;
    num_not_copied = copy_from_user(read_arg, usr_arg, sizeof(rd_rwfile_arg_t));
    if (num_not_copied != 0 || read_arg->num_bytes < 0) {
        kfree(read_arg);
        return -EINVAL;
    }
    data_requested = read_arg->num_bytes;
    data_fulfillable = min(data_requested, MAX_FILE_SIZE);
    data_left_to_read = data_fulfillable;
    file_object_t fo = get_file_descriptor_table_entry(fdt, read_arg->fd);
    if (fo.index_node == NULL) {
        kfree(read_arg);
        return -EINVAL;
    }

    data_buf = kcalloc(data_fulfillable, 1, GFP_KERNEL);
    if (data_buf == NULL) {
        kfree(read_arg);
        return -EINVAL;
    }

    dest = data_buf;

    read_lock(&fo.index_node->file_lock);

    if (fo.index_node->type != REG) {
        read_unlock(&fo.index_node->file_lock);
        kfree(read_arg);
        kfree(data_buf);
        return -EINVAL;
    }

    inode = fo.index_node;

    // start reading data
    while (data_left_to_read > 0) {
        if (fo.file_position == inode->size) // file_position is at EOF
            break;

        from = get_byte_address(inode, fo.file_position);
        bytes_until_end_of_block = (unsigned long) BLOCK_END(from) - (unsigned long) from;
        bytes_left_in_file = inode->size - fo.file_position;
        data_to_be_read_at_address = min(bytes_until_end_of_block, bytes_left_in_file);
        data_to_copy = min(data_to_be_read_at_address, data_left_to_read);
        memcpy(dest, from, data_to_copy);
        num_copied = data_to_copy;
        data_left_to_read -= num_copied;
        dest += num_copied;
        fo.file_position += num_copied;
        if (num_not_copied > 0) break;
    }
    read_unlock(&fo.index_node->file_lock);
    set_file_descriptor_table_entry(fdt, read_arg->fd, fo);
    copy_to_user(read_arg->address, data_buf, data_requested - data_left_to_read);
    kfree(read_arg);
    kfree(data_buf);
    return data_fulfillable - data_left_to_read;
}

static int rd_write(const pid_t pid, const rd_rwfile_arg_t *usr_arg) {
    rd_rwfile_arg_t *write_arg = NULL;
    unsigned long data_requested = 0,
            data_fulfillable = 0,
            data_left_to_write = 0,
            data_to_copy = 0,
            space_available_at_dest = 0,
            num_copied = 0,
            num_not_copied = 0;
    void *curr_offset_address = NULL, *dest = NULL, *src = NULL, *data_buf = NULL;
    index_node_t *inode = NULL;
    // make sure the process has a file descriptor table
    file_descriptor_table_t *fdt = get_file_descriptor_table(pid);
    if (fdt == NULL)
        return -1;

    // copy argument from user space and check validity
    write_arg = kcalloc(1, sizeof(rd_rwfile_arg_t), GFP_KERNEL);
    if (write_arg == NULL)
        return -1;
    num_not_copied = copy_from_user(write_arg, usr_arg, sizeof(rd_rwfile_arg_t));
    if (num_not_copied != 0 || write_arg->num_bytes < 0) {
        kfree(write_arg);
        return -EINVAL;
    }

    data_requested = write_arg->num_bytes;
    data_fulfillable = min(data_requested, MAX_FILE_SIZE);
    data_left_to_write = data_fulfillable;

    file_object_t fo = get_file_descriptor_table_entry(fdt, write_arg->fd);
    if (fo.index_node == NULL) {
        kfree(write_arg);
        return -EINVAL;
    }

    data_buf = kcalloc(data_fulfillable, 1, GFP_KERNEL);
    if (data_buf == NULL) {
        printk(KERN_ERR
        "Couldn't calloc buffer for write\n");
        kfree(write_arg);
        return -EFBIG;
    }

    num_not_copied = copy_from_user(data_buf, write_arg->address, data_fulfillable);
    if (num_not_copied > 0) {
        printk(KERN_ERR
        "Couldnt buffer the %d bytes requested to be written\n", data_requested);
        kfree(write_arg);
        kfree(data_buf);
        return -EINVAL;
    }

    src = data_buf;
    inode = fo.index_node;

    if (!write_trylock(&inode->file_lock)) {
        kfree(write_arg);
        kfree(data_buf);
        return -EINVAL;
    }

    if (inode->type != REG || inode->size == MAX_FILE_SIZE) {
        write_unlock(&inode->file_lock);
        kfree(write_arg);
        kfree(data_buf);
        return -EINVAL;
    }

    // start writing data
    while (data_left_to_write > 0) {
        if (fo.file_position == MAX_FILE_SIZE)
            break;

        if (fo.file_position == inode->size && inode->size % BLOCK_SIZE == 0) {
            // writing past the current end of the last block of file
            printk("Getting new data block for inode\n");
            dest = extend_inode(inode);
            if (dest == NULL)
                break;
            space_available_at_dest = BLOCK_SIZE;
        } else {
            curr_offset_address = get_byte_address(inode, inode->size - 1);
            if (curr_offset_address == NULL) {
                printk(KERN_ERR "Unexpected error getting byte address of byte %d in rd_write\n", inode->size - 1);
                break;
            }
            dest = curr_offset_address + 1;
            space_available_at_dest = (unsigned long) BLOCK_END(dest) - (unsigned long) dest;
        }
        data_to_copy = min(data_left_to_write, space_available_at_dest);
        memcpy(dest, src, data_to_copy);
        num_copied = data_to_copy;
        data_left_to_write -= num_copied;
        src += num_copied;
        fo.file_position += num_copied;
        if ((inode->size - 1) < fo.file_position) { // We wrote past original EOF
            printk("Current inode size: %d\n", inode->size);
            inode->size += fo.file_position - inode->size;
            printk("New inode size: %d\n", inode->size);
        }
        if (num_not_copied > 0) break;
    }
    write_unlock(&inode->file_lock);
    set_file_descriptor_table_entry(fdt, write_arg->fd, fo);
    kfree(data_buf);
    kfree(write_arg);
    return data_fulfillable - data_left_to_write;
}

static int rd_lseek(const pid_t pid, const rd_seek_arg_t *usr_arg) {
    rd_seek_arg_t *seek_arg = NULL;
    unsigned long num_not_copied = 0;
    // make sure the process has a file descriptor table
    file_descriptor_table_t *fdt = get_file_descriptor_table(pid);
    if (fdt == NULL)
        return -1;

    // copy argument from user space and check validity
    seek_arg = kcalloc(1, sizeof(rd_seek_arg_t), GFP_KERNEL);
    if (seek_arg == NULL)
        return -1;
    num_not_copied = copy_from_user(seek_arg, usr_arg, sizeof(rd_seek_arg_t));
    if (num_not_copied != 0 || seek_arg->offset < 0) {
        kfree(seek_arg);
        return -EINVAL;
    }

    file_object_t fo = get_file_descriptor_table_entry(fdt, seek_arg->fd);
    if (fo.index_node == NULL) {
        kfree(seek_arg);
        return -EINVAL;
    }
    read_lock(&fo.index_node->file_lock);
    if (fo.index_node->type != REG ||
        seek_arg->offset > fo.index_node->size
        || seek_arg->offset >= MAX_FILE_SIZE) {
        read_unlock(&fo.index_node->file_lock);
        return -EINVAL;
    }
    read_unlock(&fo.index_node->file_lock);
    fo.file_position = seek_arg->offset;
    set_file_descriptor_table_entry(fdt, seek_arg->fd, fo);
    kfree(seek_arg);
    return 0;
}

static int rd_readdir(const pid_t pid, const rd_readdir_arg_t *usr_arg) {
    int i = 0;
    unsigned long num_not_copied = 0;
    file_descriptor_table_t *fdt = get_file_descriptor_table(pid);
    rd_readdir_arg_t *read_arg = NULL;
    directory_entry_t *entry = NULL;
    if (fdt == NULL)
        return -1;
    read_arg = kcalloc(1, sizeof(rd_readdir_arg_t), GFP_KERNEL);
    if (read_arg == NULL)
        return -1;
    num_not_copied = copy_from_user(read_arg, usr_arg, sizeof(rd_readdir_arg_t));
    if (num_not_copied != 0) {
        kfree(read_arg);
        return -EINVAL;
    }
    file_object_t fo = get_file_descriptor_table_entry(fdt, read_arg->fd);
    if (fo.index_node == NULL || fo.index_node->type != DIR) {
        kfree(read_arg);
        return -EINVAL;
    }
    // check if it's at EOF
    if (fo.index_node->size == 0 || fo.index_node->size == fo.file_position) {
        kfree(read_arg);
        return 0;
    }
    entry = get_directory_entry(fo.index_node, fo.file_position / DIR_ENTRY_SIZE);
    num_not_copied = copy_to_user(read_arg->address, entry->filename, MAX_FILE_NAME_LEN);
    if (num_not_copied != 0) {
        kfree(read_arg);
        return -EINVAL;
    }
    fo.file_position += DIR_ENTRY_SIZE;
    set_file_descriptor_table_entry(fdt, read_arg->fd, fo);
    kfree(read_arg);
    return 1;
}

module_init(initialization_routine);
module_exit(cleanup_routine);

