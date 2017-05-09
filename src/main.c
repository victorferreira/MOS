#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MOS_RL_BUFSIZE 1024
#define MOS_TOK_BUFSIZE 64
#define MOS_TOK_DELIM " \t\r\n\a"

/*
  Function Declarations for builtin shell commands:
 */
int mos_cd(char **args);
int mos_help(char **args);
int mos_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &mos_cd,
  &mos_help,
  &mos_exit
};

int mos_num_builtins()
{
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int mos_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "mos: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int mos_help(char **args)
{
  int i;
  printf("Victor Ferreira's MOS\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < mos_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int mos_exit(char **args)
{
  return 0;
}

int mos_lunch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Chils process
        if (execvp(args[0], args) == -1) {
            perror("mos");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("mos");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int mos_execute(char **args)
{
    int i;

    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < mos_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return mos_lunch(args);
}

char **mos_split_line(char *line)
{
    int bufsize = MOS_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens) {
        fprintf(stderr, "mos: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, MOS_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += MOS_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            
            if (!tokens) {
                fprintf(stderr, "mos: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, MOS_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

// version of mos_read_line using getline() on stdio.h
char *mos_read_line()
{
    char *line = NULL;
    ssize_t bufsize = 0; // have getline allocate a buffer for us.
    getline(&line, &bufsize, stdin);
    return line;
}

void mos_loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        printf("MOS> ");
        line = mos_read_line();
        args = mos_split_line(line);
        status = mos_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv)
{
    // Load config files if any.

    // Run command loop.
    mos_loop();

    // Perform any shutdown/cleanup

    return EXIT_SUCCESS;
}