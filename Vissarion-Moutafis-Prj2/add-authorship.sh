#!/bin/bash 

add-authorship-file() {
	# file is $1
	# msg is $2
	## CHANGE THIS LINE ONLY
	sed -i '1s/^/\/**\n*\tSyspro Project 2\n*\t Written By Vissarion Moutafis sdi1800119\n**\/\n \n/' "$1"
}

add-authorship-dir() {
	# dir is $1
	# msg is $2
	# regex is $3

	#first get the files of the dir
	files="$(ls "$1" | grep "$3")"
	# take care of files
	for file in $files; do
		echo "Adding Author's Message in $1/$file"
		add-authorship-file "$1/$file" "$msg"
	done

	# take care of directories
	dirs="$(ls -l "$1" | grep "^d" | cut -d ":" -f 2 | cut -d ' ' -f 2)"  
	for d in $dirs; do
		add-authorship-dir "$1/$d" "$2" "$3"
	done
}

add-authorship() {
	# $1 the file/dir
	# $1 the regex that will choose the files

	# usage message (DO NOT CHANGE)
	usage="Usage:  ~\$ add-authorship project-dir|file [regex to choose files if $1 is a dir]"
	

	elem="$1"

	# check for argument
	if [[ "$#" -lt 1 ]]; then
		echo "ERROR: Not enough args."
		echo $usage
	fi
	# check for arg type
	if [ ! -f "$elem" ] && [ ! -d "$elem" ]; then
		echo "ERROR: Argument not a file or directory."
		echo $usage
	fi

	regex="*"
	if [ -d "$elem" ]; then
		# case the element is a dir
		if [ ! -z $2 ]; then
			regex="$2"
		fi

		add-authorship-dir "$elem" "" "$regex"
	else
		# case the element is a file
		add-authorsip-file "$elem" ""
	fi
}

add-authorship "." "\.[c,h]$"
echo "Done."