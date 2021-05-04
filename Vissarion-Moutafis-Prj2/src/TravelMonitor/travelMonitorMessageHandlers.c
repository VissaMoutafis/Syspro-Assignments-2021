#include "TravelMonitor.h"
#include "StructManipulation.h"

void get_bf_from_child(void *_monitor, char *msg, int msg_len, void *return_args[]) {
    // just print it to see it works
    // printf("Len: %d -- ", msg_len);
    // for (int i = 0; i < msg_len; i++)
    //     if (msg[i])
    //         putchar(msg[i]);
    //     else
    //         putchar('-');
    // puts("");

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
    // so just add the new bf tuple to, one for every 
    for (int i = 0; i < cols-1; i++)
        virus_stats_add_bf((VirusStats)entry, countries[i], bf);

    // NOW WE HAVE TO ADD BF IN A LIST OF UNIQUE BF STRUCTS SO THAT WE CAN FREE LATER
    list_insert(((VirusStats)entry)->unique_bfs, bf, true);

    for (int i = 0; i < cols; i++) free(parsed_header[i]);
    free(parsed_header);
}

void travel_request_handler(void *monitor, char *msg, int msg_len, void *return_args[]) {
    // ret args = {char *response, char *date}
    
}