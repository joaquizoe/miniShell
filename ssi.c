/**
 * @brief P1: A Simple Shell Interpreter (SSI)
 * @author Joaqui Santiago
 * @date September 18, 2026
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>

#define MAX_PROMPT_LENGTH 256

/* Function Declarations */
void handler(int sig);
void create_prompt(char *prompt);
void tokenize(char **args, char* response);
void basic_program(char **args);
void change_directory(char **command);
void run_program(char **args);
void create_node(pid_t pid, char **command);
void background_program(char **command);
int count_background_program();
void background_list();
void free_list();
void check_background();
char *get_path_access(char *name);

/**
 * @brief Represents a background program in the linked list
 * Source: Tutorial 2
 */
typedef struct bg_process {
pid_t pid;
char command[MAX_PROMPT_LENGTH];
struct bg_process* next;
} bg_process;

/* Initiate the head of the linked list as NULL */
bg_process *head = NULL;

int main() {
    /* Source: https://man7.org/linux/man-pages/man2/sigaction.2.html */
    /*         https://www.geeksforgeeks.org/c/signals-c-language/ */
    struct sigaction sa = {0};
    sa.sa_handler = &handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if(sigaction(SIGINT, &sa, NULL) == -1){
        perror("sigaction");
        exit(1);
    }

    while(1) {
        /* Check if any background programs are currently running */
        check_background();

        char *args[MAX_PROMPT_LENGTH];
        char prompt[MAX_PROMPT_LENGTH];

        create_prompt(prompt);

        char *response = readline(prompt);

        /* If response is NULL, terminate the program */
        if(response == NULL) {
            free_list();
            exit(0);
        }
        /* If user presses Enter without a command, prompt again */
        if(response[0] == '\0') {
            free(response);
            continue;
        }

        /* Tokenize the user's response */
        tokenize(args, response);

        /* Run the program after tokenize */
        run_program(args);

        /* Free the user's response */
        free(response);
    }

    return 0;
}

/**
 * @brief signal hander for CTRL + C without terminating ssi
 * @param sig The signal number received
 */
void handler(int sig) {
    write(STDOUT_FILENO, "\n", 1);
}

/**
 * @brief Creates a prompt that includes username, hostname, and directory
 * @param prompt The string where the created prompt will be stored
 */
void create_prompt(char *prompt) {
    char *username;
    char hostname[50]; 
    char directory[50];

    username = getlogin();
    gethostname(hostname, sizeof(hostname));
    getcwd(directory, sizeof(directory));

    snprintf(prompt, MAX_PROMPT_LENGTH, "%s@%s: %s > ", username, hostname, directory);
}

/**
 * @brief Splits the user's response into separated arguments
 * @param args The array where the separated command arguments are stored
 * @param response The string containing the user's response
 */
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

/**
 * @brief Determines which command was entered and executes it
 * @param command The array containing the command and its arguments
 */
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

/**
 * @brief Executes the basic programs 
 * @param command The array containing the basic command and its arguments
 */
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

/**
 * @brief Changes the current directory 
 * @param command The array containing the change directory command and its arguments
 */
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

/**
 * @brief Creates a node containing a background program
 * @param pid The process ID of the background program
 * @param command The array containing the background program and its arguments
 */
void create_node(pid_t pid, char **command) {
    bg_process *node = malloc(sizeof(bg_process));
    node->pid = pid;
    node->command[0] = '\0';

    char *path = get_path_access(command[1]);

    if(path != NULL) {
        strcat(node->command, path);
        free(path);
    } else {
        strcat(node->command, command[1]);
    }

    for(int i = 2; command[i] != NULL; i++) {
        strcat(node->command, " ");
        strcat(node->command, command[i]);
    }

    node->next = NULL;

    if(head == NULL) {
        head = node;
    } else {
        bg_process *current = head;

        while(current->next != NULL) {
            current = current->next;
        }

        current->next = node;
    }
}

/**
 * @brief Executes the program in the background
 * @param command The array containing the background program and its arguments
 */
void background_program(char **command) {
    if(command[1] == NULL) {
        printf("bg: missing command\n");
        return;
    }
    pid_t pid = fork();
    
    if(pid < 0) {
        perror("fork failed");
        return;
    }

    if(pid == 0) {
        execvp(command[1], &command[1]);
        perror("execvp failed");
        exit(1);
    } else {
        create_node(pid, command);
    }
}

/**
 * @brief Counts the current number of background programs
 * @return The number of background programs
 */
int count_background_program() {
    int count = 0;
    bg_process *current = head;

    while(current != NULL) {
        count++;
        current = current->next;
    }

    return count;
}

/**
 * @brief Shows the list of background programs currently running
 */
void background_list() {
      int count = count_background_program();

    for(bg_process *current = head; current != NULL; current = current->next) {
        printf("%d: %s\n", current->pid, current->command);
    }

    printf("Total Background jobs: %d\n", count);
}

/**
 * @brief Frees the allocated space for the background programs
 */
void free_list() {
    bg_process *current = head;

    while(current != NULL) {
        bg_process *next = current->next;
        free(current);
        current = next;
    }

    head = NULL;
}

/**
 * @brief Checks if any background jobs have been terminated
 */
void check_background() {
    /* If there are no background jobs, return to main program */
    if(count_background_program() == 0) {
        return;
    }

    bg_process *current = head;
    bg_process *previous = NULL;

    while(current != NULL) {
        /* If background program has terminated, returns its pid, otherwise return 0 */
        /* Source: https://www.geeksforgeeks.org/c/wait-system-call-c/ */
        pid_t result = waitpid(current->pid, NULL, WNOHANG);

        if(result > 0) {
            printf("%d: %s has terminated.\n", current->pid, current->command);

            bg_process *temp = current;

            /* If the terminated program is the head, make the next program the head */
            if(previous == NULL) {
                head = current->next;
                current = head;
            } else {
                previous->next = current->next;
                current = current->next;
            }

            free(temp);     // Free the background job that terminated
        } else {
            previous = current;
            current = current->next;
        }
    }
}

/**
 * @brief Gets the full executable path of a background command
 * @param name Name of the command
 * @return The full path of  the command, Null otherwise
 * Source: Tutorial 3
 */
char *get_path_access(char *name) {
    if(strchr(name, '/') != NULL) {
        return realpath(name, NULL);
    }

    char *dirs = strdup(getenv("PATH"));

    // printf("%s\n", dirs);
    
    for(char *dir = strtok(dirs, ":"); dir != NULL; dir = strtok(NULL, ":")) {
        char buffer[MAX_PROMPT_LENGTH];
        snprintf(buffer, sizeof(buffer), "%s/%s", dir, name);

        if(access(buffer, X_OK) == 0) {
            free(dirs);
            return strdup(buffer);
        }
    }

    free(dirs);
    return NULL;
}