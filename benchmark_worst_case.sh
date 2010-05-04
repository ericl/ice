#!/bin/bash
echo "This takes a long time, which is silly given that the state
1 move previous is trivially findable."

time (
	./ice data/start2.pbm data/end2-longtime.pbm &&
	echo "DONE"
)
