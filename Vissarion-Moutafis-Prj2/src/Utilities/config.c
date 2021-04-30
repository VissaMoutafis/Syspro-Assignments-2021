#include "Config.h"

//routine to create fifos
void create_unique_fifo_pair(bool init, int unique_id, char *from, char *to) {
    if (init) {
        // check if the fifo dir exists. create it. return;
        if (access(FIFO_DIR, F_OK) == 0) delete_dir(FIFO_DIR);

        if (mkdir(FIFO_DIR, 0777) == -1) {
            perror("mkdir at fifo dir creation");
            exit(1);
        }
        return;
    }

    sprintf(to, "%s/to-monitor-%d.fifo", FIFO_DIR, unique_id);
    if (mkfifo(to, 0777) == -1) {perror("mkfifo"); exit(1);}
    sprintf(from, "%s/from-monitor-%d.fifo", FIFO_DIR, unique_id);
    if (mkfifo(from, 0777) == -1) {perror("mkfifo"); exit(1);}
}

void clean_fifos(void) {
    // check if the fifo dir exists. create it. return;
    if (access(FIFO_DIR, F_OK) == 0) delete_dir(FIFO_DIR);
    else {perror("clean fifos"); exit(1);}
}

