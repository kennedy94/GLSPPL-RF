!/bin/bash
since Bash v4

set -e
path=$(pwd)
echo path= $path

let est=1
let budget=3600
let j=1

#problems=("A6.txt" "A7.txt"  "A8.txt"  "A9.txt"  "A10.txt" "B6.txt"  "B7.txt"  "B8.txt"  "B9.txt"  "B10.txt" "C6.txt"  "C7.txt"  "C8.txt"  "C9.txt"  "C10.txt" "D6.txt"  "D7.txt"  "D8.txt"  "D9.txt"  "D10.txt" "E6.txt"  "E7.txt"  "E8.txt"  "E9.txt"  "E10.txt")

problems=("A6.txt" "B6.txt" "C6.txt" "D6.txt" "E6.txt" )

while [ $est -le 1 ]; do
	let i=2
	if [ $est == 1 ]
	then
		let i=1
	fi
	echo $est
	while [ $i -le 10 ]; do
		pathi=$path/LotSizing/bin/Release/TestesCustos/
		echo pathi= $pathi
		cd $pathi

		for inst in ${problems[@]}; do
			./LStuni $inst saida.csv $est $i $budget 2 & 
			j=$(( $j + 1 ));
			if [ $( expr  $j % 20) == 0 ]
			then
				wait
			fi
		done
		i=$(( $i + 1 ))
	done
	est=$(( $est + 1 ))
done
