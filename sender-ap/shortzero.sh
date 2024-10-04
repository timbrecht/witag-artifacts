#!/bin/bash
scp root@192.168.1.1:/root/${1}-*.txt .

for i in {5..8}
do 
	echo $i
	./new_run.sh  ${1}-${i}.txt ${1}-${i}.out  > temp
	if [[ $i -gt 0 ]]
	then
		python3 findpatterns.py ${1}-${i}.out >> ${1}.out
	else
		 python3 findpatterns.py ${1}-0.out > ${1}.out

	fi
	echo "" >> ${1}.out
	echo "" >> ${1}.out
done
