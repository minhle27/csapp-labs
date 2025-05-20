#include "cachelab.h"

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void printUsage(char* progname)
{
    printf("Usaage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n", progname);
}

typedef struct
{
    bool  isValid;
    char* tag;
} CacheLine;

typedef struct
{
    int        numLines;
    CacheLine* lines;
} CacheSet;

int main(int argc, char* argv[])
{
    int opt;

    int   h_flag = 0, v_flag = 0;
    int   s = -1, E = -1, b = -1;
    char* tracefile = NULL;

    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1)
    {
        switch (opt)
        {
            case 'h':
                h_flag = 1;
                break;
            case 'v':
                v_flag = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                tracefile = optarg;
                break;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }
    printf("s: %d, E: %d, b: %d, h_flag: %d, v_flag: %d\n", s, E, b, h_flag, v_flag);

    CacheSet* cache = (CacheSet*) malloc((1 << s) * sizeof(CacheSet));
    if (cache == NULL)
    {
        printf("Mem Alloc for cache failed\n");
        return 1;
    }

    FILE* fp = fopen(tracefile, "r");
    if (fp == NULL)
    {
        printf("Could not open trace file\n");
        return 1;
    }

    char          readline[100];
    char          op;
    unsigned long addr;
    int           size;

    while (fgets(readline, sizeof(readline), fp))
    {
        if (readline[0] == 'I')
            continue;

        sscanf(readline + 1, " %c %lx,%d", &op, &addr, &size);
        printf("Op: %c, Addr: 0x%lx, Sz: %d\n", op, addr, size);
    }

    printSummary(0, 0, 0);

    free(cache);
    return 0;
}
