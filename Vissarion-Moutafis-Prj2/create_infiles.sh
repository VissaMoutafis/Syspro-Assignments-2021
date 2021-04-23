#!/bin/bash

##################################################################################
# Test-Directory Hierarchy Generator, written by Vissarion Moutafis (sdi1800119) #
##################################################################################

# basic usage help method
usage="Usage: create_infiles.sh inputFile input_dir numFilesPerDirectory"

# print help section
if [[ "$@" == *"-h"* ]] || [[ "$@" == *"--help"* ]]
then
    echo $usage
    echo ""
    echo "inputFile: A file-path that is readable and consists of records that will populate the directories."
    echo "input_dir: The name of the non-existent directory that we will create."
    echo "numFilesPerDirectory: The number of files each directory will contain."
    exit 1
fi

# check the ammount of arguments
if [ "$#" -ne 3 ]
then
    printf "Error: Wrong Amount of arguments.\n%s\n" "$usage"
    exit 1
fi

# check if input file exists
if [ ! -f "$1" ]
then
    printf "Error: File %s does not exist.\n%s\n" "$1" "$usage"
    exit 1
fi

# make sure that the input dir does not exist
if [ -f "$2" ] || [ -d "$2" ]
then
    printf "Error: There is already a file or directory with name '%s'.\n%s\n" "$2" "$usage"
    exit 1
fi

#make sure that the 3rd argument is numeric
if ! [ "$3" -eq "$3" ] 2>/dev/null
then
    printf "Error: numFilesPerDirectory is not numeric.\n%s\n" "$usage"
    exit 1
fi

# set the variables to use (given by the user arguments)
infile="$1"
dirname="$2"
numFilesPerDir="$3"

# get all the data for less disk operations
data="$(cat $infile)"

# create the directory
mkdir "$dirname"
# set the main path
mainPath="$dirname"

# find all different countries
countries="$(echo "$data" | cut -d " " -f4 | sort -u)"

for country in $countries
do
    # create the directory for the respective country
    mkdir "$mainPath/$country"
    # set the current path for ease of use
    curPath="$mainPath/$country"

    # create numFilePerDirectory files and populate them from the country data
    for i in `seq 1 $numFilesPerDir`
    do
        # create the new records file
        touch "$curPath/$country-$i.txt"
        # populate it with records that occur every $i-1 recs in the data set for the respective country
        echo "$(echo "$data" | grep "$country" | awk "NR%$numFilesPerDir==$i-1")" >> "$curPath/$country-$i.txt"
    done
done