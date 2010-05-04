#!/bin/bash

if [ -n "$1" ]; then
	X=$1
else
	X=1
fi

time (
i=0
while [ $i -lt $X ]; do
	./ice data/start44.pbm data/end44.pbm &&
	./ice data/start44.pbm data/end44-1.pbm &&
	./ice data/start44.pbm data/end44-2.pbm &&
	./ice data/start44.pbm data/end44-3.pbm &&
	./ice data/start44.pbm data/end44-4.pbm &&
	./ice data/start44.pbm data/end44-5.pbm &&
	echo "ALL PASSED"
	let i+=1
done
)
