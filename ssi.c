#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>

#define MAX_PROMPT_LENGTH 100

void print_prompt();
void tokenize(char **args, char* response);
void basic_program(char **args);
void change_directory(char **command);
void run_program(char **args);

int main() {

    while(1) {
        char *args[MAX_PROMPT_LENGTH];

        print_prompt();

        char *response = readline("");

        /* If response is NULL, terminate the program */
        if(response == NULL) {
            exit(0);
        }
        /* If response is Enter, create a new line */
        if(response[0] == '\0') {
            free(response);
            continue;
        }

        /* Tokenize the user's response */
        tokenize(args, response);

        /* Run the program after tokenize */
        run_program(args);
    }

    return 0;
}

void print_prompt() {
    char *username;
    char hostname[50]; 
    char directory[50];

    username = getlogin();
    gethostname(hostname, sizeof(hostname));
    getcwd(directory, sizeof(directory));

    printf("%s@%s: %s > ", username, hostname, directory);
}

void tokenize(char **args, char *response) {
    int i = 0;
    char* token = strtok(response, " ");

    while(token != NULL) {
        args[i] = token;
        token = strtok(NULL, " ");
        i++;
    }

    args[i] = NULL;
}

void run_program(char **command) {
    if(strcmp(command[0], "cd") == 0) {
            change_directory(command);
        } else if(strcmp(command[0], "bg") == 0) {
            exit(1);
        } else if(strcmp(command[0], "bglist") == 0) {
            exit(1);
        } else {
            basic_program(command);
        }
    
}

void basic_program(char **command) {
    pid_t pid = fork();

    if(pid < 0) {
        perror("fork failed");
        exit(1);
    }
    if(pid == 0) {
        execvp(command[0], command);
        perror("execvp failed");
        exit(1);
    } else {
        waitpid(pid, NULL, 0);
    }
}

void change_directory(char **command) {
    if(command[1] == NULL) {
        chdir(getenv("HOME"));
    } else if(strcmp(command[1], "~") == 0) {
        chdir(getenv("HOME"));
    } else{
        chdir(command[1]);
    }
}