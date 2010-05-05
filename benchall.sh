#!/bin/bash

if [ -n "$1" ]; then
	X=$1
else
	X=1
fi

time (
i=0
while [ $i -lt $X ]; do
	./ice data/s44.pbm data/e44.pbm;
	./ice data/start2.pbm data/end2-longtime.pbm &&
	./ice data/startspiral.pbm data/endspiral.pbm &&
	./ice data/start4.pbm data/end4-0.pbm &&
	./ice data/start4.pbm data/end4-1.pbm &&
	./ice data/start4.pbm data/end4-2.pbm &&
	./ice data/start4.pbm data/end4-3.pbm &&
	./ice data/start4.pbm data/end4-4.pbm &&
	./ice data/start5.pbm data/end5.pbm &&
	./ice data/start5a.pbm data/end5a.pbm &&
	./ice data/start2.pbm data/end2.pbm &&
	./ice data/start61C.pbm data/end61C.pbm &&
	./ice data/start.pbm data/end.pbm &&
	./ice data/start3.pbm data/end3.pbm &&
	./ice data/start4.pbm data/end4-longtime.pbm &&
	./ice data/start44.pbm data/end44.pbm &&
	./ice data/start44.pbm data/end44-1.pbm &&
	./ice data/start44.pbm data/end44-2.pbm &&
	./ice data/start44.pbm data/end44-3.pbm &&
	./ice data/start44.pbm data/end44-4.pbm &&
	./ice data/start44.pbm data/end44-5.pbm &&
	! ./ice data/startsparse.pbm data/endsparse.pbm &&
	echo "ALL PASSED"
	let i+=1
done
)
