#include "tools.h"

//Main processing functions for user commands

//Split the given arguments into arg1 and arg2
//Either or both argument can have quotations
//If an argument does not have quotations, it is space-delimiter tokenized
void smart_split(char *args, char **arg1, char **arg2) {
    //Trim whitespace from arguments
    int start, end;
    trim_whitespace(args, &start, &end);
    if(end <= start) {
        return;
    }

    //Check if arg1 has quotations
    int arg1_end, arg2_start;
    int arg1_q = 0;
    if(args[start] == '"') {
        arg1_q = 1;
        //Increment start so we don't include the quotation mark
        start++;
        for(arg1_end = start; arg1_end < end; arg1_end++)
            if(args[arg1_end] == '"')
                break;
        //Don't incremenet arg1_end so we don't include the quotation mark
        //arg1_end++;
        *arg1 = malloc(arg1_end - start + 1);
        strncpy(*arg1, args + start, arg1_end - start);
        *(*arg1 + arg1_end - start) = '\0';
    }
    else {
        *arg1 = strtok(args, " \n");
        if(*arg1 == NULL) {
            return;
        }
        arg1_end = strlen(*arg1) + start;
        //Note that arg1 may be incorrect
        //If the user has a filename or path with spaces
        //and didn't put quotations around it, they'll get
        //an error when we try to open the file
    }

    //Find where arg2 starts based on arg1
    for(arg2_start = arg1_end + 1; arg2_start < end; arg2_start++)
        if(args[arg2_start] != ' ')
            break;
    //Check if arg2 has quotations
    if(args[arg2_start] == '"') {
        arg2_start++;
        for(; end > arg2_start; end--)
            if(args[end] == '"')
                break;
        *arg2 = malloc(end - arg2_start + 1);
        strncpy(*arg2, args + arg2_start, end - arg2_start);
        *(*arg2 + end - arg2_start) = '\0';
    }
    else {
        if(arg1_q)
            *arg2 = strtok(args + arg2_start, " \n");
        else
            *arg2 = strtok(NULL, " \n");
        if(*arg2 == NULL) {
            return;
        }
    }
    wlog("start      = %d\n", start);
    wlog("arg1_end   = %d\n", arg1_end);
    wlog("arg2_start = %d\n", arg2_start);
    wlog("end        = %d\n", end);
    wlog("arg1       = %s\n", *arg1);
    wlog("arg2       = %s\n", *arg2);
}

void trim_whitespace(char *name, int *start, int *end) {
    //Find leading/trailing whitespace
    int length = strlen(name);
    for(*start = 0; *start < length; (*start)++)
        if(name[*start] != ' ')
            break;
    for(*end = length - 1; *end > 0; (*end)--)
        if(name[*end] != ' ')
            break;
    (*end)++; //Move end so that we copy up to, but not including, it
}

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
        add_to_response("Could not access disk. \n");
        return NULL;
    }
}

