#!/bin/bash
echo "This never finishes, which is silly given that the state
1 move previous is trivially findable."

time (
	./ice-binary data/start2.pbm data/end2-longtime.pbm &&
	echo "DONE"
)
