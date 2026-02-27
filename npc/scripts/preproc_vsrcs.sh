#!/bin/sh

if [ $# -ne 2 ]; then
		echo "Usage : $0 <dir> <CPU_DESIGN_NAME>"
		exit 1
fi

SEARCH_DIR=$1

VSRCS=$(find $SEARCH_DIR -name "*.sv")

for src in $VSRCS; do
	sed -i '1i /*verilator public_on*/' $(src)
	sed -i -e '$a /*verilator public_off*/' $(src)
	sed -i "s/layer\$\(\w\+\)/$2_\1/g" $(src)
done
