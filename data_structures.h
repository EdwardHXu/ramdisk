#include <linux/list.h>

//define some constants here
#define RD_SIZE 0x200000    //2MB
#define BLOCK_SIZE 256
#define BLOCK_POINTER_SIZE 4
#define DIRECT 8
#define POINTER_PER_BLOCK (BLOCK_SIZE / BLOCK_POINTER_SIZE)
#define BLOCK_INDEX_NODES 256
#define INDEX_NODE_SIZE 64
#define INDEX_NODES (BLOCK_INDEX_NODES * (BLOCK_SIZE / INDEX_NODE_SIZE))
#define BLOCK_BITMAPS 4
#define BLOCK_DATA ((BLOCK_SIZE - BLOCK_SIZE * (1 + BLOCK_INDEX_NODES + BLOCK_BITMAPS)) / BLOCK_SIZE)
#define DIR_ENTRY_SIZE 16
#define DIR_ENTRY_PER_BLOCK (BLOCK_SIZE / DIR_ENTRY_SIZE)
#define MAX_DIR_ENTRIES (DIR_ENTRY_PER_BLOCK * (DIRECT + POINTER_PER_BLOCK + POINTER_PER_BLOCK*POINTER_PER_BLOCK))
#define MAX_FILES 1023
#define MAX_FILE_SIZE (BLOCK_SIZE * (DIRECT + POINTER_PER_BLOCK + POINTER_PER_BLOCK*POINTER_PER_BLOCK))
#define MAX_FILE_NAME_LEN 14
#define INIT_FDT_LEN 64     //init file descriptor length


//define data structures here
typedef struct rd_super_block {
    int num_free_blocks;
    int num_free_inodes;
    /* Additional info? (struct can be as large as BLK_SZ bytes) */
} super_block_t;

typedef enum FILE_TYPE {
    UNALLOCATED = 0,
    ALLOCATED,
    DIR,
    REG
} file_type_t;

typedef struct indirect_block {
    void *data[POINTER_PER_BLOCK];
} indirect_block_t;

typedef struct double_indirect_block_t {
    indirect_block_t *indirect_blocks[POINTER_PER_BLOCK];
} double_indirect_block_t;

typedef struct index_node {
    file_type_t type;
    int size;
    atomic_t open_count;    // Used to allow readers to increment open_count
    rwlock_t file_lock;     // sizeof(rwlock_t) == 4
    void *direct[DIRECT];
    indirect_block_t *single_indirect;
    double_indirect_block_t *double_indirect;
} index_node_t;             //sizeof(index_node_t) == 52?

typedef struct directory_entry {
    char filename[MAX_FILE_NAME_LEN];   // 14 bytes including null terminator
    unsigned short index_node_number;   // 2 bytes
} directory_entry_t;

typedef struct file_object {
    index_node_t *index_node;
    off_t file_position;
} file_object_t;

/* file_descriptor_table_t should be an -opaque- type */
typedef struct file_descriptor_table {
    struct list_head list;
    file_object_t *entries;
    pid_t owner; //pid of the process the fdt belongs to
    size_t entries_length;
    size_t num_free_entries;
} file_descriptor_table_t;

/* Directory -block- has BLK_SZ / sizeof(directory_entry_t)
   directory entries == 16 entries */