#pragma once

#ifndef __ANDROID__
  #error "This io.h is only for Android!"
#endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fnmatch.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

// File attribute constants for _finddata_t
#ifndef _A_NORMAL
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20
#endif

// Standard MSVC POSIX aliases
#ifndef _access
#define _access access
#endif
#ifndef _unlink
#define _unlink unlink
#endif
#ifndef _chmod
#define _chmod chmod
#endif
#ifndef _open
#define _open open
#endif
#ifndef _close
#define _close close
#endif
#ifndef _read
#define _read read
#endif
#ifndef _write
#define _write write
#endif
#ifndef _lseek
#define _lseek lseek
#endif

static inline long _filelength(int fd) {
    struct stat st;
    if (fstat(fd, &st) == 0) {
        return (long)st.st_size;
    }
    return -1L;
}

static inline long _tell(int fd) {
    return (long)lseek(fd, 0, SEEK_CUR);
}

#ifndef _FINDDATA_T_DEFINED
#define _FINDDATA_T_DEFINED
typedef unsigned long _fsize_t;

struct _finddata_t {
    unsigned    attrib;
    time_t      time_create;
    time_t      time_access;
    time_t      time_write;
    _fsize_t    size;
    char        name[260];
};
#endif

struct _android_find_state {
    DIR* dir;
    char dirPath[512];
    char pattern[260];
};

static inline int _findnext(intptr_t handle, struct _finddata_t* fileinfo) {
    if (handle == -1 || !handle || !fileinfo) return -1;
    struct _android_find_state* state = (struct _android_find_state*)handle;
    if (!state->dir) return -1;

    struct dirent* entry;
    while ((entry = readdir(state->dir)) != NULL) {
        const char* pat = (strcmp(state->pattern, "*.*") == 0) ? "*" : state->pattern;
        if (fnmatch(pat, entry->d_name, FNM_CASEFOLD) == 0) {
            memset(fileinfo, 0, sizeof(struct _finddata_t));
            strncpy(fileinfo->name, entry->d_name, sizeof(fileinfo->name) - 1);

            char fullPath[1024];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", state->dirPath, entry->d_name);

            struct stat st;
            if (stat(fullPath, &st) == 0) {
                fileinfo->size = (_fsize_t)st.st_size;
                fileinfo->time_create = st.st_ctime;
                fileinfo->time_access = st.st_atime;
                fileinfo->time_write  = st.st_mtime;
                fileinfo->attrib = S_ISDIR(st.st_mode) ? _A_SUBDIR : _A_NORMAL;
            } else if (entry->d_type == DT_DIR) {
                fileinfo->attrib = _A_SUBDIR;
            } else {
                fileinfo->attrib = _A_NORMAL;
            }
            if (entry->d_name[0] == '.' && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                fileinfo->attrib |= _A_HIDDEN;
            }
            return 0;
        }
    }
    return -1;
}

static inline int _findclose(intptr_t handle) {
    if (handle == -1 || !handle) return -1;
    struct _android_find_state* state = (struct _android_find_state*)handle;
    if (state->dir) closedir(state->dir);
    free(state);
    return 0;
}

static inline intptr_t _findfirst(const char* filespec, struct _finddata_t* fileinfo) {
    if (!filespec || !fileinfo) return -1;

    struct _android_find_state* state = (struct _android_find_state*)calloc(1, sizeof(struct _android_find_state));
    if (!state) return -1;

    const char* slash1 = strrchr(filespec, '/');
    const char* slash2 = strrchr(filespec, '\\');
    const char* slash = (slash1 > slash2) ? slash1 : slash2;

    if (slash) {
        size_t dirLen = (size_t)(slash - filespec);
        if (dirLen == 0) {
            strcpy(state->dirPath, "/");
        } else {
            if (dirLen >= sizeof(state->dirPath)) dirLen = sizeof(state->dirPath) - 1;
            memcpy(state->dirPath, filespec, dirLen);
            state->dirPath[dirLen] = '\0';
        }
        strncpy(state->pattern, slash + 1, sizeof(state->pattern) - 1);
    } else {
        strcpy(state->dirPath, ".");
        strncpy(state->pattern, filespec, sizeof(state->pattern) - 1);
    }

    for (char* p = state->dirPath; *p; ++p) {
        if (*p == '\\') *p = '/';
    }

    state->dir = opendir(state->dirPath);
    if (!state->dir) {
        free(state);
        return -1;
    }

    if (_findnext((intptr_t)state, fileinfo) != 0) {
        closedir(state->dir);
        free(state);
        return -1;
    }

    return (intptr_t)state;
}