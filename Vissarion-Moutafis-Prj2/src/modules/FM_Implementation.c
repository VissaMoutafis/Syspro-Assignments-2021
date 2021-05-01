#include "FM.h"

struct file_manager {
    HT file_path_table;
    List directory_list;
} ;

struct dentry {
    char *dir_path;     // directory path 
    List files;         // list of all the directory files
} ;

struct fentry {
    char *file_path;    // file path
    DirectoryEntry dir; // pointer to the directory entry in the dir hash table
};


// Utilities

FileEntry create_file_entry(char *file_path, DirectoryEntry dir) {
    FileEntry f = calloc(1, sizeof(*f));
    f->file_path = calloc(strlen(file_path)+1, sizeof(char));
    strcpy(f->file_path, file_path);
    f->dir = dir;
    return f;
}

int compare_file_entries(Pointer f1, Pointer f2) {
    char *file1 = ((FileEntry)f1)->file_path;
    char *file2 = ((FileEntry)f2)->file_path;
    char *buf1, *buf2;
    buf1 = realpath(file1, NULL);
    buf2 = realpath(file2, NULL);
    int c = strcmp(buf1, buf2);
    if (buf1) free(buf1);
    if (buf2) free(buf2);
    return c;
}
u_int32_t hash_file_entry(Pointer f) {
    FileEntry fd = (FileEntry)f;
    return hash_i((unsigned char *)fd->file_path, (unsigned int)strlen(fd->file_path));
}

void destroy_file_entry(Pointer f) {
    FileEntry fd = (FileEntry)f;
    free(fd->file_path);
    free(f);
}


DirectoryEntry create_directory_entry(char *dir_path) {
    DirectoryEntry d = calloc(1, sizeof(*d));
    d->dir_path = calloc(strlen(dir_path)+1, sizeof(char));
    strcpy(d->dir_path, dir_path);
    d->files = list_create(compare_file_entries, NULL);
    return d;
}

int compare_dir_entry(Pointer d1, Pointer d2) {
    char *dir1 = ((DirectoryEntry)d1)->dir_path;
    char *dir2 = ((DirectoryEntry)d2)->dir_path;
    char *buf1, *buf2;
    buf1 = realpath(dir1, NULL);
    buf2 = realpath(dir2, NULL);
    int c = strcmp(buf1, buf2);
    if (buf1) free(buf1);
    if (buf2) free(buf2);
    return c;
}

void destroy_dir_entry(Pointer d) {
    DirectoryEntry dir = (DirectoryEntry)d;
    free(dir->dir_path);
    list_destroy(&(dir->files));
    free(d);
}

static void add_file_entries(FM fm, DirectoryEntry dir_entry) {
    // first open the dir 
    DIR *dirp = opendir(dir_entry->dir_path);
    
    // check if everything is ok
    if (dirp == NULL) {
        char error_buf[BUFSIZ];
        sprintf(error_buf, "Problem opening directory %s", dir_entry->dir_path);
        perror(error_buf);
    } else {
        struct dirent *direntp = NULL;
        // traverse the directory entries
        while ((direntp = readdir(dirp)) != NULL) {
            if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0)
                continue;

            // create a new path and add the files in the fm hashtable and the directory entry list
            char file_path[BUFSIZ];
            memset(file_path, 0, BUFSIZ);
            sprintf(file_path, "%s/%s", dir_entry->dir_path, direntp->d_name);
            FileEntry file_entry = create_file_entry(file_path, dir_entry);
            Pointer old = NULL;

            ht_insert(fm->file_path_table, file_entry, false, &old);
            list_insert(dir_entry->files, file_entry, true);

            #ifdef DEBUG
            assert(ht_contains(fm->file_path_table, file_entry, &old));
            assert(list_find(dir_entry->files, file_entry));
            #endif
        }   
        closedir(dirp);
    }
}

