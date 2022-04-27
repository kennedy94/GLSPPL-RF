#!/bin/bash
# since Bash v4

set -e
path=$(pwd)
echo path= $path

let est=1
let budget=3600

while [ $est -le 10 ]; do
	let i=2
	echo $est
	while [ $i -le 10 ]; do
		pathi=$path/LotSizing/bin/Release
		#echo pathi= $pathi
		cd $pathi
		./LStuni A1.txt  bud3600_A1.csv $est $i $budget 2 &
		./LStuni A2.txt  bud3600_A2.csv $est $i $budget 2 &
		./LStuni A3.txt  bud3600_A3.csv $est $i $budget 2 &
		./LStuni A4.txt  bud3600_A4.csv $est $i $budget 2 &
		./LStuni A5.txt bud3600_A5.csv  $est $i $budget 2 &
		./LStuni B1.txt  bud3600_B1.csv $est $i $budget 2 &
		./LStuni B2.txt  bud3600_B2.csv $est $i $budget 2 &
		./LStuni B3.txt  bud3600_B3.csv $est $i $budget 2 &
		./LStuni B4.txt  bud3600_B4.csv $est $i $budget 2 &
		./LStuni B5.txt bud3600_B5.csv  $est $i $budget 2 & 
		./LStuni C6.txt  bud3600_C6.csv $est $i $budget 2 &
		./LStuni C7.txt  bud3600_C7.csv $est $i $budget 2 &
		./LStuni C8.txt  bud3600_C8.csv $est $i $budget 2 &
		./LStuni C9.txt  bud3600_C9.csv $est $i $budget 2 &
		./LStuni C10.txt bud3600_C10.csv  $est $i $budget 2 & wait
		#./LStuni D6.txt  bud3600_D6.csv $est $i $budget 2 &
		#./LStuni D7.txt  bud3600_D7.csv $est $i $budget 2 &
		#./LStuni D8.txt  bud3600_D8.csv $est $i $budget 2 &
		#./LStuni D9.txt  bud3600_D9.csv $est $i $budget 2 &
		#./LStuni D10.txt bud3600_D10.csv  $est $i $budget 2 &
		#./LStuni E6.txt  bud3600_E6.csv $est $i $budget 2 &
		#./LStuni E7.txt  bud3600_E7.csv $est $i $budget 2 &
		#./LStuni E8.txt  bud3600_E8.csv $est $i $budget 2 &
		#./LStuni E9.txt  bud3600_E9.csv $est $i $budget 2 &
		#./LStuni E10.txt bud3600_E10.csv  $est $i $budget 2 & wait
		echo " $i"
		i=$(( $i + 1 ))
	done

	est=$(( $est + 1 ))
done
