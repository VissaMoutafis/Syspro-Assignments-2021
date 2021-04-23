#include "FileUtilities.h"

// return 1 if the path is a directory, else 0
int is_dir(char *_path) {
    char *path = realpath(_path, NULL);
    struct stat buf;
    memset(&buf, 0, sizeof(buf));
    if (lstat(path, &buf) < 0) {
        perror("lstat");
        exit(1);
    }
    free(path);
    return (buf.st_mode & S_IFMT) == S_IFDIR;
}

char * get_parent_dir(char *path) {
    // start from the end of the path and eliminate the part after the last '/'
    char *parent_dir = NULL;
    int path_len = strlen(path);
    int i;
    for (i = path_len-1; i > 0; i--) {
        if (path[i] == '/')
            break;
    }
    char *str = NULL;
    if (i > 0){
        str = calloc(i+1, sizeof(char));
        strncpy(str, parent_dir, i); 
    } else {
        str = calloc(2, sizeof(char));
        strcpy(str, ".");
    }
    return str;
}