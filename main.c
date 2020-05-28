#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TOKEN_DELIM " \t\r\n\a"
#define TOKEN_BUFSIZE 64

enum bool {
    false,
    true
};

void main_loop(void);
char* read_line(void);
char** parse_args(char* line);
enum bool exec_cmd(char** args);

//int main(int argc, char **argv)
int main(void)
{
    // init shell configs
    // run main shell loop
    main_loop();
    // clenaup before shutdown
    return EXIT_SUCCESS;
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
        should_continue = exec_cmd(args);

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

enum bool exec_cmd(char** args)
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

