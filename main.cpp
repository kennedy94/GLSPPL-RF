#include "Problema.h"
#include "RF.h"

int main(int argc, char* argv[]) {

	if (argc < 1)
		return 0;

	const char* instancia = argv[1];
	const char* saida = argv[2];
	int estrategia = -1,
		k = -1,
		divisao_tempo = -1;
	double BUDGET = 3600.0;

	if (argc > 3)
		estrategia = atoi(argv[3]);

	if (argc > 4) {
		k = atoi(argv[4]);
	}

	if (argc > 5) {
		BUDGET = atoi(argv[5]);
	}
	if (argc > 6) {
		divisao_tempo = atoi(argv[6]);
	}

	RF Teste(instancia);

	//Teste.FIX_AND_OPTIMIZE(Teste.RELAX_AND_FIX(estrategia, k, true));
	cout << "Executando :" << estrategia << "," << saida << "," << k << "," << BUDGET << endl;
	Teste.RELAX_AND_FIX(estrategia, saida, k, BUDGET, divisao_tempo);
	return 0;
}