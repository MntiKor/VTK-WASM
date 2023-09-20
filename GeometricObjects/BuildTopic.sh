#!/bin/bash

topic=$1
count_working=0
count_fails=0
for example in ${topic}/*/
do
	mkdir ${example}/build
	cd ${example}/build
	cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DVTK_DIR=/VTK-install/Release/lib/cmake/vtk -DCMAKE_BUILD_TYPE=Release ..
	if [ $? -ne 0 ]
	then
		echo $(basename ${example}) >> /work/doesntcompile.txt
		count_fails+=1
	else
		cmake --build .
		if [ $? -ne 0 ]
		then
			echo $(basename ${example}) >> /work/doesntcompile.txt
			count_fails+=1
		else
			count_working+=1
		fi
	fi
	cd /work
done
echo ${count_working} worked
echo ${count_fails} failed
