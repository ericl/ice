#!/bin/bash

./ice-binary "$@"
exit $?

if [ ! -e ice-binary ]; then
	echo "need to compile first"
	exit 1
fi

mkdir -p cache

result=`cat "$1" "$2" | openssl md5`

if [ -e cache/$result ]; then
	cat cache/$result
else
	./ice-binary "$@" | tee cache/tmp
	mv cache/tmp cache/$result
fi

exit 0
