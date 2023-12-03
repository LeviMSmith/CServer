#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void utils_generate_sessid(char *sessid) {
    const char *chars = "0123456789abcdef";
    int i, r;

    srand((unsigned int) time(0)); // Initialize random number generator

    for (i = 0; i < 36; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            sessid[i] = '-';
        } else {
            r = rand() % 16;
            sessid[i] = chars[r];
        }
    }

    sessid[36] = '\0'; // Null-terminate the string
}
