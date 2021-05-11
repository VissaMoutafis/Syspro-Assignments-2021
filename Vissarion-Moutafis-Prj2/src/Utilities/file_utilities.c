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

char *get_elem_name(char *path){
    int i = strlen(path)-1;
    while (i > 0 && path[i-1] != '/') i--;

    return path + i;
}


void delete_dir(char *path) {
    DIR *dp;
    struct dirent *dir;

    // first open the src directory
    if ((dp = opendir(path)) == NULL) {
        fprintf(stderr, "Trouble opening directory '%s'.\n", path);
        exit(1);
    }

    // Now we are ready to check the containings of the  target directory
    while ((dir = readdir(dp)) != NULL) {
        if (strcmp(".", dir->d_name) == 0 ||
            strcmp("..", dir->d_name) == 0)
            continue;

        // reconstruct the suppossed path as it would be in the original
        // directory
        char *element_path = calloc(strlen(path) + 1 + strlen(dir->d_name) + 1, sizeof(char));
        strcpy(element_path, path);
        strcat(element_path, "/");
        strcat(element_path, dir->d_name);

        delete_element(element_path);

        // free the allocated memory
        free(element_path);
        element_path = NULL;
    }

    closedir(dp);
    rmdir(path);
}

void delete_file(char *path) {
    if (unlink(path) != 0) {
        char b[BUFSIZ];
        sprintf(b, "Unlinking '%s'", path);
        perror(b);
        exit(1);
    }
}

void delete_element(char *path) {
    if (is_dir(path))
        delete_dir(path);
    else
        delete_file(path);
}