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
void create_node(pid_t pid, char **command);
void background_program(char **command);
int count_background_program();
void background_list();
void free_list();

typedef struct bg_process {
pid_t pid;
char command[MAX_PROMPT_LENGTH];
struct bg_process* next;
} bg_process;

bg_process *head = NULL;

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
            background_program(command);
        } else if(strcmp(command[0], "bglist") == 0) {
            background_list();
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
    if(command[1] == NULL || strcmp(command[1], "~") == 0) {
        if(chdir(getenv("HOME")) != 0) {
            perror("cd");
        }
    } else { 
        if(chdir(command[1]) != 0) {
            perror("cd");
        }
    }
}

void create_node(pid_t pid, char **command) {
    bg_process *node = malloc(sizeof(bg_process));
    node->pid = pid;
    node->command[0] = '\0';

    for(int i = 1; command[i] != NULL; i++) {
        strcat(node->command, command[i]);

        if(command[i + 1] != NULL) {
            strcat(node->command, " ");
        }
    }

    node->next = head;
    head = node;
}

void background_program(char **command) {
    if(command[1] == NULL) {
        printf("bg: missing command\n");
        return;
    }
    pid_t pid = fork();
    
    if(pid < 0) {
        perror("fork failed");
    }

    if(pid == 0) {
        execvp(command[1], &command[1]);
        perror("execvp failed");
        exit(1);
    } else {
        create_node(pid, command);
    }
}

int count_background_program() {
    int count = 0;
    bg_process *current = head;

    while(current != NULL) {
        count++;
        current = current->next;
    }

    return count;
}

void background_list() {
    for(bg_process *current = head; current != NULL; current = current->next) {
        printf("%d:  %s\n", current->pid, current->command);
    }

    int count = count_background_program();

    printf("Total Background jobs:  %d\n", count);
}

void free_list() {
    bg_process *current = head;

    while(current != NULL) {
        bg_process *next = current->next;
        free(current);
        current = next;
    }

    head = NULL;
}