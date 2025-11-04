#!/bin/bash


src=${*: -1}
dst=${*: -2:1}
cc=$1
flags=${*: 2:$#-4}

dst_S=${dst%.o}.S
if [[ "$src" == *.S ]] then
echo "run ::: cp $src $dst_S"
  cp $src $dst_S
  riscv64-linux-gnu-$cc -E $flags -S -o $dst_S $src
else
echo "run ::: riscv64-linux-gnu-$cc $flags -S -o $dst_S $src"
  riscv64-linux-gnu-$cc $flags -S -o $dst_S $src
fi

#cat $dst_S

echo "state ::: finish make dst_S"

# replace pseudo instructions for load/store
sp="[[:space:]]*"
sp_require="[[:space:]]+"
reg="[[:alnum:]]+"
comma="$sp,$sp"
symbol="[[:alnum:]\._]+"
sed -E -i -e "s/(l[bhw]u?)${sp_require}(${reg})${comma}(${symbol})(${sp}[-+]${sp}${symbol})?${sp}\$/la \2, \3\4; \1 \2, 0(\2);/" \
          -e "s/(s[bhw])${sp_require}(${reg})${comma}(${symbol})(${sp}[-+]${sp}${symbol})?${comma}(${reg})${sp}\$/la \5, \3\4; \1 \2, 0(\5);/" $dst_S

#cat $dst_S
echo "state ::: finish replace pseudo inst"

#cat $dst_S
# insert inst-replace.h to each .h files
minirv_path=$AM_HOME/tools/minirv
lut_bin_path=$minirv_path/lut.bin
sed -i "1i#include \"$minirv_path/inst-replace.h\"" $dst_S
flock $minirv_path/.lock -c "test -e $lut_bin_path || (cd $minirv_path && gcc gen-lut.c && ./a.out && rm a.out)"


#sed -E -i 's/TEST_RR_OP\((.*),[[:space:]]*(div|mul),(.*),(.*),(.*)\)/TEST_CASE(\1,x14,\3,li x10,MASK_XLEN(\4);li x11,MASK_XLEN(\5);call __\2si3;addi x14,x10,0;)/g' $dst_S

#sed -E -i 's/TEST_RR_OP\((.*),[[:space:]]*(div|mul),(.*),(.*),(.*)\)/TEST_CASE(\1,x14,\3,li x10,MASK_XLEN(\4);li x11,MASK_XLEN(\5);call __\2si3;addi x14,x10,0;)/g' $dst_S

sed -E -i "s/(div|mul)${sp_require}(${reg})${comma}(${reg})${comma}(${reg})/addi x10,\3,0;addi x11,\4,0;call __\1si3;addi \2,x10,0/" $dst_S

cat $dst_S
echo "run ::: riscv64-linux-gnu-gcc -I$src_dir $flags -D_LUT_BIN_PATH=\"$lut_bin_path\" -Wno-trigraphs -c -o $dst $dst_S"

src_dir=`dirname $src`
riscv64-linux-gnu-gcc -I$src_dir $flags -D_LUT_BIN_PATH=\"$lut_bin_path\" -Wno-trigraphs -c -o $dst $dst_S

#riscv64-linux-gnu-gcc -I$src_dir $flags -D_LUT_BIN_PATH=\"$lut_bin_path\" -Wno-trigraphs -S -o ~/proced.S $dst_S
echo "state ::: finish insert inst-replace.h"

# set a non-standard extension flag in e_flags to indicate minirv
/bin/echo -ne '\x80' | dd of=$dst bs=1 seek=39 count=1 conv=notrunc 2> /dev/null
