#!/bin/bash
# since Bash v4

set -e
path=$(pwd)
echo path= $path


let est=1
let budget=3600
let i=1

while [ $budget -le 3600 ]; do
	pathi=$path/LotSizing/bin/Release
	echo pathi= $pathi
	cd $pathi
	./LStuni A1.txt modelo.csv $est $i $budget 2 & 
	./LStuni A2.txt modelo.csv $est $i $budget 2 & 
	./LStuni A3.txt modelo.csv $est $i $budget 2 & 
	./LStuni A4.txt modelo.csv $est $i $budget 2 & 
	./LStuni A5.txt modelo.csv $est $i $budget 2 & wait
	./LStuni B1.txt modelo.csv $est $i $budget 2 & 
	./LStuni B2.txt modelo.csv $est $i $budget 2 & 
	./LStuni B3.txt modelo.csv $est $i $budget 2 & 
	./LStuni B4.txt modelo.csv $est $i $budget 2 & wait
	./LStuni B5.txt modelo.csv $est $i $budget 2 & 
	./LStuni C1.txt modelo.csv $est $i $budget 2 & 
	./LStuni C2.txt modelo.csv $est $i $budget 2 & 
	./LStuni C3.txt modelo.csv $est $i $budget 2 & 
	./LStuni C4.txt modelo.csv $est $i $budget 2 & 
	./LStuni C5.txt modelo.csv $est $i $budget 2 & wait
	./LStuni D1.txt modelo.csv $est $i $budget 2 & 
	./LStuni D2.txt modelo.csv $est $i $budget 2 & 
	./LStuni D3.txt modelo.csv $est $i $budget 2 & 
	./LStuni D4.txt modelo.csv $est $i $budget 2 & 
	./LStuni D5.txt modelo.csv $est $i $budget 2 & wait
	./LStuni E1.txt modelo.csv $est $i $budget 2 & 
	./LStuni E2.txt modelo.csv $est $i $budget 2 & 
	./LStuni E3.txt modelo.csv $est $i $budget 2 & 
	./LStuni E4.txt modelo.csv $est $i $budget 2 & 
	./LStuni E5.txt modelo.csv $est $i $budget 2 & wait
	let budget=budget+600
done