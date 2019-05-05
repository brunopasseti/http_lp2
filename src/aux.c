#include "../includes/aux.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



char* ReadFile(const char* PATH, int *len){
    char *buffer = NULL;
    size_t length;
    FILE* fp = fopen(PATH, "rb");
    if(fp){
        fseek (fp, 0, SEEK_END);
        length = ftell (fp);
        fseek (fp, 0, SEEK_SET);
        buffer = malloc (length+1);
        if (buffer){
            int a = fread (buffer, 1, length, fp);
        }
        fclose (fp);
        // puts(buffer);
    }
    if(buffer){
        *len = length;
        // puts(buffer);
        return buffer;
    }
    else return NULL;
}

char* getFileName(char* request){
    char* rest = request;
    char *tkn;
    while ((tkn = strtok_r(rest, " ", &rest))){
        if(tkn[0] == '/') break;
    }
    // puts(tkn);
    return tkn;
}