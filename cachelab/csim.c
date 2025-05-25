#include "cachelab.h"

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int missCnt  = 0;
int hitCnt   = 0;
int evictCnt = 0;

int           h_flag = 0, v_flag = 0;
int           s = -1, E = -1, b = -1;
char*         tracefile     = NULL;
unsigned long accessCounter = 0;

typedef struct
{
    bool          isValid;
    unsigned long tag;
    unsigned long lastUsed;
} CacheLine;

typedef struct
{
    int        numLines;
    CacheLine* lines;
} CacheSet;

void printUsage(char* progname)
{
    printf("Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n", progname);
}

unsigned long parseSetIndex(unsigned long addr)
{
    unsigned long shifted = addr >> b;
    unsigned long mask    = (1UL << s) - 1;
    return shifted & mask;
}

unsigned long parseTagBits(unsigned long addr)
{
    return addr >> (s + b);
}

bool initCache(CacheSet** p_cache)
{
    CacheSet* cache = (CacheSet*) malloc((1 << s) * sizeof(CacheSet));
    if (cache == NULL)
    {
        printf("Mem Alloc for cache failed\n");
        return false;
    }

    for (unsigned long i = 0; i < (1 << s); ++i)
    {
        cache[i].numLines = 0;
        cache[i].lines    = (CacheLine*) malloc(E * sizeof(CacheLine));
        if (cache[i].lines == NULL)
        {
            return false;
        }
        for (int j = 0; j < E; j++)
        {
            cache[i].lines[j].isValid  = false;
            cache[i].lines[j].tag      = 0;
            cache[i].lines[j].lastUsed = 0;
        }
    }
    *p_cache = cache;
    return true;
}

bool isHit(unsigned long addr, CacheSet* set)
{
    unsigned long tag = parseTagBits(addr);
    for (int i = 0; i < set->numLines; i++)
    {
        if (set->lines[i].isValid && set->lines[i].tag == tag)
        {
            accessCounter++;
            set->lines[i].lastUsed = accessCounter;
            return true;
        }
    }
    return false;
}

void loadIntoCache(unsigned long addr, CacheSet* set)
{
    unsigned long tag = parseTagBits(addr);
    if (set->numLines < E)
    {
        set->lines[set->numLines].isValid  = true;
        set->lines[set->numLines].tag      = tag;
        set->lines[set->numLines].lastUsed = accessCounter;
        set->numLines++;
    }
    else
    {
        // LRU eviction

        int           lruId  = 0;
        unsigned long oldest = set->lines[0].lastUsed;
        for (int i = 1; i < E; i++)
        {
            if (set->lines[i].lastUsed < oldest)
            {
                oldest = set->lines[i].lastUsed;
                lruId  = i;
            }
        }
        evictCnt++;
        set->lines[lruId].tag      = tag;
        set->lines[lruId].lastUsed = accessCounter;
    }
}

int main(int argc, char* argv[])
{
    int opt;

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
    /*printf("s: %d, E: %d, b: %d, h_flag: %d, v_flag: %d\n", s, E, b, h_flag, v_flag);*/

    CacheSet* cache = NULL;
    if (!initCache(&cache))
    {
        printf("Failed to set up cache\n");
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
        /*printf("Op: %c, Addr: 0x%lx, Sz: %d\n", op, addr, size);*/

        unsigned long setIndex = parseSetIndex(addr);
        CacheSet*     set      = &cache[setIndex];
        if (op == 'L')
        {
            if (!isHit(addr, set))
            {
                printf("L %lx,%d miss", addr, size);
                missCnt++;
                if (set->numLines == E)
                {
                    printf(" eviction");
                }
                loadIntoCache(addr, set);
            }
            else
            {
                hitCnt++;

                printf("L %lx,%d hit", addr, size);
            }
        }
        else if (op == 'S')
        {
            if (!isHit(addr, set))
            {
                printf("S %lx,%d miss", addr, size);
                missCnt++;
                if (set->numLines == E)
                {
                    printf(" eviction");
                }
                loadIntoCache(addr, set);
            }
            else
            {
                hitCnt++;
                printf("S %lx,%d hit", addr, size);
            }
        }
        else if (op == 'M')
        {
            if (!isHit(addr, set))
            {
                printf("M %lx,%d miss ", addr, size);
                missCnt++;
                if (set->numLines == E)
                {
                    printf("eviction ");
                }
                loadIntoCache(addr, set);
            }
            else
            {
                hitCnt++;
                printf("M %lx,%d hit ", addr, size);
            }
            hitCnt++;
            printf("hit");
        }
        printf(" \n");
    }

    printSummary(hitCnt, missCnt, evictCnt);

    fclose(fp);

    // free all lines and cache
    for (int i = 0; i < 1 << s; i++)
    {
        free(cache[i].lines);
        cache[i].lines = NULL;
    }

    free(cache);
    cache = NULL;

    return 0;
}
