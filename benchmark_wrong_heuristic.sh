#!/bin/bash
echo "This takes a looong time with score = a + b,
about half as long with score = a,
and a fraction of a second with score = b"

time (
	./ice-binary data/start4.pbm data/end4-longtime.pbm &&
	echo "DONE"
)
