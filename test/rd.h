#ifndef RD_LIB_H
#define RD_LIB_H
int rd_creat(char *pathname);
int rd_mkdir(char *pathname);
int rd_open(char *pathname);
int rd_close(int fd);
int rd_read(int fd, char *address, int num_bytes);
int rd_write(int fd, char *address, int num_bytes);
int rd_lseek(int fd, int offset);
int rd_unlink(char *pathname);
int rd_readdir(int fd, char *address);
#endif
