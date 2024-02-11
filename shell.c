//
// Created by ali on 1/31/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGUMENTS 1024

void change_directory(char *path) {
    char absolute_path[PATH_MAX];
    char curdir[PATH_MAX];

    if (path != NULL) {
        if (path[0] == '/') {
            if (chdir(path) != 0) {
                perror("cd");
            }
        } else {

            if (getcwd(curdir, sizeof(curdir)) == NULL) {
                perror("getcwd");
                return;
            }

            if (snprintf(absolute_path, sizeof(absolute_path), "%s/%s", curdir, path) < 0) {
                perror("snprintf");
                return;
            }

            if (chdir(absolute_path) != 0) {
                perror("cd");
                return;
            }
        }
    }
}

void print_working_directory() {
    char curdir[PATH_MAX];

    if (getcwd(curdir, sizeof(curdir)) == NULL) {
        perror("getcwd");
        return;
    }

    printf("%s\n", curdir);
}

void list_files() {
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(".")) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (strncmp(entry->d_name, ".", 1) != 0) {
                printf("%s\t", entry->d_name);
            }
        }
        printf("\n");
        closedir(dir);
    } else {
        perror("opendir");
    }
}

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void execute_file(char *file) {
    pid_t child_pid;
    int status;

    child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {
        char *args[] = {file, NULL};
        execvp(file, args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        waitpid(child_pid, &status, 0);
    }
}

void echo (char *tokens[MAX_ARGUMENTS + 2]) {

    char message[1024] = "";
    const char *fileName = NULL;

    int toRead = 0;

    for (int i = 1; tokens[i] != NULL; ++i) {
        if (strcmp(tokens[i], ">") == 0) {
            fileName = tokens[i + 1];
            break;
        } else if (strcmp(tokens[i], "<") == 0) {
            fileName = tokens[i + 1];
            toRead = 1;
        } else {
            strcat(message, tokens[i]);
            strcat(message, " ");
        }
    }

    message[strlen(message) - 1] = '\0';

    if (fileName == NULL) {
        printf("%s\n", message);
    } else if (!toRead) {
        FILE *file = fopen(fileName, "w");

        if (file != NULL) {
            fprintf(file, "%s\n", message);
            fclose(file);
        } else {
            perror("Error opening file");
        }
    } else {

        int c;
        FILE *file = fopen(fileName, "r");
        if (file) {
            while ((c = fgetc(file)) != EOF) {
                printf("%c", c);
            }
            fclose(file);
        }
    }
}

void make_directory(char *path) {
    char absolute_path[PATH_MAX];
    char curdir[PATH_MAX];

    if (path != NULL) {
        if (path[0] == '/') {
            if (mkdir(path, 0777) != 0) {
                perror("mkdir");
            } else {
                strcpy(curdir, absolute_path);
            }
        } else {
            if (getcwd(curdir, sizeof(curdir)) == NULL) {
                perror("getcwd");
                return;
            }

            if (snprintf(absolute_path, sizeof(absolute_path), "%s/%s", curdir, path) < 0) {
                perror("snprintf");
                return;
            }

            if (mkdir(absolute_path, 0777) != 0) {
                perror("mkdir");
                return;
            }
        }
    }
}

void create_file(char *tokens[MAX_ARGUMENTS + 2]) {

    for (int i = 1; tokens[i] != NULL; ++i) {
        FILE *fptr;
        fptr = fopen(tokens[i], "w");
        fclose(fptr);
    }
}

void handle_command(char *tokens[MAX_ARGUMENTS + 2]) {

    if (strcmp(tokens[0], "cd") == 0) {
        change_directory(tokens[1]);
    } else if (strcmp(tokens[0], "mkdir") == 0) {
        make_directory(tokens[1]);
    } else if (strcmp(tokens[0], "ls") == 0) {
        list_files();
    } else if (strcmp(tokens[0], "pwd") == 0) {
        print_working_directory();
    } else if (strcmp(tokens[0], "clear") == 0) {
        clear_screen();
    } else if (strncmp(tokens[0], "./", 2) == 0) {
        execute_file(tokens[0]);
    } else if (strcmp(tokens[0], "echo") == 0) {
        echo(tokens);
    } else if (strcmp(tokens[0], "touch") == 0) {
        create_file(tokens);
    } else {
        printf("Unknown command: %s\n", tokens[0]);
    }
}

void execute_command(char *input) {
    char *token;
    char *tokens[MAX_ARGUMENTS + 2];

    int i = 0;
    token = strtok(input, " ");

    while (token != NULL && i < MAX_ARGUMENTS + 1) {
        tokens[i++] = token;
        token = strtok(NULL, " ");
    }

    tokens[i] = NULL;

    if (tokens[0] != NULL) {
        handle_command(tokens);
    }
}

int main() {
    char input[MAX_INPUT_SIZE];

    while (1) {
        printf("MyShell> ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Error reading input\n");
            exit(EXIT_FAILURE);
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0) {
            printf("Exiting the shell...\n");
            break;
        }

        execute_command(input);
    }

    return 0;
}
