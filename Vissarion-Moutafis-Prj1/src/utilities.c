/*
** Usefull utilities like string parsers and file lines to string converters
**  Written by Vissarion Moutafis sdi1800119
*/
#include "Utilities.h"

char **parse_line(char *data_str, int *columns, char* sep) {
    // Initialize the columns counter
    *columns = 0;
    
    char **data = NULL;

    // split the str to all space-separated substring
    char *tok = strtok(data_str, sep);

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

void *create_person(char * citizenID, char *firstName, char *lastName, char *country, int age, char *virusName, char *vaccinated, char *date, bool deep_copy) {
    assert(citizenID);
    assert(firstName);
    assert(lastName);
    assert(country);
    assert(age >= 0);
    assert(virusName);
    assert(vaccinated);

    Person p = calloc(1, sizeof(*p));

    p->age = age;
    p->vaccinated = (strcmp(vaccinated, "YES") == 0);
    if (deep_copy) {
        p->citizenID = calloc(strlen(citizenID)+1, sizeof(char));
        strcpy(p->citizenID, citizenID);

        p->firstName = calloc(strlen(firstName) + 1, sizeof(char));
        strcpy(p->firstName, firstName);

        p->lastName = calloc(strlen(lastName) + 1, sizeof(char));
        strcpy(p->lastName, lastName);

        p->country = calloc(strlen(country) + 1, sizeof(char));
        strcpy(p->country, country);

        p->virusName = calloc(strlen(virusName) + 1, sizeof(char));
        strcpy(p->virusName, virusName);

        if (date) {
            p->date = calloc(strlen(date) + 1, sizeof(char));
            strcpy(p->date, date);
        } else 
            p->date = NULL;

    } else {
        p->citizenID = citizenID;
        p->firstName = firstName;
        p->lastName = lastName;
        p->country = country;
        p->virusName = virusName;
        p->date = date;
    }

    return (void*)p;
}

void person_destroy(void *_p) {
    Person p = (Person)_p;

    free(p->citizenID);
    free(p->firstName);
    free(p->lastName);
    free(p->country);
    free(p->virusName);
    free(p->date);
    free(p);
}

int compare_numeric_str(char *s1, char *s2) {
    // the idea is that we get 2 strings that cannot be accomodated by the casual system types,
    // so we have to compare them as strings
    char *p1, *p2;
    p1 = s1;
    p2 = s2;
    // first we have to remove the padding of zeros, if it exists for each string.
    // i.e. s1 = "0001" -> "1"
    while (p1[0] && p1[0] == '0' && p1[1]) p1++;
    while (p2[0] && p2[0] == '0' && p2[1]) p2++;

    // now if one string is larger than the other then its larger as a number
    // if the strings has equal lengths then we should return the value of strlen
    int l1 = strlen(p1), l2 = strlen(p2);
    if (l1 > l2)        return 1;
    else if (l1 < l2)   return -1;
    else                return strcmp(p1, p2); 
}

int person_cmp(void *p1, void *p2) {
    return compare_numeric_str(((Person)p1)->citizenID, ((Person)p2)->citizenID);
}

u_int32_t person_hash(void *p) {
    return hash_i((unsigned char *)((Person)p)->citizenID, ((Person)p)->age);
}


// test main
// int main(int argc, char **argv) {
//     int c = compare_numeric_str(argv[1], argv[2]);
//     if (c > 0) puts("Larger");
//     else if (c < 0) puts("Smaller");
//     else puts("Equal");

//     exit(0);
// }