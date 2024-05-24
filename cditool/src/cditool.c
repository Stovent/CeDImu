#include "cditool.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define CMD_IMPORT 1

static void printUsage(const char* argv0)
{
    printf("Usage: %s <command> <command options>\n"
           "Commands:\n"
           "\t--import -o <output> -d <cd drive>\n"
           , argv0);
}

static const struct option options[] = {
    {"import", no_argument, NULL, CMD_IMPORT},
    {0, 0, 0, 0},
};

int main(int argc, char* argv[])
{
    const char *drive = NULL, *output = NULL;
    int command = 0;

    int opt;
    while((opt = getopt_long(argc, argv, "o:d:", options, NULL)) != -1)
    {
        switch(opt)
        {
        case CMD_IMPORT:
            command = CMD_IMPORT;
            break;
        case 'o':
            output = optarg;
            break;
        case 'd':
            drive = optarg;
            break;
        default:
            printUsage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if(command == CMD_IMPORT)
        return !import(output, drive);

    printf("Unknown command\n");
    printUsage(argv[0]);
    exit(EXIT_FAILURE);

    return 0;
}
