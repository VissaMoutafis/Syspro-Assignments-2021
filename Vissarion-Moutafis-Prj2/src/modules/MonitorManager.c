#include "MonitorManager.h"
#include "FileUtilities.h"

// Basic Utilities for monitor trace hashtable
int cmp_trace(Pointer _t1, Pointer _t2) {
    Trace t1, t2;
    t1 = (Trace)_t1;
    t2 = (Trace)_t2;

    return strcmp(t1->country,t2->country);
}

void del_trace(Pointer _t) {
    Trace t = (Trace) _t;
    free(t->country);
    free(_t);
}

u_int32_t hash_trace(Pointer _t) {
    Trace t = (Trace)_t;
    return hash_i((unsigned char*) t->country, strlen(t->country));
}

Pointer create_trace(MonitorTrace *m_trace, char *country, bool deep) {
    Trace t = calloc(1, sizeof(*t));
    t->m_trace = m_trace;
    if (!deep) {
        t->country = country;
    } else {
        t->country = calloc(strlen(country) + 1, sizeof(char));
        strcpy(t->country, country);
    }

    return (Pointer) t;
}

// create monitor manager 
MonitorManager monitor_manager_create(int num_monitors) {
    MonitorManager m = calloc(1, sizeof(*m));

    m->num_monitors = num_monitors;
    m->active_monitors = 0;
    m->countries_index = ht_create(cmp_trace, hash_trace, del_trace);
    m->monitors = calloc(num_monitors, sizeof(MonitorTrace));

    // initialize all pid to -1.
    for (int i=0; i < num_monitors; i++) m->monitors[i].pid=-1;
    
    return m;
}

// add a monitor into the manager
// return the index
int monitor_manager_add(MonitorManager manager, pid_t pid, int in_fifo, int out_fifo) {
    assert(manager);
    
    // first find an empty position
    int i = 0;
    // iterate till we reach the end of the board, or till we find an empty slot
    while (i < manager->num_monitors && manager->monitors[i].pid >= 0)
        i++;

    if (i == manager->num_monitors) {
        fprintf(stderr, "Cannot add more monitors. Max capacity reached.\n");
        return -1;
    }

    MonitorTrace *trace_p = &manager->monitors[i];

    trace_p->pid = pid;
    trace_p->in_fifo = in_fifo;
    trace_p->out_fifo = out_fifo;
    return i;
}

// search for monitor with pid='pid'. 'trace' copies that monitor trace if found. 
// Return index if found else -1
int monitor_manager_search_pid(MonitorManager manager, pid_t pid, MonitorTrace *trace) {
    int index = -1;
    if (trace) {
        memset(trace, 0, sizeof(Trace));
        trace->pid = -1;
    }

    // search for the pid
    for (int i = 0; i < manager->num_monitors; i++) {
        if (pid == manager->monitors[i].pid) {
            // if found set the index and the trace instance 
            index = i;
            *trace = manager->monitors[i];
            break;
        }
    }
    // return the index (whatever that is)
    return index;
}

// get monitor at i-th index. 'trace' copies that monitor trace if found.
// Return true if found else false
bool monitor_manager_get_at(MonitorManager manager, int i, MonitorTrace *trace) {
    bool found = false;
    if (trace) {
        memset(trace, 0, sizeof(Trace));
        trace->pid = -1;
    }

    if (i < manager->num_monitors && manager->monitors[i].pid >= 0) {
        *trace = manager->monitors[i];
        found = true;
    }

    return found;
}


// add a copy of 'country' to i-th monitor
void monitor_manager_add_country(MonitorManager manager, int i, char *country_path) {
    if (i < manager->num_monitors && manager->monitors[i].pid >= 0) {
        char ** country_paths = manager->monitors[i].countries_paths;
        
        // create the new array
        char **new_array = calloc(manager->monitors[i].num_countries + 1, sizeof(char*));
        // copy the old array and delete it
        memcpy(new_array, country_paths, manager->monitors[i].num_countries*sizeof(char*));
        free(country_paths);

        //create the new entry and add it to the new array
        int write_at = manager->monitors[i].num_countries;
        char *new_path_entry = calloc(strlen(country_path)+1, sizeof(char));
        strcpy(new_path_entry, country_path);
        new_array[write_at] = new_path_entry;
        // reset the slot's countries paths array
        manager->monitors[i].countries_paths = new_array;
        manager->monitors[i].num_countries++;
        
        // add a records of that in the hashtable

        // first make a deep copy of the input trace entry
        Trace trace = create_trace(&manager->monitors[i], get_elem_name(country_path), true);

        Pointer old = NULL;
        ht_insert(manager->countries_index, trace, false, &old);

        #ifdef DEBUG
        assert(old == NULL);
        puts(trace->country);
        #endif
    }
}

void monitor_manager_destroy(MonitorManager monitor) {
    
    for (int i = 0; i < monitor->num_monitors; i++) {
        for (int j = 0; j < monitor->monitors[i].num_countries; j++)
            free(monitor->monitors[i].countries_paths[j]);
        free(monitor->monitors[i].countries_paths);
    }
    free(monitor->monitors);
    ht_destroy(monitor->countries_index);
    free(monitor);
}
