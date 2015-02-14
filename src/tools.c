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

short inode_create(char *name, char type, short *inode_counter) {
    FILE *disk = access_disk(0);

    // Determine a free offset
    int offset;
    for (offset = 0; offset < INODE_TABLE_SIZE; offset += INODE_SIZE) {
        fseek(disk, offset, SEEK_SET);
        char inode_type;
        fread(&inode_type, sizeof(char), 1, disk);
        if (inode_type == 0) {
            break;
        }
    }
    if (offset == INODE_TABLE_SIZE) {
        fprintf(stderr, "inode table full\n");
        return -1;
    }

    //Writing inode data
    fseek(disk, offset, SEEK_SET); //Seeking to the offset that this inode is to be written
    printf("write 1:\n");
    fwrite(&type, sizeof(char), 1, disk);
    printf("write 2:\n");
    fwrite(inode_counter, sizeof(short), 1, disk);
    short id = *inode_counter++;
    int block_offset = block_create(name); //Creating the new block and getting the offset
    printf("write 3:\n");
    fwrite(&block_offset, sizeof(int), 1, disk);
    commit_disk(disk);
    return id;
}

int block_create(char *name) {
    FILE *disk = access_disk(0);

    //Determine an offset for the new block
    int offset;
    for (offset = INODE_TABLE_SIZE; offset < DISK_SIZE; offset += BLOCK_SIZE) { 
        char block_check;
        fseek(disk, offset, SEEK_SET);
        fread(&block_check, sizeof(char), 1, disk);
        if (block_check == 0) {
            break;
        }
    }

    //Check if offset reached the end
    if (offset == DISK_SIZE) {
        fprintf(stderr, "Disk is full!\n");
        return -1;
    }

    fseek(disk, offset, SEEK_SET);
    fwrite(name, sizeof(char), strlen(name) + 1, disk);//Write the name
    //Seek forward by 32 regardless of name length
    fseek(disk, MAX_FILENAME_LENGTH, offset);
    //Write a -1 to indicate no next block
    int empty_offset = -1;
    fwrite(&empty_offset, sizeof(int), 1, disk);

    commit_disk(disk);
    printf("%d", offset);
    return offset;
}

int disk_create(short *inode_counter) {
    FILE *disk = access_disk(1);
    fseek(disk, DISK_SIZE, SEEK_SET);
    char emptiness = '0';
    fwrite(&emptiness, sizeof(char), 1, disk);
    commit_disk(disk);   
    return directory_create("/", inode_counter);
}

int directory_create(char *name, short *inode_counter) {
    short id = inode_create(name, 'd', inode_counter);
    return 0; // The offset for the / inode
}

void update_prompt(int current_dir_inode, char *path) {
    FILE *disk = access_disk(0);
    fseek(disk, current_dir_inode + sizeof(char) + sizeof(short), SEEK_SET);
    int dir_offset = 0;
    fread(&dir_offset, sizeof(int), 1, disk);
    fseek(disk, dir_offset, SEEK_SET);
    char *dir_name = malloc(MAX_FILENAME_LENGTH);
    fread(dir_name, sizeof(char), MAX_FILENAME_LENGTH, disk);
    strcpy(path, dir_name);
    commit_disk(disk);
}

void ls_dir(int current_dir_inode) {
    FILE *disk = access_disk(0);
    //Go directly to the first block pointer for the current inode
    fseek(disk, current_dir_inode + 3, SEEK_SET);
    int data_block_offset = -1;
    fread(&data_block_offset, sizeof(int), 1, disk);
    printf("%d\n", data_block_offset);
    
    //Go directly to the data in the block
    fseek(disk, data_block_offset*INODE_SIZE + 36, SEEK_SET);
    char *block_data = malloc(4060 * sizeof(char));
    fread(block_data, sizeof(char), 4060, disk);
    printf("block data:\n%s\n", block_data);
    commit_disk(disk);
}
