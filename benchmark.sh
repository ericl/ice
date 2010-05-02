#!/bin/bash

time (
	./ice start4.pbm end4-0.pbm &&
	./ice start4.pbm end4-1.pbm &&
	./ice start4.pbm end4-2.pbm &&
	./ice start4.pbm end4-3.pbm &&
	./ice start4.pbm end4-4.pbm &&
	./ice start5.pbm end5.pbm &&
	./ice start2.pbm end2.pbm &&
	./ice start.pbm end.pbm &&
	./ice start3.pbm end3.pbm&&
	echo "ALL PASSED"
)
