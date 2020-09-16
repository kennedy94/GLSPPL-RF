#include "Problema.h"
#include "Modelo.h"

int main(int argc, char* argv[]) {

	if (argc < 1)
		return 0;

	const char* instancia = argv[1];
	int estrategia = atoi(argv[2]),
		k = atoi(argv[3]),
		fix_opt = atoi( argv[4]);
	//Modelo Teste("IO_PrA1.txt");
	Modelo Teste(instancia);

	//Teste.resolver();
	//Teste.resolver_linear();

	//Teste.FIX_AND_OPTIMIZE(Teste.RELAX_AND_FIX(estrategia, k, true));
	Teste.RELAX_AND_FIX(estrategia, k, fix_opt);
	return 0;
}