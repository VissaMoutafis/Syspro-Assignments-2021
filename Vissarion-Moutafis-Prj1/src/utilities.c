/*
** Usefull utilities like string parsers and file lines to string converters
**  Written by Vissarion Moutafis sdi1800119
*/
#include "Utilities.h"

char **parse_line(char *data_str, int *columns, char* sep) {
    // Initialize the columns counter
    *columns = 0;
    char *data_string = calloc(1 +strlen(data_str), sizeof(char));
    strcpy(data_string, data_str);
    char **data = NULL;
    
    // split the str to all space-separated substring
    char *tok = strtok(data_string, sep);

    while (tok != NULL) {
        // for all the tokens we just keep on going to parse the data correclty

        // increase the counter
        (*columns)++;
        // //resize the array
        char **new_data = calloc(*columns, sizeof(char *));

        if (data != NULL) {
            memcpy(new_data, data, ((*columns) - 1) * sizeof(char *));
            free(data);
        }

        // re-assign the old data table to the new one
        data = new_data;

        // create a copy of the token
        data[(*columns) - 1] = malloc((strlen(tok) + 1) * sizeof(char));
        // .. and add i to the data table
        strcpy(data[(*columns) - 1], tok);

        tok = strtok(NULL, sep);
    }
    free(data_string);
    return data;
}

char *make_str(FILE **_stream_) {
    /*
    ** A utility to transform a stream input to a string.
    ** there is no need to enter a string length
    ** WARNING: It returns the whole line and only the line. Parse it carefully.
    */

    // First we will get the line
    unsigned int len = 1;
    char *str = calloc(1, sizeof(char));
    unsigned int c;

    while ((c = fgetc(*_stream_)) != EOF && c != '\n') {
        len++;
        char *new_str = calloc(len, sizeof(char));
        strcpy(new_str, str);
        new_str[len - 2] = c;
        free(str);
        str = new_str;
    }

    if (strlen(str) == 0 && c == EOF) { 
        // if we are at the end of the file and 
        // the str is empty just free the 1 byte and return EOF
        free(str);
        return NULL;
    }

    return str; // return the str
}

// classic trick to count the number of lines in a file
size_t fget_lines(char *filename) {
    FILE *fin = fopen(filename, "r"); // open the file for reading
    if (!fin) {
        printf("The file you entered does not exist!\n");
        exit(EXIT_FAILURE);
    }
    size_t count = 0;
    unsigned char c;
    while ((c = fgetc(fin)) != EOF && !feof(fin))
        count = (c == '\n') ? count + 1 : count;

    // close the file properly
    fclose(fin);

    return count;
}

bool is_numeric(char* str) {
    for (int i = 0; str[i]; i++)
        if (!(str[i] >= '0' && str[i] <= '9'))
            return false;
    
    return true;
}


char* num_to_str(int num) {
    if (num == 0) return "0";

    char* str = calloc(1, sizeof(char));
    int len = 1;
    while (num) {
        char dig = num%10 + '0';
        num /= 10;

        char* new_str = calloc(++len, sizeof(char));
        strcpy(new_str+1, str);
        new_str[0] = dig;
        free(str);
        str = new_str;
    }

    return str;
}

int get_len_of_int(int num) {
    // this function will find the length of integers given
    int cnt = 0;
    while (num) {
        cnt++;
        num /= 10;
    }

    return cnt;
}


bool check_date(char *date) {
    int d,m,y;
    bool proper_format = (sscanf(date, "%d-%d-%d", &d,&m,&y) == 3);

    return proper_format
        && d > 0 && d <= 30
        && m > 0 && m <= 12
        && y >= 0;
}

bool check_date_in_range(char *_date, char *_date1, char *_date2) {
    int d,d1,d2,m,m1,m2,y,y1,y2;
    sscanf(_date, "%d-%d-%d", &d, &m, &y);
    sscanf(_date1, "%d-%d-%d", &d1, &m1, &y1);
    sscanf(_date2, "%d-%d-%d", &d2, &m2, &y2);    

    int date, start, end;
    date = 10000*y + 100*m + d;
    start = 10000*y1 +100*m1 + d1;
    end = 10000*y2 + 100*m2 + d2;

    return date >= start && date <= end;
}

// test main
// int main(int argc, char **argv) {
//     printf("%s\n", check_date_in_range(argv[1], argv[2], argv[3]) ? "YES" :"NO");
//     exit(0);
// }