#include <argp.h> // opt parsing
#include <stdio.h> // printf
#include <stdlib.h> // atoi

/* options parsing */

static char doc[] = "dynamic program analyser";

static char args_doc[] = "[PATH | -p PID | -]";

static struct argp_option options[] = 
{
    {"help", 'h', 0, 0, "Prints help message", 0},
    {"pid", 'p', "PID", 0, "Attach to a process pid", 0},
    {0}
};

typedef struct
{
    int pid;
    char *path;
} arguments_t;

void print_arg_help()
{
    printf("usage: dpa [-p PID | -h, --help | PATH | -]\n");
}

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    arguments_t *arguments = state->input;
    switch (key)
    {
    case 'h':
        print_arg_help();
        exit(EXIT_SUCCESS);
    case 'p':
        arguments->pid = atoi(arg);
        break;
    case ARGP_KEY_ARG:
        arguments->path = arg;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

/* main code */

void print_help_commands()
{
    printf("available commands:");
    printf("  help, h\n");
}

int main(int argc, char **argv)
{
    /* arg parsing */
    arguments_t args;
    args.path = NULL;
    args.pid = -1;
    if (argc > 3)
        exit(EXIT_FAILURE);
    argp_parse(&argp, argc, argv, 0, 0, &args);

    /* check args */
    printf("pid:%d\n", args.pid);
    printf("path: %s\n", args.path);

    return 0;
}
