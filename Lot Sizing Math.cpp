#include "Problema.h"
#include "Modelo.h"

int main() {

	Modelo Teste("IO_PrA1.txt");

	//Teste.resolver();


	Teste.RELAX_AND_FIX(8, 4);

	return 0;
}