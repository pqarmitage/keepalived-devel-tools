#!/bin/bash

for f in $(git status | grep modified: | sed -e "s/.*: *//"); do
	[[ $f =~ \.c ]] || continue

	file=${f##*/}
	file=${file/.c/.h}

	grep -q "^+* *# *include[ 	]*\"$file" $f
	[[ $? -eq 0 ]] && continue

	mkdir -p tmp/new/${f%/*}
	cp -p $f tmp/new/$f
	sed -i -e "s/\(^[ 	]*#[ 	]*include[ 	]*\"\)/#include \"$file\"\n\1/" tmp/new/$f

	grep -q "^+* *# *include[ 	]*\"$file" tmp/new/$f
	[[ $? -ne 0 ]] && echo tmp/new/$f NOT edited || echo tmp/new/$f edited
done
