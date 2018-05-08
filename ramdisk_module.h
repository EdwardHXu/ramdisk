/*
 * Defines necessary info for accessing ramdisk through
 * ioctls
 *
 */

#include <linux/ioctl.h>
#include "data_structures.h"

typedef struct rd_rwfile_arg {
    char *address;
    int fd;
    int num_bytes;
} rd_rwfile_arg_t;

typedef struct rd_seek_arg {
    int fd;
    int offset;
} rd_seek_arg_t;

typedef struct rd_readdir_arg {
    char *address;
    int fd;
} rd_readdir_arg_t;

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
static void debug_print_fdt_pids(void);
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

/* *** Declarations of ramdisk synchronization */
DEFINE_RWLOCK(rd_init_rwlock);
/* Locks to ensure consistent view of ramdisk memory */
DEFINE_SPINLOCK(super_block_spinlock);
DEFINE_SPINLOCK(block_bitmap_spinlock);
DEFINE_RWLOCK(index_nodes_rwlock);
DEFINE_RWLOCK(file_descriptor_tables_rwlock);

/* Declarations of ramdisk data structures */
static bool rd_initialized_flag = false;
static super_block_t *super_block = NULL;
static index_node_t *index_nodes = NULL; // 256 blocks/64 bytes per inode = 1024 inodes
static void *block_bitmap = NULL; // 4 blocks => block_bitmap is 1024 bytes long
static void *data_blocks = NULL; // len(data_blocks) == 7931 blocks
static int temp = 0;
static LIST_HEAD(file_descriptor_tables);

#define INODE_PTR(index) (index_node_t *) (((void *) index_nodes) + index * INDEX_NODE_SIZE)
#define BLOCK_START(byte_address) ((void *)byte_address - (((unsigned long) ((void *)byte_address - data_blocks)) % BLOCK_SIZE))
#define BLOCK_END(byte_address) (BLOCK_START(byte_address) + BLOCK_SIZE)



/* Major device number used for ioctls */
#define MAJOR_NUM 100
#define RD_INIT _IO(MAJOR_NUM, 0)
#define RD_CREAT _IOW(MAJOR_NUM, 1, char *)
#define RD_MKDIR _IOW(MAJOR_NUM, 2, char *)
#define RD_OPEN _IOW(MAJOR_NUM, 3, char *)
#define RD_CLOSE _IO(MAJOR_NUM, 4)
#define RD_READ _IOWR(MAJOR_NUM, 5, struct rd_rwfile_arg)
#define RD_WRITE _IOW(MAJOR_NUM, 6, struct rd_rwfile_arg)
#define RD_LSEEK _IOW(MAJOR_NUM, 7, struct rd_seek_arg)
#define RD_UNLINK _IOW(MAJOR_NUM, 8, char *)
#define RD_READDIR _IOWR(MAJOR_NUM, 9, char *)
#define DBG_PRINT_FDT_PIDS _IO(MAJOR_NUM, 9 + 1)
#define DBG_MK_FDT _IO(MAJOR_NUM, 9 + 2)
#define DBG_RM_FDT _IO(MAJOR_NUM, 9 + 3)
#define DBG_TEST_OFFSET_INFO _IO(MAJOR_NUM, 9 + 4)

