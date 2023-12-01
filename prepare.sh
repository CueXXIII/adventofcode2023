#!/bin/bash

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
exec bash