static void get_dir_new_files(FM fm, DirectoryEntry dir_entry, char ***dir_new_files, int *dir_len) {
    // first open the dir 
    DIR *dirp = opendir(dir_entry->dir_path);
    Pointer old = NULL;
    *dir_new_files = NULL;
    *dir_len = 0;
    // check if everything is ok
    if (dirp == NULL) {
        char error_buf[BUFSIZ];
        sprintf(error_buf, "Problem opening directory %s", dir_entry->dir_path);
        perror(error_buf);
    } else {
        struct dirent *direntp = NULL;
        // traverse the directory entries
        while ((direntp = readdir(dirp)) != NULL) {
            if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0)
                continue;
            
            // create a new path and add the files in the fm hashtable and the directory entry list
            char file_path[BUFSIZ];
            memset(file_path, 0, BUFSIZ);
            sprintf(file_path, "%s/%s", dir_entry->dir_path, direntp->d_name);
            struct fentry dummy_f = {.file_path=file_path};
            
            // if the entry exists then go to the next one
            if (ht_contains(fm->file_path_table, &dummy_f, &old))
                continue;

            // if the file entry is new then add it the the FM struct and in the new files array
            FileEntry file_entry = create_file_entry(file_path, dir_entry);
            Pointer old = NULL;

            ht_insert(fm->file_path_table, file_entry, false, &old);
            list_insert(dir_entry->files, file_entry, true);

            // debug
            #ifdef DEBUG
            assert(ht_contains(fm->file_path_table, file_entry, &old));
            assert(list_find(dir_entry->files, file_entry));
            #endif

            //create a new array
            char **new_array = calloc((*dir_len) + 1, sizeof(char *));
            if (*dir_len > 0)
                memcpy(new_array, *dir_new_files, ((*dir_len) * sizeof(char *)));
            
            // create the new string path
            char *new_entry = calloc(strlen(file_path) + 1, sizeof(char));
            strcpy(new_entry, file_path);
            new_array[*dir_len] = new_entry;

            // free the previous array
            if (*dir_new_files) free(*dir_new_files);

            //set the new array as the current one
            *dir_new_files = new_array;
            (*dir_len)++;
        }   
        closedir(dirp);
    }
}

// // // File manager methods // // //

FM fm_create(char **init_directories, int array_len) {
    FM fm = calloc(1, sizeof(*fm));
    fm->directory_list = list_create(compare_dir_entry, destroy_dir_entry);
    fm->file_path_table = ht_create(compare_file_entries, hash_file_entry, destroy_file_entry);
    
    if (init_directories) {
        // there exist initialization directories
        // so we loop through the string vector
        // and add all of them and their containings with the respective function
        for (int i = 0; i < array_len; i++) {
            fm_add_directory(fm, init_directories[i]);
        }
    }

    return fm;
}

bool fm_add_directory(FM fm, char *dir_path) {
    if (is_dir(dir_path)) {
        // check if it already exists
        struct dentry dummy_d= {.dir_path=dir_path};
        if (!list_find(fm->directory_list, &dummy_d)) {
            // first add the dir entry
            DirectoryEntry dir_entry = create_directory_entry(dir_path);
            list_insert(fm->directory_list, dir_entry, true);
            // then add all the entries (except . and ..) in the fm hashtable and the directory entry file list
            add_file_entries(fm, dir_entry);
            
            #ifdef DEBUG
            assert(list_find(fm->directory_list, dir_entry));
            #endif

            return true;
        }
    }
    return false;
}

bool fm_add_file(FM fm, char *file_path) {
    #ifdef DEBUG
    assert(fm);
    assert(file_path);
    #endif

    // check if its actually a readable object
    if (!is_dir(file_path)) {
        // check if it is already in the hash table
        // if it's not create a new File entry and add it
        struct fentry dummy_f = {.file_path=file_path};
        Pointer ptr; 
        if (!ht_contains(fm->file_path_table, &dummy_f, &ptr)) {
           // check also for parent directory 
           char *parent_dir_path = get_parent_dir(file_path);
           
           struct dentry dummy_d = {.dir_path=parent_dir_path};
           DirectoryEntry parent_dir_entry = list_node_get_entry(fm->directory_list, list_find(fm->directory_list, &dummy_d));
           if (!parent_dir_entry) {
                // if it does not exists we must add it in the fm list
                parent_dir_entry = create_directory_entry(parent_dir_path);
                list_insert(fm->directory_list, parent_dir_entry, true);
           }
            // create the file entry and insert it in the hash table and in the directory list
            FileEntry file_entry = create_file_entry(file_path, parent_dir_entry);
            ht_insert(fm->file_path_table, file_entry, false, &ptr);
            list_insert(parent_dir_entry->files, file_entry, true);

            #ifdef DEBUG
            assert(ht_contains(fm->file_path_table, &file_entry, &ptr));
            assert(list_find(fm->directory_list, &parent_dir_entry));
            assert(list_find(parent_dir_entry->files, &file_entry));
            #endif

            return true;
        }
    }
    return false;
}

