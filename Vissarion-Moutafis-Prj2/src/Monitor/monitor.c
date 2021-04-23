#include "FM.h"
// test main

int main(int argc, char **argv) {
    char *array[6] = {"./testdir/France",     "./testdir/Germany",
                      "./testdir/New-Guinea", "./testdir/Spain",
                      "./testdir/Sweden",     "./testdir/UK"};
    FM fm = fm_create(array, 6);

    for (int i = 0; i < 6; i++) {
        char **records = NULL;
        int len = 0;
        fm_read_from_directory(fm, array[i], &records, &len);
        printf("%s:\n", array[i]);
        for (int j = 0; j < len; j++) {
            printf("%s\n", records[j]);
            free(records[j]);
        }
        printf("\n");
        free(records);
    }
    getchar();
    char **new_files = NULL;
    int l = 0;
    fm_check_for_updates(fm, &new_files, &l);
    puts("New Files:");
    for (int i = 0; i < l; i++) {
        puts(new_files[i]);
        free(new_files[i]);
    }
    free(new_files);

    for (int i = 0; i < 6; i++) {
        char **records = NULL;
        int len = 0;
        fm_read_from_directory(fm, array[i], &records, &len);
        printf("%s:\n", array[i]);
        for (int j = 0; j < len; j++) {
            printf("%s\n", records[j]);
            free(records[j]);
        }
        printf("\n");
        free(records);
    }

    fm_dectroy(fm);
    exit(0);
}