#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <fts.h>
#include <sys/stat.h>


int descending(const FTSENT * const *a, const FTSENT * const *b)
{
    if (a->fts_statp->st_ctime > b->fts_statp->st_ctime) return 1;
    else return -1;
}

FTS*
getdir(const char* path)
{
    char* pathlist[2] = { dirlist, NULL };

    return fts_open(pathlist, FTS_NOCHDIR, descending);
}

int
next_file(const int current, char* path)
{
    FTS* dir = getdir(path);
    if (current >= 0) {
        current += 1;
        if (current > ?) {
            current = 0;
        }
    }
}

int
prev_file(const int current, char* path)
{
    FTS* dir = getdir(path);
}