void fm_read_from_directory(FM fm, char *dir_path, char ***records, int *length) {
    // We have to traverse through all the files in the directory and: 
    // 1. create the respective record array
    // 2. concatenate it with the overall record array

    // initialize the array
    *records = NULL;
    *length = 0;

    //open dir
    DIR *dirp = opendir(dir_path);

    if (dirp == NULL) {
        char error_buf[BUFSIZ];
        sprintf(error_buf, "Problem opening directory %s", dir_path);
        perror(error_buf);
    } else {
        struct dirent *direntp = NULL;
        // traverse the directory entries
        while ((direntp = readdir(dirp)) != NULL) {
            if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0)
                continue;

            // create a new file path to get the complete file path
            char file_path[BUFSIZ];
            memset(file_path, 0, BUFSIZ);
            sprintf(file_path, "%s/%s", dir_path, direntp->d_name);
            
            // get the array record
            char ** file_records = NULL;
            int file_records_len = 0;
            fm_read_from_file(fm, file_path, &file_records, &file_records_len);
            
            // extent the current record array
            char **new_rec_array = calloc((*length)+file_records_len, sizeof(char *));
            if (*length > 0) {
                // append the old array only if there are records in it
                memcpy(new_rec_array, *records, (*length) * sizeof(char *));
            }
            memcpy(new_rec_array+(*length), file_records, file_records_len * sizeof(char *));


            // free the memory of the 2 arrays (old records and file records)
            if (*records) free(*records);
            if (file_records) free(file_records);

            // set the new_rec_array as the current record array
            *records = new_rec_array;
            *length = (*length) + file_records_len;
        } 
        closedir(dirp);  
    }    
}

void fm_read_from_file(FM fm, char *file_path, char ***records, int *length) {
    // first we must open the file 
    FILE *fp = fopen(file_path, "r");
    *records = NULL; 
    *length = 0;

    if (!fp) {
        perror("Could not open file in fm_read_from_file");
    } else {
        // get the number of the lines in the file in order to create a record table
        // also set the length variable 
        int lines = fget_lines(file_path);
        *records = NULL;
        *length = 0;
        for (unsigned int i = 0; i < lines; i++) {
            // create the record string
            char *record_line = make_str(&fp);
            // insert it in the record table
            if (strlen(record_line) == 0) {
                // the string contains only the null char so free it and read the next string
                free(record_line);
                continue;
            }
            // reallocate the memory of the array
            char **new_array = calloc((*length+1), sizeof(char *));
            
            // set the new array
            if (*length > 0)
                memcpy(new_array, *records, (*length)*sizeof(char *));
            new_array[(*length)] = record_line;

            // apply the extensions of the records array
            free(*records); 
            *records = new_array;
            (*length) ++; 
        }
        // close the file now that we are done
        fclose(fp);
    }
}

void fm_check_for_updates(FM fm, char ***new_files, int *length) {
    // we have to iterate through every directory and:
    // 1. traverse through its files
    // 2. check if they are in the file_path_table
    // 3. if not add them in the table and the according directory list
    // 4. add them in the new_files array (and increase the *length by one each time)

    ListNode dir_node = list_get_head(fm->directory_list);
    *new_files = NULL;
    *length = 0;
    while (dir_node) {
        // get the directory
        DirectoryEntry dir_entry = list_node_get_entry(fm->directory_list, dir_node);
        // get the dir's new_files array
        char **dir_new_files = NULL;
        int dir_len = 0;
        get_dir_new_files(fm, dir_entry, &dir_new_files, &dir_len);

        // create a new larger files array
        if (dir_len > 0){
            char ** new_file_array = calloc((*length) + dir_len, sizeof(char *));
            if (*length > 0) {
                //concatenate the old array
                memcpy(new_file_array, *new_files, (*length) * sizeof(char *));
            }
            // concatenate the new array from current file
            memcpy(new_file_array+(*length), dir_new_files, dir_len*sizeof(char*));

            if (*new_files) free(*new_files);
            if (dir_new_files) free(dir_new_files);

            *new_files = new_file_array;
            *length = (*length) + dir_len; 
        }
        // go to the next directory
        dir_node = list_get_next(fm->directory_list, dir_node);
    }
}

void fm_destroy(FM fm) {
    list_destroy(&(fm->directory_list));
    ht_destroy(fm->file_path_table);
    free(fm);
}

List fm_get_directory_list(FM fm) {
    assert(fm);
    return fm->directory_list;
}

void fm_read_from_dir_entry(FM fm, DirectoryEntry dir_entry, char ***records, int *length) {
    assert(fm);
    assert(dir_entry);
    fm_read_from_directory(fm, dir_entry->dir_path, records, length);
}

void fm_read_from_file_entry(FM fm, FileEntry file_entry, char ***records, int *length) {
    assert(fm);
    assert(file_entry);
    fm_read_from_file(fm, file_entry->file_path, records, length);
}

char * fm_get_dir_name(FM fm, DirectoryEntry entry) {
    return entry ? entry->dir_path : NULL;
}