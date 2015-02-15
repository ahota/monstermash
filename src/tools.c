#include "tools.h"

//Main processing functions for user commands


//Open the disk file for reading
//Make sure to commit_disk() when done
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

//Close the given disk file
int commit_disk(FILE *disk) {
    int ret = fclose(disk); 
    disk = 0;
    if (ret) {
        fprintf(stderr, "Could not commit disk correctly. \n");
    }
    return ret;
}

//Create a new inode and return its ID
short inode_create(char *name, char type, short *inode_counter) {
    FILE *disk = access_disk(0);

    //Find the first free space for this inode
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
    //Seeking to the offset that this inode is to be written
    fseek(disk, offset, SEEK_SET);
    fwrite(&type, sizeof(char), 1, disk);
    //Write the current value in inode_counter
    fwrite(inode_counter, sizeof(short), 1, disk);
    //Save the current inode_counter and then increment it
    short id = (*inode_counter)++;

    //Create a new data block for this inode to point to
    int block_offset = block_create(name);
    //Save the offset of the created block
    fwrite(&block_offset, sizeof(int), 1, disk);
    commit_disk(disk);
    return id;
}

//Create a new data block and return its offset
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
    //Write the name
    fwrite(name, sizeof(char), strlen(name) + 1, disk);
    //Seek forward by 32 regardless of name length
    fseek(disk, MAX_FILENAME_LENGTH, offset);
    //Write a -1 to indicate no next block
    int empty_offset = -1;
    fwrite(&empty_offset, sizeof(int), 1, disk);

    commit_disk(disk);
    return offset;
}

//That new disk smell
int disk_create(short *inode_counter) {
    FILE *disk = access_disk(1);
    //Seek to the end and write a single byte to force the file to be 100 MB
    fseek(disk, DISK_SIZE, SEEK_SET);
    char emptiness = '0';
    fwrite(&emptiness, sizeof(char), 1, disk);
    commit_disk(disk);

    //Create the root directory
    return directory_create("/", inode_counter, NULL);
}


//Write directory contents to a directory's data block
void write_dir_data(char *name, int *current_dir_inode, short inode_id) {
    FILE *disk = access_disk(0);
    //Seek to the block_offset
    fseek(disk, *current_dir_inode + 3, SEEK_SET);
    int block_offset = 0;
    fread(&block_offset, sizeof(int), 1, disk);
    fseek(disk, block_offset + 36, SEEK_SET);

    int i;
    for (i = 0; i < 4060; i += MAX_FILENAME_LENGTH + sizeof(int)) {
        int temp_id = 0;
        fseek(disk, block_offset + 36 + i, SEEK_SET);
        fread(&temp_id, sizeof(int), 1, disk);
        if (temp_id == 0) {
            break;
        }
    }
    if (i >= 4060) {
        //Go to next block
    }
    else {
        //Go back and write the id
        fseek(disk, -1 * sizeof(int), SEEK_CUR);
        //Write the new file/dir inode id
        fwrite(&inode_id, sizeof(int), 1, disk);
        printf("%d\n", inode_id);
        fwrite(name, sizeof(char), strlen(name), disk);
    }
    commit_disk(disk);
}

//New directory
int directory_create(char *name, short *inode_counter, int* current_dir_inode) {
    short id = inode_create(name, 'd', inode_counter);
    //Check if we're writing the root directory
    //Root doesn't need data in it yet
    if (current_dir_inode != NULL) {
        write_dir_data(name, current_dir_inode, id);
    }

    //wtf
    return 0; // The offset for the / inode
}

//Update the shell prompt to reflect the user's current location
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

//Iterate through the current directory's data table and list valid files/dirs
void ls_dir(int current_dir_inode) {
    FILE *disk = access_disk(0);
    //Go directly to the first block pointer for the current inode
    fseek(disk, current_dir_inode + 3, SEEK_SET);
    int data_block_offset = -1;
    fread(&data_block_offset, sizeof(int), 1, disk);
    printf("%d\n", data_block_offset);
    
    //Go directly to the data in the block
    fseek(disk, data_block_offset + 36, SEEK_SET);
    int i;
    char *name = malloc(33);
    for (i = 0; i < 4060; i += sizeof(int) + MAX_FILENAME_LENGTH) {
        //Go to current entry in inode table
        fseek(disk, data_block_offset + 36 + i, SEEK_SET);
        //Skip inode id
        fseek(disk, sizeof(int), SEEK_CUR);
        fread(name, sizeof(char), MAX_FILENAME_LENGTH, disk);
        if(strlen(name) != 0)
            printf("%s/\t", name);
    }
    printf("\n");
    commit_disk(disk);
}

//Go to a directory
int ch_dir(char *name, int *current_dir_inode) {
    FILE *disk = access_disk(0);
    fseek(disk, *current_dir_inode + 3, SEEK_SET);
    int data_block_offset = -1;
    fread(&data_block_offset, sizeof(int), 1, disk);
    fseek(disk, data_block_offset + 36, SEEK_SET);

    int i;
    char *check_name = malloc(32);
    int inode_id = 0;
    //Find the directory/file in the current directory's inode table
    for (i = 0; i < 4060; i += sizeof(int) + MAX_FILENAME_LENGTH) {
        fseek(disk, data_block_offset + 36 + i, SEEK_SET);
        fread(&inode_id, sizeof(int), 1, disk);
        printf("%d\n", inode_id);
        //fseek(disk, sizeof(short), SEEK_CUR);
        fread(check_name, sizeof(char), MAX_FILENAME_LENGTH, disk);
        if (strcmp(name, check_name) == 0) {
            break;
        }
    }
    if (i >= 4060) {
        fprintf(stderr, "Directory does not exist\n");
        //destroy_file_system();
        return *current_dir_inode;
    }
    
    //Go to the inode and set the current inode pointer to its offset
    for (i = 0; i < INODE_TABLE_SIZE; i += INODE_SIZE) {
        short check_inode_id;
        fseek(disk, i + 1, SEEK_SET);
        fread(&check_inode_id, sizeof(short), 1, disk);
        if (check_inode_id == inode_id) {
            commit_disk(disk);
            return i;
        }
    }
    //Weird error that hopefully never happens
    fprintf(stderr, "Couldn't find inode %d\n", inode_id);
    return *current_dir_inode;
}

