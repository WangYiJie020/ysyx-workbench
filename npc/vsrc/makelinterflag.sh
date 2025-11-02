
# raw call
#  self -f unexist_file ...
# trans to
#  verilator -f myf ...

logfile='./makelinter.log'

echo "call $@" >> $logfile

orign_file=$(echo "$3" | sed 's/\/tmp\/.*\/sources//')
#orign_file=$(echo "$3" | sed 's/tmp//')
echo "think orign is $orign_file" >> $logfile

flag_file="./linter.f"
#find ./*.v | grep -v $3 > $flag_file
find . -name "*.v" -exec realpath {} \; | grep -v $orign_file > $flag_file

shift 2
verilator -f $flag_file --lint-only $@ 2>> $logfile
verilator -f $flag_file --lint-only $@
