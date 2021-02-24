#!/bin/bash

# change the internal field separator to parse the files appropriately
OLD_IFS=$IFS
IFS=$'\n'
# declare some constants
ID_MAX=9999 #we get the largest user-defined number and create ids less than or equal to it 
STR_MIN_LEN=3
STR_MAX_LEN=12
AGE_MAX=120
CUR_YEAR=2021
MIN_YEAR=$(($CUR_YEAR-$AGE_MAX))

# error checking for the number of arguments
if [ "$#" -ne 4 ] 
then 
    echo "Usage: ./testFile.sh virusesFile countriesFile numLines duplicatesAllowed"
    exit 1
fi 

if [ ! -f $1 ] 
then
    echo "$1 doesn't exist."
    exit 1
fi

if [ ! -f $2 ] 
then 
    echo "$2 doesn't exist."
    exit 1
fi

if [ -n "$3 " ] && [ "$3" -eq "$3" ] 2>/dev/null 
then 
    if [ "$3" -gt "$ID_MAX" ] && [ "$4" -eq 0 ]
    then 
        echo "The number of lines ($3) is greater than the number of possible ids ($ID_MAX), while duplicates are not allowed. Either allow duplicates or decrease the #lines."
        exit 1
    fi 
else 
    echo "The 3rd arg is #lines, hence it must be an integer."
    exit 1
fi

# get the viruses in an array
viruses=( $(cat "$1") ) 
# get the countries in an array
countries=( $(cat "$2") )
# get the number of lines the output file will contain
numLines=$3
# set the flag that determines if duplicates are allowed or not
duplicatesAllowed=$4
# create an empty array of the 
ids=(  )

# create a new inputFile (delete it if already created)
touch inputFile; rm -f inputFile; touch inputFile;

# function to check if an element (id) is contained in the ids list
contains () {
    # we will check if the first argument is part of the list that start from the second argument
    # we will use the grep command and the echo command in order to find the element. If the element
    # is found then the string produced will be non-empty and the following condition will evaluate to true
    if [ ! -z "`echo ${*:2} | grep -w $1`" ]
    then 
        return 1
    fi

    return 0
}

create_record () {
    #get a random name and surname (lengths in [3, 12])
    name_len=$(($RANDOM % ($STR_MAX_LEN-$STR_MIN_LEN) + $STR_MIN_LEN))
    surname_len=$(($RANDOM % ($STR_MAX_LEN-$STR_MIN_LEN) + $STR_MIN_LEN))
    s="`tr -dc A-Za-z < /dev/urandom | head -c $(($name_len+$surname_len))`"
    name="`echo -n "$s" | head -c $name_len`"
    surname="`expr substr "$s" $name_len $surname_len`"

    # get age
    age=$(($RANDOM % $AGE_MAX + 1)) 

    #get country
    j=$(($RANDOM%${#countries[@]}))
    country="${countries[$j]}"

    #get virus
    j=$(($RANDOM%${#viruses[@]}))
    virus="${viruses[$j]}"

   

    if [ "$(($RANDOM % 2))" -eq 0 ]
    then 
        ans="NO"
    else
        ans="YES"
        #get vaccination date
        day=$(($RANDOM % 30 + 1))
        month=$(($RANDOM % 12 + 1))
        year=$(($RANDOM % $age + ($CUR_YEAR-$age))) # the vaccination date must be in the lifespan of the patien
        vaccination_date="$day-$month-$year"
    fi    

    echo "$id $name $surname $country $age $virus $ans $vaccination_date"
    # reset  values besides id
    name=""
    surname=""
    country=""
    age=""
    virus=""
    ans=""
    vaccination_date=""
}

# create the records
for ((i=0; i<${numLines}; i++))
do
    # first we create a random ID in [0, ID_MAX] and check if the id list contains the id
    id=$(($RANDOM % $ID_MAX))
    contains "$id" "${ids[@]}" # check for duplicates
    ret=$?
    # make sure it's unique (if necessary)
    while [ "$duplicatesAllowed" -eq 0 ] && [ "$ret" -eq 1 ]
    do
        id=$(($RANDOM % $ID_MAX))
        contains "$id" "${ids[@]}" # check for duplicates
        ret=$?
    done
    
    # add the id to the id list
    ids+=( "$id" )

    create_record

    if [ "$duplicatesAllowed" -eq 1 ] && [ "$(($RANDOM%($i+1)))" -eq 0 ]
    then 
        let 'i=i+1'
        create_record
    fi
done > inputFile

# reset the IFS
IFS=$OLD_IFS