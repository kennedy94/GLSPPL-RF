#include "Problema.h"
#include "Modelo.h"

int main() {

	Modelo Teste("IO_PrA1.txt");

	//Teste.resolver();


	Teste.FIX_AND_OPTIMIZE(Teste.RELAX_AND_FIX(1, 8, true));

	return 0;
}