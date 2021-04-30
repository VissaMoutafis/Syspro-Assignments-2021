#pragma once
#include "Types.h"
#include "FileUtilities.h"

#define FIFO_DIR "fifos"
#define DEF_BUFFER_SIZE 100

// Basic Fifo Setup

// create a unique fifo. if init is true create only the fifo directory
void create_unique_fifo_pair(bool init, int unique_id, char *from, char *to);

// routine to clean up the fifos
void clean_fifos(void);