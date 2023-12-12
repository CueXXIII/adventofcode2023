#!/bin/bash

# As per request from https://www.reddit.com/r/adventofcode/wiki/faqs/automation
# replace RUN_BY with your contact information and recalculate the
# sha1 hash, or change the check in other ways to ensure it only runs
# on your system(s).
RUN_BY=github.com/CueXXIII
if [ "$(echo "$RUN_BY $(getent passwd $USER)" | sha1sum)" != "df4815d42e2e26c0a42f040272cdf61305c7eea0  -" ]; then
	echo "Please read the script how to run it."
	exit 1;
fi

set -e

read -p "Day to prepare: " day
read -p "Headline of puzzle: " h1

puzzle="$(printf %02d "$day") - $h1"
if test -d "$puzzle"; then
	echo "Puzzle \"$h1\" already created."
	exit 1
fi

mkdir "$puzzle"

read -p "Program name: " obj

cd "$puzzle"

<../Makefile perl -pe "s/SRC=/SRC=$obj.cc/" >Makefile
<../template.cc perl -pe "s/<example.txt>/<input.txt>/" >"$obj.cc"

touch example.txt
vi example.txt

git add Makefile example.txt
git commit -m "day $day skeleton"

gvim $obj.cc

if [ -f ../cookies.txt ]; then
    wget --user-agent "github.com/CueXXIII/adventofcode2023/blob/master/prepare.sh by $RUN_BY" --load-cookies ../cookies.txt -O input.txt https://adventofcode.com/2023/day/$day/input
    if [ -f input.txt ]; then
	# let's look at our input
        cat input.txt
	echo
        ls -l input.txt
    fi
fi

exec bash
