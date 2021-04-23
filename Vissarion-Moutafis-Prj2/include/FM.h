#pragma once

#include "Types.h"
#include "HT.h"
#include "List.h"
#include "Utilities.h"
#include "FileUtilities.h"

// basic typedef type 
typedef struct file_manager *FM;

// This is a file manager for every monitor in order to keep track of all the directories
// and files in its juristiction and ease the reading and updating process.

// create the file manager.
// return the FM struct.
FM fm_create(char *init_directories[], int array_len);

// Add a directory and all files in it in the file manager struct.
// Return true if successful, else return false.
bool fm_add_directory(FM fm, char *dir_path);

// Add a new file in the file manager struct.
// Return true if successful, else return false.
bool fm_add_file(FM fm, char *file_path);

// Read all the lines from all the files, recorded by the file manager, 
// in a specific directory.
void fm_read_from_directory(FM fm, char *dir_path, char ***records, int *length);

// Read all the lines from a specific file.
// Return the records in a string array.
void fm_read_from_file(FM fm, char *file_path, char ***records, int *length);

// Check for any added files in the recorded directories.
// Return the paths of the newly added files 
void fm_check_for_updates(FM fm, char ***new_files, int *length);

// Free all the memory hold by the file manager
void fm_dectroy(FM fm);

// get the directory list of fm
List fm_get_directory_list(FM fm);