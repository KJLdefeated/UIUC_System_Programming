/**
 * mad_mad_access_patterns
 * CS 341 - Fall 2024
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tree.h"
#include "utils.h"
/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/

BinaryTreeNode* readNode (FILE* fd, uint32_t offset) {
    fseek(fd, offset + sizeof(BinaryTreeNode), SEEK_SET);
    int wordLength = 0;
    while (fgetc(fd) != 0) {
        wordLength++;
    }
    BinaryTreeNode* node = malloc(sizeof(BinaryTreeNode) + wordLength);
    fseek(fd, offset, SEEK_SET);
    fread(node, sizeof(BinaryTreeNode) + wordLength, 1, fd);
    return node;
}

uint32_t findWord(FILE* fd, char* word) {
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
    FILE* fd = fopen(argv[1], "r");
    if (fd == NULL) {
        openFail(argv[1]);
        return 2;
    }
    fseek(fd, 0, SEEK_SET);
    char magic[4];
    fread(magic, 1, 4, fd);
    if (strncmp(magic, "BTRE", 4) != 0) {
        formatFail(argv[1]);
        return 2;
    }
    char** words = argv + 2;
    for (int i=0; i < argc-2; i++) {
        char* word = words[i];
        uint32_t offset = findWord(fd, word);
        if (offset == 0) {
            printNotFound(word);
        } else {
            BinaryTreeNode* node = readNode(fd, offset);
            printFound(word, node->count, node->price);
            free(node);
        }
    }
    return 0;
}
