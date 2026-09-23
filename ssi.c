#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>

#define MAX_PROMPT_LENGTH 100

void print_prompt();

int main() {
    return 0;
}

void print_prompt() {
    char* username = (char*)malloc(50);
    char hostname[50]; 
    char directory[50];

    username = getlogin();
    gethostname(hostname, sizeof(hostname));
    getcwd(directory, sizeof(directory));

    free(username);
}