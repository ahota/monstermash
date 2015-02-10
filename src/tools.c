#include "tools.h"

FILE *access_disk(int first_access) {
    char *access = first_access ? "w+" : "r+"; 
    FILE *disk = fopen(FS_PATH, access);
    if (disk) {
        fseek(disk, 0, SEEK_SET);
        return disk;
    }
    else {
        fprintf(stderr, "Could not access disk. \n");
        return NULL;
    }
}

int commit_disk(FILE *disk) {
    int ret = fclose(disk); 
    disk = 0;
    if (ret) {
        fprintf(stderr, "Could not commit disk correctly. \n");
    }
    return ret;
}

short inode_create(char type, int offset, short *inode_counter) {
    FILE *disk = access_disk(0);
    fseek(disk, offset, SEEK_SET);
    fwrite(&type, sizeof(char), 1, disk);
    fwrite(inode_counter, sizeof(short), 1, disk);
    short id = *inode_counter++;
    int block_offset = INODE_TABLE_SIZE;
    fwrite(&block_offset, sizeof(int), 1, disk);
    commit_disk(disk);
    return id;
}

void block_create(char *name, int offset) {
    FILE *disk = access_disk(0);
    fseek(disk, offset, SEEK_SET);
    fwrite(name, sizeof(char), strlen(name) + 1, disk);//Write the name
    fseek(disk, MAX_FILENAME_LENGTH, offset);
    //fseek(disk, 4, SEEK_CUR); //Leave next_block_pointer empty
    commit_disk(disk);
}

short disk_create(short *inode_counter) {
    FILE *disk = access_disk(1);
    fseek(disk, DISK_SIZE, SEEK_SET);
    char emptiness = '0';
    fwrite(&emptiness, sizeof(char), 1, disk);
    commit_disk(disk);   
    return directory_create("/", inode_counter);
}

short directory_create(char *name, short *inode_counter) {
    short id = inode_create('d', 0, inode_counter);
    block_create(name, INODE_TABLE_SIZE);
    return id;
}