//Close the given disk file
int commit_disk(FILE *disk) {
    int ret = fclose(disk); 
    disk = 0;
    if (ret) {
        add_to_response("Could not commit disk correctly. \n");
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
        add_to_response("inode table full\n");
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
        add_to_response("Disk is full!\n");
        return -1;
    }

    fseek(disk, offset, SEEK_SET);
    //Write the name
    fwrite(name, sizeof(char), strlen(name) + 1, disk);
    //Seek forward by 32 regardless of name length
    fseek(disk, offset + MAX_FILENAME_LENGTH, SEEK_SET);
    //Write a -1 to indicate no next block
    int empty_offset = -1;
    fwrite(&empty_offset, sizeof(int), 1, disk);
    //Write a 1 for link_count
    short link_count = 1;
    fwrite(&link_count, sizeof(short), 1, disk);

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

//Find the first free space in a block's inode table
//Insert the given name and id
int insert_entry(int block_offset, char *name, short inode_id) {
    FILE *disk = access_disk(0);
    int i;
    //Go through every entry in this block's inode table
    for(i = 0; i < BLOCK_SIZE - METADATA_SIZE; i += DIR_TABLE_ENTRY_SIZE) {
        int temp_id = 0;
        fseek(disk, block_offset + METADATA_SIZE + i, SEEK_SET);
        fread(&temp_id, sizeof(int), 1, disk);
        char risk;
        fread(&risk, sizeof(char), 1, disk);
        //Space exists and it's not . or ..
        if(temp_id == 0 && risk != '.') {
            fseek(disk, block_offset + METADATA_SIZE + i, SEEK_SET);
            fwrite(&inode_id, sizeof(short), 1, disk);
            fseek(disk, sizeof(short), SEEK_CUR);
            fwrite(name, sizeof(char), strlen(name), disk);
            commit_disk(disk);
            return 0;
        }
    }
    //Couldn't find space in this block
    commit_disk(disk);
    return -1;
}

//Find an entry with the given name and return its inode_id
short find_entry(int block_offset, char *name) {
    FILE *disk = access_disk(0);
    int i;
    int temp_id;
    char *temp_name = malloc(MAX_FILENAME_LENGTH);
    //Search this block's inode table
    for(i = 0; i < BLOCK_SIZE - METADATA_SIZE; i += DIR_TABLE_ENTRY_SIZE) {
        fseek(disk, block_offset + METADATA_SIZE + i, SEEK_SET);
        fread(&temp_id,  sizeof(int),  1, disk);
        fread(temp_name, sizeof(char), MAX_FILENAME_LENGTH, disk);
        //If name is NULL, we're searching for anything (wildcard * search)
        //Anything that isn't . or .., that is
        if(name == NULL && temp_id != 0 && temp_name[0] != '.') {
            commit_disk(disk);
            return (short)temp_id;
        }
        //Found it
        if(name != NULL && strcmp(temp_name, name) == 0) {
            commit_disk(disk);
            return (short)temp_id;
        }
    }
    //Didn't find it in this block
    commit_disk(disk);
    return -1;
}

//Remove an entry in the given block, return 0 on success, -1 on failure
//This does not remove inodes or data blocks! It only removes table entries
int remove_entry(int block_offset, char *name) {
    FILE *disk = access_disk(0);
    int i;
    int temp_id;
    char *temp_name = malloc(MAX_FILENAME_LENGTH);
    //Search this block's inode table
    for(i = 0; i < BLOCK_SIZE - METADATA_SIZE; i += DIR_TABLE_ENTRY_SIZE) {
        fseek(disk, block_offset + METADATA_SIZE + i, SEEK_SET);
        fread(&temp_id,  sizeof(int),  1, disk);
        fread(temp_name, sizeof(char), MAX_FILENAME_LENGTH, disk);
        //Found it
        if(strcmp(temp_name, name) == 0) {
            wipe(disk, block_offset + METADATA_SIZE + i, DIR_TABLE_ENTRY_SIZE);
            commit_disk(disk);
            return 0;
        }
    }
    //Didn't find it in this block
    commit_disk(disk);
    return -1;
}

//Write directory contents to a directory's data block
void write_dir_data(char *name, int *current_dir_inode, short inode_id) {
    FILE *disk = access_disk(0);
    //Seek to the block_offset
    fseek(disk, *current_dir_inode + 3, SEEK_SET);
    int block_offset = 0;
    fread(&block_offset, sizeof(int), 1, disk);
    while(insert_entry(block_offset, name, inode_id) != 0) {
        fseek(disk, block_offset + MAX_FILENAME_LENGTH, SEEK_SET);
        int next_block_offset;
        fread(&next_block_offset, sizeof(int), 1, disk);
        if(next_block_offset == -1) {
            next_block_offset = block_create("CONTINUED");
            fseek(disk, block_offset + MAX_FILENAME_LENGTH, SEEK_SET);
            fwrite(&next_block_offset, sizeof(int), 1, disk);
        }
        block_offset = next_block_offset;
    }
    commit_disk(disk);
}

//New directory
int directory_create(char *name, short *inode_counter, int* current_dir_inode) {
    short id = inode_create(name, 'd', inode_counter);
    //Write dot
    int inode_offset = find_inode_offset(id);
    write_dir_data(".", &inode_offset, id);
    //Check if we're writing the root directory
    //Root doesn't need data in it yet
    if (current_dir_inode != NULL) {
        //Write dot dot
        short parent_inode_id = find_inode_id(*current_dir_inode);
        write_dir_data("..", &inode_offset, parent_inode_id);
        //Write new directory to parent
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
    
    int i;
    char *name = malloc(33);
    int next_block_offset;
    int counter = 0;
    do {
        fseek(disk, data_block_offset + MAX_FILENAME_LENGTH, SEEK_SET);
        fread(&next_block_offset, sizeof(int), 1, disk);
        for (i = 0; i < BLOCK_SIZE - METADATA_SIZE; i += DIR_TABLE_ENTRY_SIZE) {
            //Go to current entry in inode table
            fseek(disk, data_block_offset + METADATA_SIZE + i, SEEK_SET);
            
            int temp_inode_id = 0;
            fread(&temp_inode_id, sizeof(int), 1, disk);
            fread(name, sizeof(char), MAX_FILENAME_LENGTH, disk);
            if(strlen(name) != 0) {
                //Get the type of this element from its inode
                fseek(disk, find_inode_offset(temp_inode_id), SEEK_SET);
                char type = 0;
                fread(&type, sizeof(char), 1, disk);
                if (type == 'd') {
                    add_to_response(BOLDBLUE "%-*s" RESET,
                            MAX_FILENAME_LENGTH + 1, name);
                }
                else if (type == 'f') {
                    add_to_response(RESET "%-*s" RESET,
                            MAX_FILENAME_LENGTH + 1, name);
                }
                else if (type == 'l'){
                    add_to_response(CYAN "%-*s" RESET,
                            MAX_FILENAME_LENGTH + 1, name); 
                }
                else {
                    add_to_response("Unknown element type %c\n", type);
                }
                counter = (counter + 1) % 2;
                if(!counter)
                    add_to_response("\n");
            }
        }
    } while((data_block_offset = next_block_offset) != -1);
    add_to_response("\n");
    commit_disk(disk);
}

//Go to a directory
int ch_dir(char *name, int *current_dir_inode) {
    int temp_dir_inode = expand_path(name, current_dir_inode, 1);
    if(temp_dir_inode != -1) {
        return temp_dir_inode;
    }
    return *current_dir_inode;
}

int find_inode_offset(short inode_id) {
    FILE *disk = access_disk(0);
    int i;
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
    add_to_response("Couldn't find inode %d\n", inode_id);
    commit_disk(disk);
    return -1;
}

short find_inode_id(int inode_offset) {
    FILE *disk = access_disk(0);
    fseek(disk, inode_offset + 1, SEEK_SET); //+1 for skipping inode type
    short inode_id = 0;
    fread(&inode_id, sizeof(short), 1, disk);
    commit_disk(disk);
    return inode_id;
}

void wipe(FILE* disk, int offset, int n_bytes) {
    fseek(disk, offset, SEEK_SET);
    char *emptiness = malloc(n_bytes);
    int i;
    for (i = 0; i < n_bytes; i++) {
        emptiness[i] = 0;
    }
    fwrite(emptiness, sizeof(char), n_bytes, disk);
}

int directory_remove(char *name, int *current_dir_inode) {
    FILE *disk = access_disk(0);
    fseek(disk, *current_dir_inode + 3, SEEK_SET);
    int data_block_offset = -1, parent_block_offset;
    fread(&parent_block_offset, sizeof(int), 1, disk);
    data_block_offset = parent_block_offset;

    int i;
    char *check_name = malloc(32);
    short inode_id = 0;
    int rmdir_inode_offset = 0, rmdir_block_offset = 0;
    int next_block_offset;
    //Find the directory/file in the current directory's inode table
    do {
        fseek(disk, data_block_offset + MAX_FILENAME_LENGTH, SEEK_SET);
        fread(&next_block_offset, sizeof(int), 1, disk);
        inode_id = find_entry(data_block_offset, name);
        if(inode_id != -1) {
            rmdir_inode_offset = find_inode_offset(inode_id);
            break;
        }
    } while((data_block_offset = next_block_offset) != -1);

    //rmdir_offset was not updated in the loop
    if(rmdir_inode_offset == 0) {
        add_to_response(BOLDRED "Invalid directory name\n" RESET);
        return -1;
    }

    //Get the first data block offset for this directory
    fseek(disk, rmdir_inode_offset + 3, SEEK_SET);
    fread(&rmdir_block_offset, sizeof(int), 1, disk);
    data_block_offset = rmdir_block_offset;
    //See if the directory is empty
    do {
        fseek(disk, data_block_offset + MAX_FILENAME_LENGTH, SEEK_SET);
        fread(&next_block_offset, sizeof(int), 1, disk);
        short temp_id = find_entry(data_block_offset, NULL);
        //Something was in this table
        if(temp_id != -1) {
            add_to_response(BOLDRED "Cannot delete non-empty directory\n"
                    RESET);
            return -1;
        }
    } while((data_block_offset = next_block_offset) != -1);

    //directory is empty, delete its blocks
    data_block_offset = rmdir_block_offset;
    do {
        fseek(disk, data_block_offset + MAX_FILENAME_LENGTH, SEEK_SET);
        fread(&next_block_offset, sizeof(int), 1, disk);
        wipe(disk, data_block_offset, BLOCK_SIZE);
    } while((data_block_offset = next_block_offset) != -1);

    //delete its inode
    wipe(disk, rmdir_inode_offset, INODE_SIZE);

    //delete it from the parent
    data_block_offset = parent_block_offset;
    do {
        fseek(disk, data_block_offset + MAX_FILENAME_LENGTH, SEEK_SET);
        fread(&next_block_offset, sizeof(int), 1, disk);
        if(remove_entry(data_block_offset, name) != -1) {
            commit_disk(disk);
            return 0;
        }
    } while((data_block_offset = next_block_offset) != -1);

    //Who knows what happened
    add_to_response(BOLDRED "Unknown error occurred when deleting directory\n"
            RESET);
    commit_disk(disk);
    return -1;
}

int file_exists(char *name, int *current_dir_inode, int shallow) {
    FILE *disk = access_disk(0);
    fseek(disk, (*current_dir_inode) + 3, SEEK_SET);
    int data_block_offset = 0;
    fread(&data_block_offset, sizeof(int), 1, disk);
    int i;
    int next_block_offset;
    do {
        fseek(disk, data_block_offset + MAX_FILENAME_LENGTH, SEEK_SET);
        fread(&next_block_offset, sizeof(int), 1, disk);
        for (i = 0; i < 4058; i += DIR_TABLE_ENTRY_SIZE) {
            fseek(disk, data_block_offset + i + METADATA_SIZE, SEEK_SET);
            int inode_id = 0;
            char *temp_name = malloc(MAX_FILENAME_LENGTH);
            fread(&inode_id, sizeof(int), 1, disk);
            fread(temp_name, sizeof(char), MAX_FILENAME_LENGTH, disk);
            if (strcmp(name, temp_name) == 0) {
                //Check to see if this is a link or not
                int offset = find_inode_offset(inode_id);
                fseek(disk, offset, SEEK_SET);
                char type;
                fread(&type, sizeof(char), 1, disk);
                int ret_id = inode_id;
                if (type == 'l' && !shallow) {
                    int data_block_offset;
                    fseek(disk, sizeof(short), SEEK_CUR);
                    fread(&data_block_offset, sizeof(int), 1, disk);
                    fseek(disk, data_block_offset + METADATA_SIZE, SEEK_SET);
                    wlog("looking at %d\n", data_block_offset +
                            METADATA_SIZE);
                    fread(&ret_id, sizeof(int), 1, disk);
                }
                commit_disk(disk);
                return ret_id;
            }
        }
    } while((data_block_offset = next_block_offset) != -1);
    commit_disk(disk);
    return -1;
}

int file_create(char *name, short *inode_counter, int *current_dir_inode) {
    short inode_id = inode_create(name, 'f', inode_counter);
    write_dir_data(name, current_dir_inode, inode_id);
    return inode_id;
}

//Create a link `name` pointing to target `src`
void link_create(char *name, char *src,
                 short *inode_counter, int *current_dir_inode) {
    //If the user enters paths:
    //find the target, the link's directory, and the link's name
    int src_inode_offset = expand_path(src, current_dir_inode, 0);
    char *link_parent, *link_name;
    get_parent_path(name, &link_parent);
    get_filename(name, &link_name);

    //The inode the link will be under
    int link_parent_inode_offset = expand_path(link_parent,
            current_dir_inode, 1);

    //The link's inode
    short inode_id = inode_create(link_name, 'l', inode_counter);
    //Add the link to its parent's inode table
    write_dir_data(link_name, &link_parent_inode_offset, inode_id);
    //Search current dir and find the inode id for src
    int src_inode_id = find_inode_id(src_inode_offset);

    //char src_inode_text[33];
    //sprintf(src_inode_text, "%d", src_inode_id);
    //write_data(inode_id, 0, src_inode_text);
    FILE *disk = access_disk(0);
    fseek(disk, find_inode_offset(inode_id) + 3, SEEK_SET);
    int data_block_offset;
    fread(&data_block_offset, sizeof(int), 1, disk);
    fseek(disk, data_block_offset + METADATA_SIZE, SEEK_SET);
    wlog("%d\n", data_block_offset + METADATA_SIZE);
    int ret = fwrite(&src_inode_id, sizeof(int), 1, disk);
    wlog("Wrote %d\n", ret);
    increment_link(src_inode_offset);

    if(strcmp(link_parent, ".") != 0) {
        free(link_parent);
    }
    commit_disk(disk);
}

void increment_link(int target_inode_offset) {
    FILE *disk = access_disk(0);
    int data_block_offset = -1;
    fseek(disk, target_inode_offset + 3 , SEEK_SET);
    fread(&data_block_offset, sizeof(int), 1, disk);
    fseek(disk, data_block_offset + METADATA_SIZE - sizeof(short), SEEK_SET);
    short count = -1;
    fread(&count, sizeof(short), 1, disk);
    count++;
    fseek(disk, -1 * sizeof(short), SEEK_CUR);
    fwrite(&count, sizeof(short), 1, disk);
    commit_disk(disk);
}

void link_remove(char *name, short *inode_counter, int *current_dir_inode) {
    int link_inode_offset = expand_path(name, current_dir_inode, 1);
    if (link_inode_offset == -1) {
        return;
    }
    int link_inode_id = find_inode_id(link_inode_offset);
    FILE *disk = access_disk(0);
    char type = '0';
    fseek(disk, link_inode_offset, SEEK_SET);
    fread(&type, sizeof(char), 1, disk);
    
    //Get the data block offset
    fseek(disk, sizeof(short), SEEK_CUR);
    int data_block_offset;
    fread(&data_block_offset, sizeof(int), 1, disk);

    if (type == 'l') {
        //Remove the link inode   
        //Go to the data block and find the target inode
        //Delete the data block
        //Decrement using target inode
        //Delete that inode and its data if necessary
        wipe(disk, link_inode_offset, INODE_SIZE);
        fseek(disk, data_block_offset + METADATA_SIZE, SEEK_SET);
        wlog("Position of data in link: %d\n",
                data_block_offset + METADATA_SIZE);
        int target_inode_id;
        fread(&target_inode_id, sizeof(int), 1, disk);
        
        //wipe link data
        wipe(disk, data_block_offset, BLOCK_SIZE);
        int target_inode_offset = find_inode_offset(target_inode_id);
        int target_link_count = decrement_link(target_inode_offset);
        if (target_link_count == 0) {
            //Delete the original inode and its data
            int target_data_block_offset;
            fseek(disk, target_inode_offset + 3, SEEK_SET);
            fread(&target_data_block_offset, sizeof(int), 1, disk);
            wipe(disk, target_inode_offset, INODE_SIZE);
            wipe(disk, target_data_block_offset, BLOCK_SIZE);
        }
        //Remove the link from the parent directory
        remove_file_from_dir(disk, *current_dir_inode, link_inode_id);
    }
    else if (type == 'f') {
        int target_link_count = decrement_link(link_inode_offset);
        if (target_link_count == 0) {
            wipe(disk, link_inode_offset, INODE_SIZE);
            wipe(disk, data_block_offset, BLOCK_SIZE);
        }
        //Remove filename from parent directory
        remove_file_from_dir(disk, *current_dir_inode, link_inode_id);
    }
    else {
        add_to_response(BOLDRED "Cannot unlink directories.\n" RESET);
    }
    commit_disk(disk);
    return;
}

void remove_file_from_dir(FILE *disk, int parent_offset, int inode_id) {
    fseek(disk, parent_offset + 3, SEEK_SET);
    int parent_data_block_offset;
    fread(&parent_data_block_offset, sizeof(int), 1, disk);
    int i;
    for (i = 0; i < BLOCK_SIZE - METADATA_SIZE; i += DIR_TABLE_ENTRY_SIZE) {
        fseek(disk, parent_data_block_offset + METADATA_SIZE + i, SEEK_SET);
        int temp_id;
        fread(&temp_id, sizeof(int), 1, disk);
        //wlog("temp_id: %d\tinode_id: %d\n", temp_id, inode_id);
        if (temp_id == inode_id) {
            wipe(disk, parent_data_block_offset + METADATA_SIZE + i,
                    DIR_TABLE_ENTRY_SIZE);//phew
            return;
        }
    }
}

//Dear future us, 
//Sorry for copying this from increment_link. We have no time. 
int decrement_link(int target_inode_offset) {
    FILE *disk = access_disk(0);
    int data_block_offset = -1;
    fseek(disk, target_inode_offset + 3 , SEEK_SET);
    fread(&data_block_offset, sizeof(int), 1, disk);
    fseek(disk, data_block_offset + METADATA_SIZE - sizeof(short), SEEK_SET);
    short count = -1;
    fread(&count, sizeof(short), 1, disk);
    count--;
    fseek(disk, -1 * sizeof(short), SEEK_CUR);
    fwrite(&count, sizeof(short), 1, disk);
    commit_disk(disk);
    return count;
}

void write_data(int fd, int file_offset, char *text) {
    FILE *disk = access_disk(0);
    int inode_offset = find_inode_offset(fd);
    fseek(disk, inode_offset + 3, SEEK_SET);
    int data_block_offset = 0;
    fread(&data_block_offset, sizeof(int), 1, disk);

    //Figure out how many blocks to skip
    //and what file_offset will be in the destination
    int i;
    int dest_file_offset = file_offset % (BLOCK_SIZE - METADATA_SIZE);
    int n_skip_blocks = file_offset / (BLOCK_SIZE - METADATA_SIZE);

    for (i = 0; i < n_skip_blocks; i++) {
        fseek(disk, data_block_offset + MAX_FILENAME_LENGTH, SEEK_SET);
        fread(&data_block_offset, sizeof(int), 1, disk);
        if (data_block_offset == -1) {
            add_to_response(BOLDRED "ERROR: Cannot write past end of file\n"
                    RESET);
            commit_disk(disk);
            return;
        } 
    } 

    fseek(disk, data_block_offset + METADATA_SIZE + dest_file_offset, SEEK_SET);

    int len = strlen(text);
    int free_space = BLOCK_SIZE - METADATA_SIZE - dest_file_offset;
    if (free_space > len) {
        fwrite(text, sizeof(char), len, disk);
        commit_disk(disk);
    }
    else {
        char *section = malloc(free_space);
        strncpy(section, text, free_space);
        fwrite(section, sizeof(char), free_space, disk);
        //Write next block's offset to this block's metadata
        int next_block = block_create("CONTINUED");
        fseek(disk, data_block_offset + MAX_FILENAME_LENGTH, SEEK_SET);
        fwrite(&next_block, sizeof(int), 1, disk);
        commit_disk(disk);
        write_data(fd, file_offset + free_space, text + free_space);
    }
}

void read_data(int fd, int file_offset, int size) {
    int inode_offset = find_inode_offset(fd);
    FILE *disk = access_disk(0);
    fseek(disk, inode_offset + 3, SEEK_SET);
    int data_block_offset = 0;
    fread(&data_block_offset, sizeof(int), 1, disk);
    
    //Figure out how many blocks to skip
    //and what file_offset will be in the destination
    int i;
    int dest_file_offset = file_offset % (BLOCK_SIZE - METADATA_SIZE);
    int n_skip_blocks = file_offset / (BLOCK_SIZE - METADATA_SIZE);

    for (i = 0; i < n_skip_blocks; i++) {
        fseek(disk, data_block_offset + MAX_FILENAME_LENGTH, SEEK_SET);
        int potential_next_block_offset;
        fread(&potential_next_block_offset, sizeof(int), 1, disk);
        if (potential_next_block_offset != -1) {
            data_block_offset = potential_next_block_offset;
        }
    } 
    fseek(disk, data_block_offset + METADATA_SIZE + dest_file_offset, SEEK_SET);

    int this_block_text = BLOCK_SIZE - METADATA_SIZE - dest_file_offset;
    char *text = malloc(this_block_text + 1);
    fread(text, sizeof(char), this_block_text, disk);
    text[this_block_text] = '\0';
    if (!force_printf)
        add_to_response("%s", text);
    else 
        printf("%s", text);

    free(text);

    if (size > this_block_text) {
        int leftover = size - this_block_text;
        //Get next block's offset and read from it
        fseek(disk, data_block_offset + MAX_FILENAME_LENGTH, SEEK_SET);
        fread(&data_block_offset, sizeof(int), 1, disk);
        if (data_block_offset != -1) {
            commit_disk(disk);   
            read_data(fd, file_offset + this_block_text, leftover);
            return;
        }
    }
    commit_disk(disk);
    if (!force_printf)
        add_to_response("\n");
    else
        printf("\n");
}

int expand_path(char *path, int *current_dir_inode, int shallow) {
    if(strcmp(path, ".") == 0) {
        return *current_dir_inode;
    }
    //Adding space support
    //Check if the user gave us quotations
    char *path_start = path;
    if(path[0] == '"') {
        if(path[strlen(path) - 1] != '"') {
            add_to_response(BOLDRED "Mismatched quotation\n" RESET);
            return -1;
        }
        path[strlen(path) - 1] = '\0';
        path_start = path + 1;
    }
    char *token = malloc(MAX_FILENAME_LENGTH);
    token = strtok(path_start, "/\n");
    int temp_dir_inode = *current_dir_inode;
    int data_block_offset = 0;
    while(token != NULL) {
        /* Go to temp inode's block
         * Check if token exists
         * Go to its inode
         */
        wlog("Looking for: %s\n", token);
        int file_check = file_exists(token, &temp_dir_inode, shallow);
        if(file_check == -1) {
            add_to_response(BOLDRED "Invalid path\n" RESET);
            return -1;
        }
        temp_dir_inode = find_inode_offset((short)file_check);
        token = strtok(NULL, "/\n");
    }
    return temp_dir_inode;
}

void get_parent_path(char *path, char **ret) {
    char *last_slash = strrchr(path,'/');
    if(last_slash == NULL) {
        free(last_slash);
        *ret = malloc(2);
        (*ret)[0] = '.';
        (*ret)[1] = '\0';
        return;
    }
    wlog("path: %p\nlast_slash: %p\n", path, last_slash);
    *ret = malloc(last_slash - path + 2);
    strncpy(*ret, path, last_slash - path);
    (*ret)[last_slash - path] = '\0';
}

void get_filename(char *path, char **ret) {
    char *last_slash = strrchr(path, '/');
    if(last_slash == NULL) {
        free(last_slash);
        *ret = strdup(path);
        return;
    }
    last_slash++;
    int length = strlen(path);
    *ret = malloc((path + length) - last_slash + 2);
    strncpy(*ret, last_slash, (path + length) - last_slash + 1);
    (*ret)[(path + length) - last_slash + 1] = '\0';
}


void copy_data(int fd, int file_offset, int size, int dest_fd) {
    int inode_offset = find_inode_offset(fd);
    FILE *disk = access_disk(0);
    fseek(disk, inode_offset + 3, SEEK_SET);
    int data_block_offset = 0;
    fread(&data_block_offset, sizeof(int), 1, disk);
    
    //Figure out how many blocks to skip
    //and what file_offset will be in the destination
    int i;
    int dest_file_offset = file_offset % (BLOCK_SIZE - METADATA_SIZE);
    int n_skip_blocks = file_offset / (BLOCK_SIZE - METADATA_SIZE);

    for (i = 0; i < n_skip_blocks; i++) {
        fseek(disk, data_block_offset + MAX_FILENAME_LENGTH, SEEK_SET);
        int potential_next_block_offset;
        fread(&potential_next_block_offset, sizeof(int), 1, disk);
        if (potential_next_block_offset != -1) {
            data_block_offset = potential_next_block_offset;
        }
    } 
    fseek(disk, data_block_offset + METADATA_SIZE + dest_file_offset, SEEK_SET);

    int this_block_text = BLOCK_SIZE - METADATA_SIZE - dest_file_offset;
    char *text = malloc(this_block_text + 1);
    fread(text, sizeof(char), this_block_text, disk);
    text[this_block_text] = '\0';
            
    char *fdt = malloc(5);
    memset(fdt, 0, 5);
    snprintf(fdt, 5, "%d", dest_fd);
    char *fd_buffer = malloc(strlen(fdt) + strlen(text) + 1);
    memset(fd_buffer, 0, strlen(fdt) + strlen(text) + 1);
    strcpy(fd_buffer, fdt);
    fd_buffer[strlen(fdt)] = ' ';
    strcat(fd_buffer, text);
    write_mm(fd_buffer);
    
    free(text);

    if (size > this_block_text) {
        int leftover = size - this_block_text;
        //Get next block's offset and read from it
        fseek(disk, data_block_offset + MAX_FILENAME_LENGTH, SEEK_SET);
        fread(&data_block_offset, sizeof(int), 1, disk);
        if (data_block_offset != -1) {
            commit_disk(disk);   
            read_data(fd, file_offset + this_block_text, leftover);
            return;
        }
    }
    commit_disk(disk);
}

void print_space(int num, int corner) {
    int i;
    if (num == 0) {
        return;
    }
    for (i = 0; i < num - 2; i++) {
        add_to_response(" ");
    }
    for (i = 0; i < 2; i++) 
    {
        if (corner && !i)
            add_to_response("\xe2\x94\x9c");
        else if(!i)
            add_to_response("\xe2\x94\x9c");
        char *line = "\xe2\x94\x80";
        add_to_response("%s", line);
    }
}

//Iterate through the current directory's data table and list valid files/dirs
void print_tree(int current_dir_inode, int depth) { 
    FILE *disk = access_disk(0);
    //Go directly to the first block pointer for the current inode
    fseek(disk, current_dir_inode + 3, SEEK_SET);
    int data_block_offset = -1;
    fread(&data_block_offset, sizeof(int), 1, disk);
    
    int i;
    char *name = malloc(33);
    int file_counter = 1;
    for (i = 0; i < 4058; i += DIR_TABLE_ENTRY_SIZE) {
        //Go to current entry in inode table
        fseek(disk, data_block_offset + METADATA_SIZE + i, SEEK_SET);
        
        int temp_inode_id = -1;
        fread(&temp_inode_id, sizeof(int), 1, disk);
        fread(name, sizeof(char), MAX_FILENAME_LENGTH, disk);
        if (temp_inode_id == -1) {
            continue;
        }
        if(strlen(name) != 0) {
            //Get the type of this element from its inode
            int temp_inode_offset = find_inode_offset(temp_inode_id);
            fseek(disk,temp_inode_offset, SEEK_SET);
            char type = 0;
            fread(&type, sizeof(char), 1, disk);

            //Get the size of the element
            float my_size = total_size((short) temp_inode_id);
            char magnitude = 0;
            int i;

            while(my_size > BLOCK_SIZE * 2) {
                my_size /= 1024;
                magnitude++;
            }

            
            if (type == 'd') {
                if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
                    print_space(depth, file_counter);
                    add_to_response(BOLDBLUE "%s\n" RESET, name);
                    print_tree(temp_inode_offset, depth + 2);
                    file_counter=0;
                }
            }
            else if (type == 'f') {
                print_space(depth, file_counter);
                //lol why does this have two RESETs
                add_to_response(RESET "%s" RESET, name);
                add_to_response(" " MAGENTA);
                if(magnitude == 0)
                    add_to_response("%d B\n", (int)my_size);
                else if(magnitude == 1)
                    add_to_response("%.1f kB\n", my_size);
                else if(magnitude == 2)
                    add_to_response("%.1f MB\n", my_size);
                else
                    add_to_response("%.1f ?B\n", my_size);
                add_to_response(RESET);
                file_counter=0;
            }
            else if (type == 'l'){
                print_space(depth, file_counter);
                add_to_response(CYAN "%s\n" RESET, name); 
                file_counter=0;
            }
            else {
                add_to_response("Unknown element type %c\n", type);
            }
        }
    }
    commit_disk(disk);
}

void get_name(short inode_id, char **name) {
    if(inode_id < 0) {
        add_to_response(BOLDRED "Invalid inode ID\n" RESET);
        return;
    }
    int inode_offset = find_inode_offset(inode_id);
    FILE *disk = access_disk(0);
    fseek(disk, inode_offset + 3, SEEK_SET);
    int data_block_offset;
    fread(&data_block_offset, sizeof(int), 1, disk);
    fseek(disk, data_block_offset, SEEK_SET);
    *name = malloc(MAX_FILENAME_LENGTH + 1);
    fread(*name, sizeof(char), MAX_FILENAME_LENGTH, disk);
    (*name)[MAX_FILENAME_LENGTH] = '\0';
    commit_disk(disk);
}

char inode_type(short inode_id) {
    if(inode_id < 0) {
        add_to_response(BOLDRED "Invalid inode ID\n" RESET);
        return 0;
    }
    int inode_offset = find_inode_offset(inode_id);
    FILE *disk = access_disk(0);
    fseek(disk, inode_offset, SEEK_SET);
    char type;
    fread(&type, sizeof(char), 1, disk);
    commit_disk(disk);
    return type;
}

short get_link_count(short inode_id) {
    if(inode_id < 0) {
        add_to_response(BOLDRED "Invalid inode ID\n" RESET);
        return 0;
    }
    int inode_offset = find_inode_offset(inode_id);
    FILE *disk = access_disk(0);
    fseek(disk, inode_offset + 3, SEEK_SET);
    int data_block_offset;
    fread(&data_block_offset, sizeof(int), 1, disk);
    fseek(disk, data_block_offset + MAX_FILENAME_LENGTH + sizeof(int),
            SEEK_SET);
    short link_count;
    fread(&link_count, sizeof(short), 1, disk);
    commit_disk(disk);
    return link_count;
}

int block_count(short inode_id) {
    if(inode_id < 0) {
        add_to_response(BOLDRED "Invalid inode ID\n" RESET);
        return 0;
    }
    int inode_offset = find_inode_offset(inode_id);
    FILE *disk = access_disk(0);
    fseek(disk, inode_offset + 3, SEEK_SET);
    int data_block_offset, next_block_offset, count = 0;
    fread(&data_block_offset, sizeof(int), 1, disk);
    do {
        fseek(disk, data_block_offset + MAX_FILENAME_LENGTH, SEEK_SET);
        fread(&next_block_offset, sizeof(int), 1, disk);
        count++;
    } while((data_block_offset = next_block_offset) != -1);

    commit_disk(disk);
    return count;
}

int total_size(short inode_id) {
    if(inode_id < 0) {
        add_to_response(BOLDRED "Invalid inode ID\n" RESET);
        return 0;
    }
    int inode_offset = find_inode_offset(inode_id);
    FILE *disk = access_disk(0);
    fseek(disk, inode_offset + 3, SEEK_SET);
    int data_block_offset, next_block_offset, last_block_offset, count = 0;
    fread(&data_block_offset, sizeof(int), 1, disk);
    do {
        fseek(disk, data_block_offset + MAX_FILENAME_LENGTH, SEEK_SET);
        fread(&next_block_offset, sizeof(int), 1, disk);
        count++;
        last_block_offset = data_block_offset;
    } while((data_block_offset = next_block_offset) != -1);

    //Start from last byte of last block
    //Search backward until we find a non-zero byte
    int i;
    char cur_byte;
    for(i = BLOCK_SIZE - 1; i > METADATA_SIZE; i--) {
        fseek(disk, last_block_offset + i, SEEK_SET);
        fread(&cur_byte, sizeof(char), 1, disk);
        if(cur_byte != 0)
            break;
    }

    commit_disk(disk);
    return (count - 1) * (BLOCK_SIZE - METADATA_SIZE) + (i - METADATA_SIZE);
}
