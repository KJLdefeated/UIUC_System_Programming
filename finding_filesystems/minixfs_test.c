/**
 * finding_filesystems
 * CS 341 - Fall 2024
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void test_create_inode_for_path(file_system* fs) {
    printf("Testing create_inode...\n");
    
    // Test 1: Create new file
    inode* node = minixfs_create_inode_for_path(fs, "/goodies/newfile.txt");
    assert(node != NULL);
    assert(get_inode(fs, "/goodies/newfile.txt") == node);
    
    // Test 2: Attempt to create existing file
    assert(minixfs_create_inode_for_path(fs, "/goodies/newfile.txt") == NULL);
    
    // // Test 3: Create file in non-existent directory
    // assert(minixfs_create_inode_for_path(fs, "/goodies/nonexistent/file.txt") == NULL);

    // // Test 4: Create directory
    // assert(minixfs_create_inode_for_path(fs, "/newdir") != NULL);

    // // Test 5: Create file in existing directory
    // assert(minixfs_create_inode_for_path(fs, "/newdir/file.txt") != NULL);

    // // Test 6: Create multiple files in same directory
    // assert(minixfs_create_inode_for_path(fs, "/newdir/file2.txt") != NULL);

    // // Test 7: Create file in directory in directory
    // assert(minixfs_create_inode_for_path(fs, "/newdir/newdir2/file.txt") != NULL);

    printf("All tests passed!\n");
}

void test_chmod(file_system* fs) {
    printf("Testing chmod...\n");

    // Test 1: Change permissions of file
    inode* node = get_inode(fs, "/goodies/newfile.txt");
    if (node == NULL) {
        node = minixfs_create_inode_for_path(fs, "/goodies/newfile.txt");
    }
    assert(minixfs_chmod(fs, "/goodies/newfile.txt", 0777) == 0);
    assert((node->mode & 0777) == 0777);

    // Test 4: Change different permissions
    node = minixfs_create_inode_for_path(fs, "/newfile.txt");
    assert(minixfs_chmod(fs, "/newfile.txt", 0700) == 0);
    assert((node->mode & 0777) == 0700);

    printf("All tests passed!\n");
}

void test_write(file_system *fs) {
    printf("Testing write...\n");

    // Test 1: Write to empty file
    inode* node = minixfs_create_inode_for_path(fs, "/newfile.txt");
    char* data = "Hello, world!";
    off_t off = 0;
    assert(minixfs_write(fs, "/newfile.txt", data, strlen(data), &off) == (ssize_t)strlen(data));
    assert(node->size == strlen(data));
    assert(off == (off_t)strlen(data));

    // Test 2: Write to non-empty file
    char* data2 = "Goodbye, world!";
    off = 0;
    assert(minixfs_write(fs, "/newfile.txt", data2, strlen(data2), &off) == (ssize_t)strlen(data2));
    assert(node->size == strlen(data2));
    assert(off == (off_t)strlen(data2));

    // Test 3: Write to non-existent file
    assert(minixfs_write(fs, "/nonexistent.txt", data, strlen(data), &off) == (ssize_t)strlen(data));

    printf("All tests passed!\n");
}

void test_read(file_system *fs) {
    printf("Testing read...\n");

    // // Test 1: Read from empty file
    // minixfs_create_inode_for_path(fs, "/newfile.txt");
    // char buf[100];
    // off_t off = 0;
    // assert(minixfs_read(fs, "/newfile.txt", buf, 100, &off) == 0);

    // // Test 2: Read from non-empty file
    // char* data = "Hello, world!";
    // minixfs_write(fs, "/newfile.txt", data, strlen(data), &off);
    // off = 0;
    // assert(minixfs_read(fs, "/newfile.txt", buf, 100, &off) == (ssize_t)strlen(data));
    // assert(strncmp(buf, data, strlen(data)) == 0);

    // Test 3: Read offset
    off_t off = 7;
    char buf[100];
    char* data = "Hello, world!";
    assert(minixfs_read(fs, "/newfile.txt", buf, 100, &off) == (ssize_t)(strlen(data) - 7));
    assert(strncmp(buf, data + 7, strlen(data) - 7) == 0);
    assert(off == (off_t)strlen(data));

    printf("All tests passed!\n");
}

int main(int argc, char *argv[]) {
    // Write tests here!
    file_system *fs = open_fs("test.fs");
    // test_create_inode_for_path(fs);
    // test_chmod(fs);
    // test_write(fs);
    test_read(fs);
    close_fs(&fs);
    return 0;
}
