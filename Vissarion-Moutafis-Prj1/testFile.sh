#!/bin/bash


# Implemented by Vissarion Moutafis sdi1800119
# Usage: ./testFile.sh virusesFile countriesFile numLines duplicatesAllowed
# virusesFile: the path of the viruses file (one word/line , no blanks)
# countriesFile: the path of the countries file (one word/line, no blanks)
# numLines: the number of lines in the produced-testcase
# duplicatesAllowed:  flag that determines if duplicates are allowed or not (0 unique records, > 0 duplicates )
# 						if the flag is set to 1 then we will include some false or extreme test-lines to challenge 
# 						the error checking of the program

# change the internal field separator to parse the files appropriately
OLD_IFS=$IFS
IFS=$'\n'
# declare some constants
ID_MAX=9999 #we get the largest user-defined number and create ids less than or equal to it 
STR_MIN_LEN=3
STR_MAX_LEN=12
AGE_MAX=120
CUR_YEAR=2021

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

# proper error checking for the third argument (#lines)
if [ -n "$3 " ] && [ "$3" -eq "$3" ] 2>/dev/null 
then 
    if [ "$3" -gt "$ID_MAX" ] && [ "$4" -eq 0 ]
    then 
        echo -e "The number of lines ($3) is greater than the number of possible ids ($ID_MAX), while duplicates are not allowed.\nWarning: Some duplicates WILL be produced."
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
if [ "$numLines" -gt "$ID_MAX" ]
then 
	duplicatesAllowed=1
fi

# the length of false and duplicates recs is 15% of the numLines 
falseRecs=$((15 * $numLines / 100))
if [ "$duplicatesAllowed" -eq 1 ]
then 
	numLines=$(($numLines - $falseRecs))
else
	falseRecs=-1
fi

# random string 
s="`tr -dc A-Za-z < /dev/urandom | head -c 100`"

# create an empty array of the 
recs=( )

# create a new inputFile (delete it if already created)
touch inputFile; rm -f inputFile; touch inputFile;

i=0

while [ "$i" -le  "$numLines" ] 
do
	# get a random id in the given range
	id=$(($RANDOM % $ID_MAX + 1))
	# if the duplicates are not allowed and the id is already taken, choose another one
	while [ "$duplicatesAllowed" -eq 0 ] && [ -n "`printf "%s\n" ${recs[@]} | cut -d ' ' -f1 | grep -w $id`" ]
	do
		id=$(($RANDOM % $ID_MAX + 1))
	done
	# set a random surname and name length within the given length-range
	name_len=$(($RANDOM % ($STR_MAX_LEN-$STR_MIN_LEN + 1) + $STR_MIN_LEN))
    surname_len=$(($RANDOM % ($STR_MAX_LEN-$STR_MIN_LEN + 1) + $STR_MIN_LEN))
    
    # get the random name/surname
    name="`echo $s | fold -w1 | shuf | tr -d '\n' | head -c $name_len`"
	surname="`expr substr "$(echo $s | fold -w1 | shuf | tr -d '\n')" $name_len $surname_len`"
	
	# get a random age within the given range
	age=$(($RANDOM % $AGE_MAX + 1))

	# get a random country
	j=$(($RANDOM % ${#countries[@]}))
    country="${countries[$j]}"

    # get a random virus
    j=$(($RANDOM%${#viruses[@]}))
    virus="${viruses[$j]}"

    ans="NO"

    if [ "$(($id % 2))" -eq 0 ]
    then
    	ans="YES $(( $RANDOM % 30 + 1 ))-$(( $RANDOM % 12 + 1 ))-$(( $CUR_YEAR - $RANDOM % ($age+1) ))"
    fi

    recs+=( "$id $name $surname $country $age $virus $ans" )

    let "i = i+1"
done

function dup_same_virus_corr {
	# choose one to duplicate for the same virus and enter a different answer
	id=$(($RANDOM % $numLines))					# get one id of the existent at random
	line="`echo ${recs[$id]}`"					# get the line
	ans="`echo $line | cut -f7 -d ' '`"		# get the answer (YES or NO)
	person="`echo $line | cut -f1-6 -d ' '`"	# get the personal details
	age="`echo $line | cut -f5 -d ' '`"
	if [ $ans == "YES" ]
	then
		ans="NO"
	else 
		ans="YES $(( $RANDOM % 30 + 1 ))-$(( $RANDOM % 12 + 1 ))-$(( $CUR_YEAR - $RANDOM % ($age+1) ))"
	fi

	# add the record
	recs+=( "$person $ans" )
}

function dup_diff_virus_corr {
	id=$(($RANDOM % $numLines))					# get one id of the existent at random
	line="`echo ${recs[$id]}`"					# get the line
	ans="NO"									
	virus="`echo $line | cut -f6 -d ' '`"
	person="`echo $line | cut -f1-5 -d ' '`"	# get the personal details
	age="`echo $line | cut -f5 -d ' '`"
	
	new_virus=$virus
	while [ $virus == $new_virus ]
	do
		new_virus="${viruses[$(($RANDOM % ${#viruses}))]}"
	done

	if [ "$(($RANDOM % 2))" -eq 0 ]
    then
    	ans="YES $(( $RANDOM % 30 + 1 ))-$(( $RANDOM % 12 + 1 ))-$(( $CUR_YEAR - $RANDOM % ($age+1) ))"
    fi

	# add the record
	recs+=( "$person $new_virus $ans" )
}

function false_virus_ans {
	id=$(($RANDOM % $numLines))					# get one id of the existent at random
	line="`echo ${recs[$id]}`"					# get the line
	pre="`echo $line | cut -f1-6 -d ' '`"
	ans="`echo $line | cut -f7 -d ' '`"
	if [ $ans == "YES" ]
	then
		ans="NO $(( $RANDOM % 30 + 1 ))-$(( $RANDOM % 12 + 1 ))-$(( $RANDOM % ($AGE_MAX+1) + $CUR_YEAR-$AGE_MAX))"
	else
		ans="YES"
	fi
	recs+=( "$pre $ans" )
}

function lost_personal_attr {
	id=$(($RANDOM % $numLines))					# get one id of the existent at random
	line="`echo ${recs[$id]}`"					# get the line
	index=$(($RANDOM % 4 + 2))
	pre="`echo $line | cut -d ' ' -f1-$(($index-1))`"
	rest="`echo $line | cut -d ' ' -f$(($index+1))-8`"
	recs+=( "$pre $rest" )
}


# add different cases of duplicates, not all acceptable
for i in `seq 0 $falseRecs`
do 
	case $(($i % 5)) in
		0)
			dup_same_virus_corr		# the virus is the same but the answer is different
			;;
		1)
			dup_diff_virus_corr		# different virus
			;;
		2)
			false_virus_ans			# insert a duplicate with the vaccination an date info wrong
			;;
		3)
			lost_personal_attr		# insert a record with one attribute lost
			;;
	esac
	
done

printf "%s\n" ${recs[@]} > inputFile
# reset the IFS
IFS=$OLD_IFS