/**
 * extreme_edge_cases
 * CS 341 - Fall 2024
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

int check_output(char **output, const char **correct) {
    if (correct == NULL) {
        return output == NULL;
    }
    int i = 0;
    while (correct[i] != NULL) {
        if (output[i] == NULL || strcmp(output[i], correct[i]) != 0) {
            return 0;
        }
        i++;
    }
    if (output[i] != NULL) {
        return 0;
    }
    return 1;
}

int test_NULL(char **(*camelCaser)(const char *), void (*destroy)(char **)){
    const char* input = NULL;
    char** result = camelCaser(input);
    const char** answer = NULL;
    int ans = check_output(result, answer);
    destroy(result);
    return ans;
}

int test_single_sentence(char **(*camelCaser)(const char *), void (*destroy)(char **)){
    const char* input = "I am Batman.";
    char** result = camelCaser(input);
    const char* answer[2] = {"iAmBatman", NULL};
    int ans = check_output(result, answer);
    destroy(result);
    return ans;
}

int test_multiple_sentence(char **(*camelCaser)(const char *), void (*destroy)(char **)){
    const char* input = "The Heisenbug is an incredible creature. Facenovel servers get their power from its indeterminism. Code smell can be ignored with INCREDIBLE use of air freshener. God objects are the new religion.";
    char** result = camelCaser(input);
    const char* answer[5] = {
        "theHeisenbugIsAnIncredibleCreature",
        "facenovelServersGetTheirPowerFromItsIndeterminism",
        "codeSmellCanBeIgnoredWithIncredibleUseOfAirFreshener",
        "godObjectsAreTheNewReligion",
        NULL
    };
    int ans = check_output(result, answer);
    destroy(result);
    return ans;
}

int test_special_character(char **(*camelCaser)(const char *), void (*destroy)(char **)){
    const char* input = "Hello! How are you? I'm fine, thank you.";
    char** result = camelCaser(input);
    const char* answer[6] = {
        "hello", 
        "howAreYou", 
        "i", 
        "mFine", 
        "thankYou",
        NULL
    };
    int ans = check_output(result, answer);
    destroy(result);
    return ans;
}

int test_mid_word_space(char **(*camelCaser)(const char *), void (*destroy)(char **)){
    const char* input = "Hello. World";
    char** result = camelCaser(input);
    const char* answer[2] = {
        "hello",
        NULL
    };
    int ans = check_output(result, answer);
    destroy(result);
    return ans;
}

int test_start_punc(char **(*camelCaser)(const char *), void (*destroy)(char **)){
    const char* input = ".Hello World.";
    char** result = camelCaser(input);
    const char* answer[3] = {
        "",
        "helloWorld",
        NULL
    };
    int ans = check_output(result, answer);
    destroy(result);
    return ans;
}

int test_empty(char **(*camelCaser)(const char *), void (*destroy)(char **)){
    const char* input = "";
    char** result = camelCaser(input);
    const char* answer[1] = {NULL};
    int ans = check_output(result, answer);
    destroy(result);
    return ans;
}

int test_all_punc(char **(*camelCaser)(const char *), void (*destroy)(char **)){
    const char* input = "?.!@#$^&*()_+";
    char** result = camelCaser(input);
    const char* answer[14] = {"","","","","","","","","","","","","",NULL};
    int ans = check_output(result, answer);
    destroy(result);
    return ans;
}

int test_single_word(char **(*camelCaser)(const char *), void (*destroy)(char **)){
    const char* input = "Hello";
    char** result = camelCaser(input);
    const char* answer[1] = {NULL};
    int ans = check_output(result, answer);
    destroy(result);
    return ans;
}

int  test_non_ASCII(char **(*camelCaser)(const char *), void (*destroy)(char **)){
    const char* input = "你好，世界！. Hello, World hi!";
    char** result = camelCaser(input);
    const char* answer[4] = {
        "你好，世界！",
        "hello",
        "worldHi",
        NULL
    };
    int ans = check_output(result, answer);
    destroy(result);
    return ans;
}

int test_All_ASCII(char **(*camelCaser)(const char *), void (*destroy)(char **)){
    const char* input = "#$fwlm,&w,ldldl,ml'()*+,-./012dokdwmo34few,m56789:;<=>?@ABCDEFGHIJKLMNO!PQRSTUV!WXYZ[^]_`abcd.!efghijklmnopqrstuvwxyz{|}~!";
    char** result = camelCaser(input);
    const char* answer[38] = {"","","fwlm","","w","ldldl","ml","","","","","","","","","012dokdwmo34few","m56789","","","","","","","abcdefghijklmno","pqrstuv","wxyz","","","","","abcd","","efghijklmnopqrstuvwxyz","","","","",NULL};
    int ans = check_output(result, answer);
    destroy(result);
    return ans;
}

int test_ASCII(char **(*camelCaser)(const char *), void (*destroy)(char **)){
    const char* input = "Hello\x01Wo.rld\x02TestC.afé\x80 au\x90 l.ait\xA0He.lloWorld\x1FTest.word1 word2\tword3\nword4.12345@#$%^.";
    char** result = camelCaser(input);
    const char* answer[13] = {
        "hello\001wo",
        "rld\002testc",
        "afé\200Au\220L",
        "ait\240he",
        "lloworld\x1Ftest",
        "word1Word2Word3Word4",
        "12345",
        "",
        "",
        "",
        "",
        "",
        NULL
    };
    int ans = check_output(result, answer);
    destroy(result);
    return ans;
}

int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    // TODO: Implement me!
    // Test
    if (!test_NULL(camelCaser, destroy)) {
        //printf("Test NULL failed\n");
        return 0;
    }
    if (!test_single_sentence(camelCaser, destroy)){
        //printf("Test single sentence failed\n");
        return 0;
    }
    if (!test_multiple_sentence(camelCaser, destroy)){
        //printf("Test multiple sentence failed\n");
        return 0;
    }
    if (!test_special_character(camelCaser, destroy)){
        //printf("Test special character failed\n");
        return 0;
    }
    if (!test_mid_word_space(camelCaser, destroy)){
        //printf("Test mid word space failed\n");
        return 0;
    }
    if (!test_start_punc(camelCaser, destroy)){
        //printf("Test start punc failed\n");
        return 0;
    }
    if (!test_empty(camelCaser, destroy)){
        //printf("Test empty failed\n");
        return 0;
    }
    if (!test_all_punc(camelCaser, destroy)){
        //printf("Test all punc failed\n");
        return 0;
    }
    if (!test_single_word(camelCaser, destroy)){
        //printf("Test single word failed\n");
        return 0;
    }
    if (!test_non_ASCII(camelCaser, destroy)){
        //printf("Test non ASCII failed\n");
        return 0;
    }
    if (!test_All_ASCII(camelCaser, destroy)){
        //printf("Test all ASCII failed\n");
        return 0;
    }
    if (!test_ASCII(camelCaser, destroy)){
        //printf("Test ASCII failed\n");
        return 0;
    }
    return 1;
}
