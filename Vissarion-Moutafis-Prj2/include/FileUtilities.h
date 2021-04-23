# pragma once


#include "Types.h"

// check if path is a dir
int is_dir(char *path);

// get the parent directory path of the current path
char *get_parent_dir(char *path);