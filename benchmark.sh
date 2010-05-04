#!/bin/bash

if [ -n "$1" ]; then
	X=$1
else
	X=1
fi

time (
i=0
while [ $i -lt $X ]; do
	./ice data/start4.pbm data/end4-0.pbm &&
	./ice data/start4.pbm data/end4-1.pbm &&
	./ice data/start4.pbm data/end4-2.pbm &&
	./ice data/start4.pbm data/end4-3.pbm &&
	./ice data/start4.pbm data/end4-4.pbm &&
	./ice data/start5.pbm data/end5.pbm &&
	./ice data/start2.pbm data/end2.pbm &&
	./ice data/start.pbm data/end.pbm &&
	./ice data/start3.pbm data/end3.pbm &&
	echo "ALL PASSED"
	let i+=1
done
)
