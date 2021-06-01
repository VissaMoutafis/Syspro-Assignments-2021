/**
*	Syspro Project 3
*	 Written By Vissarion Moutafis sdi1800119
**/
 
# pragma once


#include "Types.h"

// check if path is a dir
int is_dir(char *path);

// get the parent directory path of the current path
char *get_parent_dir(char *path);

// get the part of the <path> that resembles the file/dir name
char *get_elem_name(char *path);

void delete_dir(char *path);

void delete_file(char *path);

void delete_element(char *path);

int count_dir_containings(char *input_dir);