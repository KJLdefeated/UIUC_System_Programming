/**
 * perilous_pointers
 * CS 341 - Fall 2024
 */
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here
    first_step(81);
    int value = 132;
    second_step(&value);
    value = 8942;
    int **dvalue = malloc(sizeof(int *));
    *dvalue = &value;
    double_step(dvalue);
    char strange_value[10];
    strange_value[5] = 15;
    strange_step(strange_value);
    char *str = "ABC";
    empty_step(str);
    char *s = "ABCu";
    two_step(s, s);
    char *s1 = "ABCEFG";
    three_step(s1, s1 + 2, s1 + 4);
    char s2[] = "AAAA"; // modifiable
    s2[2] = s2[1] + 8;
    s2[3] = s2[2] + 8;
    step_step_step(s2, s2, s2);
    it_may_be_odd(s1, (int)s1[0]);
    char tok[] = "CS341,CS341,CS341";
    tok_step(tok);
    int *ptr = malloc(sizeof(int));
    *ptr = 0x00001101;
    the_end(ptr, ptr);
    return 0;
}
