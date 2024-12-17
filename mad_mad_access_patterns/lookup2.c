/**
 * mad_mad_access_patterns
 * CS 341 - Fall 2024
 */
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "tree.h"
#include "utils.h"

/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.

  ./lookup2 <data_file> <word> [<word> ...]
*/

BinaryTreeNode* readNode (char* fd, uint32_t offset) {
    int wordLength = 0;
    while (fd[offset + sizeof(BinaryTreeNode) + wordLength] != 0) {
        wordLength++;
    }
    BinaryTreeNode* node = malloc(sizeof(BinaryTreeNode) + wordLength);
    memcpy(node, fd + offset, sizeof(BinaryTreeNode) + wordLength);
    return node;
}

uint32_t findWord(char* fd, char* word) {
    uint32_t cur = BINTREE_ROOT_NODE_OFFSET;
    while (cur != 0) {
        BinaryTreeNode* node = readNode(fd, cur);
        int cmp = strcmp(word, node->word);
        if (cmp == 0) {
            free(node);
            return cur;
        } else if (cmp < 0 && node->left_child != 0) {
            cur = node->left_child;
        } else if (node->right_child != 0) {
            cur = node->right_child;
        }
        else {
            free(node);
            return 0;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printArgumentUsage();
        return 1;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        openFail(argv[1]);
        return 2;
    }
    off_t size = lseek(fd, 0, SEEK_END);
    char* file = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (file == MAP_FAILED) {
        mmapFail(argv[1]);
        return 3;
    }
    if (strncmp(file, "BTRE", 4) != 0) {
        formatFail(argv[1]);
        return 2;
    }
    char** words = argv + 2;
    for (int i=0;i<argc-2;i++){
        char* word = words[i];
        uint32_t cur = findWord(file, word);
        if (cur == 0) {
            printNotFound(word);
        } else {
            BinaryTreeNode* node = readNode(file, cur);
            printFound(node->word, node->count, node->price);
            free(node);
        }
    }
    return 0;
}
