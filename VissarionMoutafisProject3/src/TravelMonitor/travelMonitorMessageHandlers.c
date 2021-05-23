/**
*	Syspro Project 2
*	 Written By Vissarion Moutafis sdi1800119
**/
 
#include "TravelMonitor.h"
#include "StructManipulation.h"

void get_bf_from_child(void *_monitor, int opcode, char *msg, int msg_len, void *return_args[]) {
    // just print it to see it works
    #ifdef DEBUG
    printf("Len: %d -- ", msg_len);
    for (int i = 0; i < msg_len; i++)
        if (msg[i])
            putchar(msg[i]);
        else
            putchar('-');
    puts("");
    #endif

    // we know that the first 10 bytes are the byte-length of fields : <virus name><SEP><country 1><SEP>...<SEP><country n>
    // and after that we have the bf msg that we could use to create a new BF
    
    TravelMonitor monitor = (TravelMonitor)_monitor;

    // first get the header bytes
    char header_bytes_str[10+1];
    memset(header_bytes_str, 0, 11);
    memcpy(header_bytes_str, msg, 10);
    int header_bytes = atoi(header_bytes_str) - 10;

    // now get the header
    char header[header_bytes+1];
    memset(header, 0, header_bytes+1);
    memcpy(header, msg+10, header_bytes);
    // extract all the things we want to keep from the header
    int cols = 0;
    char ** parsed_header = parse_line(header, &cols, SEP);
    char *virus_name = parsed_header[0];
    char ** countries = &(parsed_header[1]);

    // and get the bf buffer and create a new BF
    char *bf_buffer = msg + 10 + header_bytes;
    BF bf = bf_create_from_buffer(bf_buffer, msg_len-10-header_bytes, SEP[0]);
    assert(bf);

    // Now we have to add properly the new bf. First find the according 
    // virus_stats in the hash (if there is none, create one)
    struct virus_stats dummy_vs = {.virus_name = virus_name};
    Pointer entry = NULL;
    if (!ht_contains(monitor->virus_stats, &dummy_vs, &entry)) {
        // the entry does not exist. create one and add it in the ht
        entry = virus_stats_create(virus_name);
        Pointer old = NULL;
        ht_insert(monitor->virus_stats, entry, false, &old);
    }

    // at this point <entry> pointer points to the appropriate virus stats entry
    // either update the existent or add the new bf tuple to the respective countries
    VirusStats vs = (VirusStats)entry;
    Pointer old = NULL;
    struct bf_tuple dummy_bft = {.country = countries[0]}; // try for the first country (all countries point to the same bf)
    if ((old=sl_search(vs->bf_per_countries, &dummy_bft))) {
        // the BF already exists so we must update the previous one
        BFTuple bft = (BFTuple)old;
        bf_union(bft->bf, bf);
    } else {
        // the bf does not exist, so just insert it to every country
        for (int i = 0; i < cols-1; i++)
            virus_stats_add_bf(vs, countries[i], bf); // either add or update the bloom filter
    }
    
    // NOW WE HAVE TO ADD BF IN A LIST OF UNIQUE BF STRUCTS SO THAT WE CAN FREE LATER
    list_insert((vs)->unique_bfs, bf, true);

    for (int i = 0; i < cols; i++) free(parsed_header[i]);
    free(parsed_header);
}

void travel_request_handler(void *monitor, int opcode, char *msg, int msg_len, void *return_args[]) {
    // ret args = {char *response, char *date}
    // msg: <YES/NO><1 space character><date of vaccination if answer is YES>
    char *ans = calloc(msg_len+1, sizeof(char));
    memcpy(ans, msg, msg_len);

    if (strcmp(ans, "NO") == 0) {
        // response == NO, date = NULL
        *((char **)return_args[0]) = ans;
        *((char**)return_args[1]) = NULL;
    } else {
        // response == YES, we must set date
        int cols=0; 
        char **parsed_ans = parse_line(ans, &cols, FIELD_SEPARATOR);
        
        // error checking
        assert(cols == 2);

        *((char**)return_args[0]) = parsed_ans[0];
        *((char **)return_args[1]) = parsed_ans[1];
        free(ans);
        free(parsed_ans);
    }
}

void get_vaccination_status(void *monitor, int opcode, char *msg, int msg_len, void *return_args[]) {
    // return_args = {char *vaccination_records}
    // msg = vaccinations record without a '\0'
    *((char **)return_args[0]) = NULL;
    if (msg_len) {
        char *b = calloc(msg_len + 1, sizeof(char));
        memcpy(b, msg, msg_len);
        *((char **)return_args[0]) = b;
    }
}