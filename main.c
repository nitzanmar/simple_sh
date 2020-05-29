#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TOKEN_DELIM " \t\r\n\a"
#define TOKEN_BUFSIZE 64

typedef enum bool { false, true } bool;

/*
 * main routine funcs declarations
 */
void main_loop(void);
char* read_line(void);
char** parse_args(char* line);
bool exec_args(char **args);
bool exec_nonbuiltin_cmd(char** args);

/* 
 * "builtins" section
 */
bool builtin_help(char **args);
bool builtin_exit(char **args);
bool builtin_cd(char **args);

// the builtins arrays must keep the same index for builtin name and func pointer
char const *builtins[] = { "help", "exit", "cd" };
bool (* const builtins_funcs[])(char **) = { &builtin_help, &builtin_exit, &builtin_cd };
int const num_builtins = sizeof(builtins) / sizeof(char *);

int main(void)
{
    main_loop();
    return EXIT_SUCCESS;
}

bool exec_args(char **args)
{
    if (args[0] == NULL) {
        return true;
    }

    for (int i = 0; i < num_builtins; ++i) {
        if (strcmp(args[0], builtins[i]) == 0) {
            return (*builtins_funcs[i])(args);
        }
    }

    return exec_nonbuiltin_cmd(args);
}

void main_loop(void)
{
    char *line;
    char **args;
    enum bool should_continue;

    do {
        printf("%s", "$> ");
        line = read_line();
        args = parse_args(line);
        should_continue = exec_args(args);

        if (line) {
            free(line);
        }

        if (args) {
            free(args);
        }
    } while (should_continue);
}

char* read_line(void)
{
    char *line = NULL;
    size_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        } else {
            perror("Failed reding line from prompt");
            exit(EXIT_FAILURE);
        }
    }

    return line;
}

char** parse_args(char* line)
{
    size_t max_args = TOKEN_BUFSIZE;
    size_t curr_idx = 0;
    char **tokens = malloc(sizeof(char*) * max_args);

    if (!tokens) {
        fprintf(stderr, "failed to allocate while parsing args\n");
        exit(EXIT_FAILURE);
    }

    char *token = strtok(line, TOKEN_DELIM);

    while (token != NULL) {
        tokens[curr_idx] = token;
        ++curr_idx;

        if (curr_idx >= max_args) {
            max_args += TOKEN_BUFSIZE;
            tokens = realloc(tokens, sizeof(char*) * max_args);

            if (!tokens) {
                fprintf(stderr, "failed to allocate while parsing args\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, TOKEN_DELIM);
    }

    tokens[curr_idx] = NULL;

    return tokens;
}

bool exec_nonbuiltin_cmd(char** args)
{
    int status;
    pid_t wpid;
    pid_t pid = fork();

    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("Child failed running cmd");
        }

        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("failed forking");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return true;
}

bool builtin_help(char **args)
{
    printf("%s\n%s\n%s\n", "simple_sh (more like stupid shell).",
            "Type your command and args, and press ENTER to execute.",
            "The following commands are shell builtins:");

    for (int i = 0; i < num_builtins; ++i) {
        printf("\t%s\n", builtins[i]);
    }

    printf("Use the man command for information on other programs.\n");

    return true;
}

bool builtin_exit(char **args)
{
    return false;
}

bool builtin_cd(char **args)
{
    if (args[0] == NULL) {
        fprintf(stderr, "%s\n", "directory path is expected in `cd` command");
        return true;
    }

    if (chdir(args[1]) != 0) {
        char err[1024];
        sprintf(err, "Failed to change dir to - %s", args[1]);
        perror(err);
    }

    return true;
}

