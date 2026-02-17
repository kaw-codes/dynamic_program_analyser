#include "cli.h"
#include "libdpa.h"
#include <argp.h> // opt parsing
#include <stdio.h> // printf, fgets
#include <stdlib.h> // atoi
#include <string.h> // strtok

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
    printf("available commands:\n");
    printf("  help,     h\n");
    printf("  attach,   a,  pid\n");
    printf("  continue, c\n");
    printf("  detach,   d\n");
    printf("  exit,     e\n");
}

int convert_str_into_id(char *cmd)
{
    if (strcmp(cmd, "help") == 0 || 
        strcmp(cmd, "h") == 0)
        return HELP;
    if (strcmp(cmd, "attach") == 0 || 
        strcmp(cmd, "a") == 0)
        return ATTACH;
    if (strcmp(cmd, "continue") == 0 || 
        strcmp(cmd, "c") == 0)
        return CONTINUE;
    if (strcmp(cmd, "detach") == 0 || 
        strcmp(cmd, "d") == 0)
        return DETACH;
    if (strcmp(cmd, "exit") == 0 || 
        strcmp(cmd, "e") == 0)
        return EXIT;
    return ERROR;
}

int main(int argc, char **argv)
{
    /* arg parsing */
    arguments_t args;
    args.path = NULL;
    args.pid = -1;
    if (argc > 3)
    {
        exit(EXIT_FAILURE);
    }
    argp_parse(&argp, argc, argv, 0, 0, &args);

    /* check args */
    printf("pid: %d\n", args.pid);
    printf("path: %s\n", args.path);

    /* if args */
    process_t *proc = NULL;
    if (args.pid != -1) // pid mode
    {
        if (attach(args.pid, true, &proc) != 0)
        {
            printf("error: issue with attach");
            return EXIT_FAILURE;
        }
        if (!proc)
        {
            printf("error: proc = NULL");
            return EXIT_FAILURE;
        }
    }
    else if (args.path) // software path mode
    {
        if (launch(args.path, true, &proc) != 0)
        {
            printf("error: issue with launch");
            return EXIT_FAILURE;
        }
        if (!proc)
        {
            printf("error: proc = NULL");
            return EXIT_FAILURE;
        }
    }

    /* routine */
    char input[128];
    while(1)
    {
        /* get cmd + args */
        printf("dpa> ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0; // to remove '\n'
        char *cmd = strtok(input, " ");
        char *arg1 = strtok(NULL, " ");
        int cmd_id = -1;
        if (cmd) cmd_id = convert_str_into_id(cmd);

        /* exec command */
        switch(cmd_id)
        {
        /* help */
        case HELP:
            print_help_commands();
            break;

        /* attach */
        case ATTACH:
            if (!arg1)
            {
                printf("error: 'attach' cmd requires 1 arg\n");
                print_help_commands();
                break;
            }
            else
            {
                if (proc)
                {
                    printf("detaching...\n");
                    if (detach(proc) != 0)
                    {
                        printf("error: issue with detach\n");
                        return EXIT_FAILURE;
                    }
                    free(proc);
                    proc = NULL;
                }
                printf("attaching...\n");
                pid_t pid = atoi(arg1);
                printf("%d\n", pid);
                if (pid <= 0)
                {
                    printf("error: wrong pid\n");
                    break;
                }
                if (attach(pid, true, &proc) != 0)
                {
                    printf("error: issue with attach\n");
                    break;
                }
                break;
            }

        /* continue */
        case CONTINUE:
            if (!proc)
            {
                printf("error: no proc attached\n");
                break;
            }
            printf("continuing...\n");
            if (resume(proc) != 0)
            {
                printf("error: issue with resume\n");
                return EXIT_FAILURE;
            }
            break;

        /* detach */
        case DETACH:
            if (!proc)
            {
                printf("error: no proc attached\n");
                break;
            }
            printf("detaching...\n");
            if (detach(proc) != 0)
            {
                printf("error: issue with detach\n");
                return EXIT_FAILURE;
            }
            break;

        /* exit */
        case EXIT:
            if (proc)
            {
                printf("detaching...\n");
                if (detach(proc) != 0)
                {
                    printf("error: issue with detach\n");
                    return EXIT_FAILURE;
                }
                free(proc);
                proc = NULL;
            }
            printf("exiting...\n");
            goto _end;
            break;
        
        /* error well */
        case ERROR:
            printf("error: wrong command\n");
            break;
        }
    }

_end:
    return 0;
}
