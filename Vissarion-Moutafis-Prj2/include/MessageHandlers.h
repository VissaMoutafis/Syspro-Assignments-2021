#pragma once


// MessageHandler type
// pass monitor (either parent or child)
// pass msg to handle (not \0 terminated)
// msg_len
// pass return args as array of void * so the user can set them properly
typedef void (*MessageHandler)(void *monitor, char *msg, int msg_len, void *return_args[]);


// Travel Monitor's message handlers
void get_bf_from_child(void *monitor, char *msg, int msg_len, void *return_args[]);
void travel_request_handler(void *monitor, char *msg, int msg_len, void *return_args[]);
void get_vaccination_status(void *monitor, char *msg, int msg_len, void *return_args[]);

///////////////////////////////////////////////////////////////////////////////////

// Monitors' message handlers

void get_dirs(void *monitor, char *msg, int msg_len, void *return_args[]);
void get_init_stats(void *monitor, char *msg, int msg_len, void *return_args[]);
void get_query(void *monitor, char *msg, int msg_len, void *return_args[]);