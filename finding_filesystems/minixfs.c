/**
 * finding_filesystems
 * CS 341 - Fall 2024
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string,
             "Free blocks: %zd\n"
             "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    inode *node = get_inode(fs, path);
    if (!node) {
        errno = ENOENT;
        return -1;
    }
    int type = node->mode >> RWX_BITS_NUMBER;
    node->mode = (new_permissions & 0777) | (type << RWX_BITS_NUMBER);
    clock_gettime(CLOCK_REALTIME, &node->ctim);
    return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    inode *node = get_inode(fs, path);
    if (!node) {
        errno = ENOENT;
        return -1;
    }
    if (owner != ((uid_t)-1))
        node->uid = owner;
    if (group != ((gid_t)-1))
        node->gid = group;
    clock_gettime(CLOCK_REALTIME, &node->ctim);
    return 0;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    // return NULL if inode already exists or cannot be created.
    if (get_inode(fs, path)){
        return NULL;
    }
        
    // find its parent inode
    const char *filename;
    inode *parent = parent_directory(fs, path, &filename);
    if (!parent){
        return NULL;
    }
    // write a dirent to disk
    minixfs_dirent new_dirent;
    new_dirent.inode_num = first_unused_inode(fs);
    new_dirent.name = strdup(filename);
    if (new_dirent.inode_num == -1 || !new_dirent.name){
        #ifdef DEBUG
        printf("No more inodes\n");
        #endif
        return NULL;
    }

    // Init inode
    inode *new_node = fs->inode_root + new_dirent.inode_num;
    init_inode(parent, new_node);
    
    // Add the new dirent to the parent
    data_block_number block_num = parent->size / sizeof(data_block);
    size_t block_offset = parent->size % sizeof(data_block);
    data_block_number *block_array = parent->direct;

    if (block_num < NUM_DIRECT_BLOCKS) {
        if (block_array[block_num] == UNASSIGNED_NODE) {
            block_array[block_num] = add_data_block_to_inode(fs, parent);
            if (block_array[block_num] == -1) {
                errno = ENOSPC;
                return NULL;
            }
        }
    }
    else {
        size_t indirect_index = block_num - NUM_DIRECT_BLOCKS;
        if (parent->indirect == UNASSIGNED_NODE) {
            parent->indirect = add_single_indirect_block(fs, parent);
            if (parent->indirect == -1) {
                errno = ENOSPC;
                return NULL;
            }
        }
        block_array = (data_block_number *)(fs->data_root + parent->indirect);
        if (block_array[indirect_index] == UNASSIGNED_NODE) {
            block_array[indirect_index] = add_data_block_to_indirect_block(fs, block_array);
            if (block_array[indirect_index] == -1) {
                errno = ENOSPC;
                return NULL;
            }
        }
        block_num = indirect_index;
    }

    make_string_from_dirent((char *)(fs->data_root + block_array[block_num]) + block_offset, new_dirent);

    // increase the parent's size
    parent->size += FILE_NAME_ENTRY;
    return new_node;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        size_t used_blocks = 0;

        for (size_t i = 0; i < fs->meta->dblock_count; i++) {
            if (get_data_used(fs, i))
                used_blocks++;
        }

        char *info_string = block_info_string(used_blocks);
        size_t len = strlen(info_string);

        if ((size_t)*off >= len)
            return 0;
        
        size_t byte_left = len - *off;
        size_t byte_to_read = count < byte_left ? count : byte_left;

        memcpy(buf, info_string + *off, byte_to_read);
        *off += byte_to_read;
        // free(info_string);
        return byte_to_read;
    }

    errno = ENOENT;
    return -1;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf, size_t count, off_t *off) {
    // calculate the maximum possible filesize
    size_t max_file_size = (NUM_INDIRECT_BLOCKS + NUM_DIRECT_BLOCKS * NUM_DIRECT_BLOCKS) * sizeof(data_block);
    if (*off + count > max_file_size) {
        errno = ENOSPC;
        return -1;
    }

    // find the inode
    inode *node = get_inode(fs, path);
    if (!node) {
        node = minixfs_create_inode_for_path(fs, path);
        if (!node) {
            errno = ENOENT;
            return -1;
        }
    }

    // Calulate the number of blocks needed
    size_t blocks_needed = (count + *off) / sizeof(data_block) + ((count + *off) % sizeof(data_block) != 0);
    if (minixfs_min_blockcount(fs, path, blocks_needed) == -1) {
        errno = ENOSPC;
        return -1;
    }

    // Write the data by blocks
    size_t written = 0;
    size_t cur_pos = *off;
    const char *write_buf = (const char *)buf;

    while(written < count) {
        size_t block_num = cur_pos / sizeof(data_block);
        size_t block_offset = cur_pos % sizeof(data_block);

        data_block_number *block_array = node->direct;
        data_block_number block;
        if (block_num < NUM_DIRECT_BLOCKS) {
            // Direct block
            if (block_array[block_num] == UNASSIGNED_NODE) {
                block_array[block_num] = add_data_block_to_inode(fs, node);
                if (block_array[block_num] == -1) {
                    errno = ENOSPC;
                    return -1;
                }
            }
            block = block_array[block_num];
        }
        else {
            // Indirect block
            size_t indirect_index = block_num - NUM_DIRECT_BLOCKS;

            if (node->indirect == UNASSIGNED_NODE) {
                node->indirect = add_single_indirect_block(fs, node);
                if (node->indirect == -1) {
                    errno = ENOSPC;
                    return -1;
                }
            }
            block_array = (data_block_number *)(fs->data_root + node->indirect);
            if (block_array[indirect_index] == UNASSIGNED_NODE) {
                block_array[indirect_index] = add_data_block_to_indirect_block(fs, block_array);
                if (block_array[indirect_index] == -1) {
                    errno = ENOSPC;
                    return -1;
                }
            }
            block = block_array[indirect_index];
        }

        size_t remaining_in_block = sizeof(data_block) - block_offset;
        size_t remaining_to_write = count - written;
        size_t to_write = remaining_in_block < remaining_to_write ? remaining_in_block : remaining_to_write;

        memcpy(fs->data_root + block + block_offset, write_buf + written, to_write);
        written += to_write;
        cur_pos += to_write;
    }

    // Update the file size
    if (*off+count > node->size)
        node->size = *off+count;
    
    // Update the access time
    clock_gettime(CLOCK_REALTIME, &node->atim);
    clock_gettime(CLOCK_REALTIME, &node->mtim);

    *off = cur_pos;

    return written;
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    // Get the inode
    inode *node = get_inode(fs, path);
    if (!node) {
        errno = ENOENT;
        return -1;
    }
    if ((size_t)*off >= node->size)
        return 0;
    if (is_directory(node)) {
        errno = EISDIR;
        return -1;
    }
    // Calculate the number of bytes to read
    size_t left_to_read = node->size - *off;
    size_t to_read = count < left_to_read ? count : left_to_read;
    // Read the data by blocks
    char* read_buf = (char*)buf;
    size_t read = 0;
    size_t cur_pos = *off;

    while(read < to_read) {
        size_t block_num = cur_pos / sizeof(data_block);
        size_t block_offset = cur_pos % sizeof(data_block);

        data_block_number block;
        if (block_num < NUM_DIRECT_BLOCKS) {
            block = node->direct[block_num];
        }
        else {
            size_t indirect_index = block_num - NUM_DIRECT_BLOCKS;
            data_block_number *block_array = (data_block_number *)(fs->data_root + node->indirect);
            block = block_array[indirect_index];
        }
        size_t remaining_in_block = sizeof(data_block) - block_offset;
        size_t remaining_to_read = to_read - read;
        size_t to_read_block = remaining_in_block < remaining_to_read ? remaining_in_block : remaining_to_read;

        memcpy(read_buf + read, (char *)(fs->data_root + block) + block_offset, to_read_block);

        read += to_read_block;
        cur_pos += to_read_block;
    }

    clock_gettime(CLOCK_REALTIME, &node->atim);

    *off = cur_pos;

    return read;
}
