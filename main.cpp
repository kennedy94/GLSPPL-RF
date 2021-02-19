#include "Problema.h"
#include "Modelo.h"

int main(int argc, char* argv[]) {

	if (argc < 1)
		return 0;

	const char* instancia = argv[1];
	string saida = "resultados.txt";
	int estrategia = -1,
		k = -1,
		fix_opt = 0;
	if (argc > 2)
		saida = atoi(argv[2]);

	if (argc > 3)
		estrategia = atoi(argv[3]);

	if (argc > 4) {
		k = atoi(argv[4]);
		
	}
	if (argc > 5) {
		fix_opt = atoi(argv[5]);
	}

	//Modelo Teste("IO_PrA1.txt");
	Modelo Teste(instancia);

	Teste.resolver();
	//Teste.resolver_linear();

	//Teste.FIX_AND_OPTIMIZE(Teste.RELAX_AND_FIX(estrategia, k, true));
	//Teste.RELAX_AND_FIX(estrategia, saida, k, fix_opt);
	//Teste.RELAX_AND_FIX_Estrat1(k);
	return 0;
}