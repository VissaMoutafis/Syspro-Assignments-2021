sdi1800119 - Vissarion Moutafis

make all : compile everything

make run : install and run with default args (check makefile to adjust them)

make valgrind-run : install and run with valgrind with default parameters

make re-run : recompile and run with default args.


NOTE FOR INSTRUCTORS: 
> During compilation I use -Wall and -Werr. The given hash functions file had a small cast warning 
  that -Wall got and -Werr printed as an error, so I added the needed type-cast to compile.
> All the data structures use void * in order to avoid coding redudancy and increase the reusability of our structures. Also we use wrapers/headers 
  for each struct in order to implement a more STL like version of the data structures we use.
> !!! IMPORTANT: Valgrind runs will show a large number of bytes allocated. THIS IS NOT ENTIRELY TRUE. During the creation of virus structs we will create a bloom filter (of 100KB). This results to creating
many "dummy" structs of already recorded viruses, for searching while the app is running, that are actually deleted after the successful search of the virus index. So during the actual interactive run time 
 we will never hold a very large number of bytes and since we are leak free, we don't have to worry about a heap overflow.


The App is divided in the following modules:
- TTY API, that contain code to handle user input, 
take care of basic error checking according to the given format and re-format the input
in order to pass it into the vaccine monitor module for further use.
- modules-directory, which contain some basic data structure implementation, as well as the implementation
of the requested data structures. Specifically we provide with a hash table, a doubly linked list, a Bloom Filter and a Skip List.
- manipulation utilitites, that handle some data formats in order to provide the app with modularity and prevent code redudancy.
- the vaccine monitor module, that uses all the previous modules for providing the user with a dynamic database,
that will effectively handle new input and print the proper error checking messages when a false input or command occur.


The Basic Structure we followed for the data base organization in the vaccine monitor app is the following:
- Keep all the valid records in a hashtable so that we can access their instance in O(1). This is the only valid instance of one citizen, all other instances are just pointers to the specific memory.
- The Virus Information (Skip lists, bloom filters, virus name) are all wraped around a struct, stored in a doubly linked list, as this is one of the fastest ways of aggregating the data needed.
- Skip lists contain a struct that keeps a pointer to the relative person and the date of vaccination or null, respective to whether he/she is vaccinated or not.
- We also provide a country index, in the notion of a list of lists, for the respective aggregating queries. The list keeps a CountryIndex wrapper that keeps a list of citizens (pointers to the hashtables instances)
  and the country name. As in the previous struct we prefer the list since its aggregating the best.
- In-order to reduce data-redudancy we added a Country Index pointer into every citizen so that we keep sizeof(pointer) 
  bytes to get the country name, while the latter gets mapped in memory only once (during the creation of the country index)

BF: The bloom filter module is implemented with bit manipulation commands and masking,
in order to avoid wasting 8 bytes for 1 bit change. There are only some basic functionalities that are impemented by the book.

SL: The skip list module was implemented according to the video provided by the instructors. The user must provide max height and sl_factor, which defines the height of every node purely by chance.
Every list node has an entry pointer and an array of pointers that point to other nodes. Each level-list of the skip list node is implemented by this array which length is only determined by its height.

HT: We use a double probing linear hash, that will actually be way more efficient in memory terms and will also provide as with fast searching given that the hashes and the probes are avoiding collisions.
The size of the has is always 2^n starting from an initial size defines in the .h file. The probe step always keeps in mind that it should be an odd number (we implement that in a special function, check the code).

List: This is a double linked list with all its basic methods and some special methods that empower the user to handle the iterator wrappers that contain the item, without actually having any
knowledge of the interior design. This is also implemented by the book.

> TTY API:
Basic tty prompt that will acquire a command expression parse it, check if it resembles any of the given valid-commands, 
do some superficial error checking and finaly pass this expression to the next chain of command
the vaccine monitor app. It is called in main function and the parsing and error checking happens also in main program routine.

> STRUCT MANIPULATION API:
Basic utilities (create, compare, destroy, hash, print, apply etc) that help us to manipulate the declared structs such as citizens, country indices, virus info structs etc, by just providing them to the 
data structures that will keep them. These are needed to increase code modularity and to provide the data structures with the proper behaviour each time, as the ADTs use void *pointers.

> VACCINE MONITOR API:
The vaccine monitor is essentially a wraper for a hash table and 2 lists, besides some numerical attributes. We provide an API, through some basic routines:

vaccine_monitor_initialize : initialize some basic variables of the vaccine monitor app
vaccine_monitor_finalize : finilizing routine the monitor 
vaccine_monitor_create : creation routine, insertion from file (if given), returns a wrapper of vaccine monitor structs
vaccine_monitor_destroy : destruction routine, de-allocates all the memory the wrapper holds
vaccine_monitor_act : the basic act routine that will accept a monitor wrapper instance, and command_index (check the allowed formats array), and a values string (basically a switch statement)

The basic functionalities detailed in the readings of the assignment and the follow-ups in K24-piazza forum, are implemented using all this structs and are actually do the error checking on their own.
When something is not properly set or valued, an error_flag is turned to true, and the proper error message is stored in error_msg. When the running routine terminates
we check for an error and print the respective message. In case of success we do the same with a buffer called ans_buf.

Due to the reusability of code segments according different statistics and the citizen-insertion-routine we separated some code blocks and used them for 2 or more 
vaccine monitor functions, providing special condition handling. (check the insert method, usage for insert from file, insertCitizen, vaccinateNow).
In the same notion we implemented some helper functions to help with the parsing, error checking and pretty printing of errors and query-answers.
These are located in Utilities file. Check Below Section.

Finaly we provide the user with a "help" guide for proper command inputs, that is printed in case the user provide the TTY with a command that is not right,
by the standards of the tty api (wrong amount of args, mis-spelled command name, etc).

NOTES FOR THE INSRTUCTORS:
- inserting from file : nothing changed according to directives
- vaccine status bloom : nothing changed, according to directives 
- vaccine status : nothing changed accirding to directives 
- population status: estimating (for a country specifically of foreach country)
                     the quantity (#yes in date range) / (#yes + #no in total)
- population status by age: same as in "population status" but for specific age ranges,

- NOTE that in the above 2 features we will traverse the list of country citizens to extract statistics foreach country,
  and for every citizen we will search the skip lists of the given virus 
- NOTE in the above 2 features we give the posibility to execute without a
  date range so they could take from 1 to 4 arguments 
- essentially vaccinate now and insert record use the same insertion function 
  with the only difference that vaccinate now will set the last 2 field to 
  "YES <current-date>" and then use insert.
- list not vaccinated persons: list all the non vaccinated persons of a virus, 
  so we will essentially just print the not-vaccinated skip list of the given virus 
- exit: free all the allocated memory.


UTILITIES FILE:
This file contains specific utilities such as my own version of fgets or a generic string parser, that help me
keeping the logic simple and the memory allocated specific pointer for easier use and deallocation. Also there exists some 
routines to manipulate date strings so that there is no redudancy in the critical routines of my code. 

NOTE:
Some functions are of no use, because this file is 
part of my personal utility library and is used almost in every assignment I delivered since K24-Prj1.
So you can be sure that this is gonna exist (maybe in different versions) in every assignment I will submit.