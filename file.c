#include "file.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fts.h>
#include <sys/stat.h>

#define LOG_MODULE "file"
#define LOG_ENABLE_DBG 1
#include "log.h"


struct FileName {
    char* name;
    struct FileName* next;
};

bool include(char* const name, char** suffix)
{
    bool match = false;
    int i = 0;
    int n = 0;
    int m = strlen(name);
    char* start = NULL;
    while (suffix[i] != NULL ) {
        n = strlen(suffix[i]);
        /* empty suffix or longer than name, skip */
        if (n == 0 || (m - n) < 1) {
            continue;
        }
        start = name + m - n;
        if (strncasecmp(start, suffix[i], n) == 0) {
            LOG_DBG("%s matched %s", name, suffix[i]);
            match = true;
            break;
        }
        i++;
    }

    return match;
}

int descending(const FTSENT **a, const FTSENT **b)
{
    if ((*a)->fts_statp->st_ctime < (*b)->fts_statp->st_ctime) return 1;
    else return -1;
}

int
getdir(char* path, struct FileName* const list, char* filters[])
{
    struct FileName* curr = list;
    struct FileName* next = NULL;
    FTSENT* file = NULL;

    char* pathlist[2] = { path, NULL };

    int pathlen = strlen(path);

    if (path[pathlen - 1] == '/') {
        path[pathlen - 1] = '\0';
    }
    LOG_DBG("fts_open(%s)", pathlist[0]);
    FTS* files = fts_open(pathlist, FTS_NOCHDIR, descending);
    if (files == NULL) {
        LOG_ERRNO("fts_open(%s) failed", path);
        return 0;
    }
    file = fts_read(files);
    file = fts_children(files, 0);
    
    int count = 0;
    while (file != NULL) {
        if (include(file->fts_name, filters)) {
            curr->name = malloc(file->fts_namelen + 1);
            if (curr->name == NULL) {
                LOG_ERR("Failed to allocate memory for file name");
                break;
            }
            strncpy(curr->name, file->fts_name, file->fts_namelen + 1);
            LOG_DBG("File %d: %s", count, curr->name);
            next = malloc(sizeof(struct FileName));
            if (next == NULL) {
                LOG_ERR("Failed to allocate memory for file list entry");
                break;
            }
            next->name = NULL;
            next->next = NULL;
            curr->next = next;
            curr = next;
        }
        file = file->fts_link;
        count++;
    }

    strcat(path, "/");

    LOG_DBG("Closing FTS");
    fts_close(files);
    LOG_DBG("Found %d files", count);
    return count;
}

void
free_names_recurse(struct FileName* file)
{
    if (file) {
        free_names_recurse(file->next);
        if (file->name) {
            LOG_DBG("Freeing name %s", file->name);
            free(file->name);
        }
        free(file);
    }
}

char*
next_file(int* const current, char* path, char* filter[])
{
    struct FileName* files = malloc(sizeof(struct FileName));
    if (files == NULL) {
        LOG_ERR("Failed allocating memory for file name entry");
        return NULL;
    }

    files->name = NULL;
    files->next = NULL;

    int numfiles = getdir(path, files, filter);
    if (numfiles <= 0) {
        /* no files or other error */
        *current = 0;
        return NULL;
    }

    if (*current >= numfiles || *current < 0) {
        *current = 0;
    }
    else {
        *current = *current + 1;
    }

    struct FileName* curfile = files;
    for (int j = 0; j < *current; j++) {
        curfile = curfile->next;
    }

    LOG_DBG("Current file: %d", *current);
    LOG_DBG("Current file name: %s", curfile->name);
    char* name = malloc(strlen(curfile->name) + 1);
    if (name == NULL) {
        LOG_ERR("Failed allocating memory for file name");
    }
    else {
        strcpy(name, curfile->name);
    }

    LOG_DBG("Freeing filename list");
    free_names_recurse(files);

    return name;
}

char*
prev_file(int* const current, char* path, char* filter[])
{
    struct FileName* files = malloc(sizeof(struct FileName));
    if (files == NULL) {
        LOG_ERR("Failed allocating memory for file name entry");
        return NULL;
    }

    files->name = NULL;
    files->next = NULL;

    int numfiles = getdir(path, files, filter);
    if (numfiles <= 0) {
        /* no files or other error */
        *current = 0;
        return NULL;
    }

    if (*current == 0) {
        *current = numfiles - 1;
    }
    else {
        *current = *current - 1;
    }

    LOG_DBG("Current file: %d", *current);
    LOG_DBG("Current file name: %s", files[*current].name);
    char* name = malloc(strlen(files[*current].name));
    if (name == NULL) {
        LOG_ERR("Failed allocating memory for file name");
    }
    else {
        strcpy(name, files[*current].name);
    }

    LOG_DBG("Freeing filename list");
    free_names_recurse(files);

    return name;
}

