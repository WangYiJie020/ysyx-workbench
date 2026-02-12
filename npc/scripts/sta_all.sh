#!/bin/bash
cat $1 | grep -v '/' | sed 's/\.sv//' | while read -r name; do
	echo "make sta $name"
	make sta STA_DEST=$name > /dev/null
done
