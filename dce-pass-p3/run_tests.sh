#!/bin/bash
FILE=$1
OUTPUT=$2
if [ "$OUTPUT" == "" ]
then
	OUTPUT="dummy"
fi

if [ "$FILE" = "-all" ]
then
    opt-3.6 -S -load Release/P3.so -dce $FILE -o ./$OUTPUT-dce-optimal.opt.ll
    opt-3.6 -S -load Release/P3.so -dce-liveness $FILE -o ./$OUTPUT-dce-liveness.opt.ll
    opt-3.6 -S -load Release/P3.so -dce-ssa $FILE -o ./$OUTPUT-dce-ssa.opt.ll

else
    opt-3.6 -S -load Release/P3.so -dce $FILE -o ./$OUTPUT-dce-optimal.opt.ll
    opt-3.6 -S -load Release/P3.so -dce-liveness $FILE -o ./$OUTPUT-dce-liveness.opt.ll
    opt-3.6 -S -load Release/P3.so -dce-ssa $FILE -o ./$OUTPUT-dce-ssa.opt.ll
    echo "LIVENESS"
    diff ./$OUTPUT-dce-optimal.opt.ll ./$OUTPUT-dce-liveness.opt.ll
	echo "SSA"
    diff ./$OUTPUT-dce-optimal.opt.ll ./$OUTPUT-dce-ssa.opt.ll

	rm ./$OUTPUT-dce-optimal.opt.ll
    rm ./$OUTPUT-dce-liveness.opt.ll
    rm ./$OUTPUT-dce-ssa.opt.ll    
fi