#include "Modelo.h"

void Modelo::resolver(){
	criar_modelo();
	

	//cplex.setParam(IloCplex::TiLim, 3600);
	try {
		//desalocar_matrizes();
		criar_modelo();
		//restriçőes de integralidade
		for (IloInt i = 0; i < N; i++) {
			for (IloInt l = 0; l < M; l++) {
				for (IloInt s = 0; s < W; s++) {
					modelo.add(IloConversion(env, x[i][l][s], ILOBOOL));
				}
			}
		}

		cplex = IloCplex(modelo);
		cplex.exportModel("Modelo.lp");

		cplex.setParam(IloCplex::TiLim, 600);

		IloNum soltime;
		ofstream resultados;
		soltime = cplex.getCplexTime();
		if (!cplex.solve()) {
			env.error() << "Otimizacao do LP mal-sucedida." << endl;
			resultados.open("resultados_modelo.txt", fstream::app);
			resultados << "\t" << cplex.getStatus() << endl;
			resultados.close();
			return;
		}
		soltime = cplex.getCplexTime() - soltime;

		resultados.open("resultados_modelo.txt", fstream::app);
		resultados << "," << cplex.getObjValue() << "," << cplex.getMIPRelativeGap() <<
			"," << soltime << "," << cplex.getStatus() << endl;
		resultados.close();

	}
	catch (IloException& e) {
		cerr << "Erro: " << e.getMessage() << endl;
		cout << "\nErro na inteira" << endl;
		return;
	}
	catch (...) {
		cerr << "Outra excecao" << endl;
	}

}

list<list<variavel>> Modelo::RF_Tm1(int k, list<vector<variavel>> particoes_completas){
	list<list<variavel>> particoes;
	IloInt i, l, s, t;

	for (auto part: particoes_completas){
		int n_por_particao = part.size() / k;
		std::sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.s < j.s;});
		list<variavel> var_list;

		int cont = 1;
		for (auto var : part) {
			var_list.push_back(var);
			if (cont% n_por_particao == 0) {
				particoes.push_back(var_list);
				var_list.clear();
			}
			cont++;
		}
	}

	return particoes;
}

list<list<variavel>> Modelo::RF_Tm2(int k, list<vector<variavel>> particoes_completas) {

	list<list<variavel>> particoes;
	IloInt i, l, s, t;

	for (auto part : particoes_completas) {
		int n_por_particao = part.size() / k;
		std::sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.s > j.s;});
		list<variavel> var_list;

		int cont = 1;
		for (auto var : part) {
			var_list.push_back(var);
			if (cont % n_por_particao == 0) {
				particoes.push_back(var_list);
				var_list.clear();
			}
			cont++;
		}
	}

	return particoes;
}

void Modelo::FIX_AND_OPTIMIZE(IloArray<IloArray<IloBoolArray>> x_hat)
{

	IloInt i, l, s, t;
	int n_vezes = N,
		n_var;

	n_var = N / n_vezes;


	try {
		criar_modelo();
		cplex = IloCplex(modelo);
		cplex.setParam(IloCplex::TiLim, 600 / n_vezes);

		IloNum soltime;
		soltime = cplex.getCplexTime();
		for ( i = 0; i < N; i++)
		{
			for (l = 0; l < M; l++)
			{
				for ( s = 0; s < W; s++)
				{
					modelo.add(IloConversion(env, x[i][l][s], ILOBOOL));
				}
			}
		}
		int vez = 0;

		while (vez < n_vezes)
		{
			IloConstraintArray restricoes(env);

			//fixar variáveis	
			for (i = vez*n_var; i < (vez + 1) * n_var; i++)
			{
				for (l = 0; l < M; l++)
				{
					for (s = 0; s < W; s++)
					{
						IloNum val;

						if (i < n_var) {
							val = x_hat[i][l][s];
						}
						else {
							if (cplex.isExtracted(x[i][l][s])){
								val = round(cplex.getValue(x[i][l][s]));
							}
							else {
								continue;
							}
						}

						restricoes.add(IloConstraint(val == x[i][l][s]));
					}
				}
			}

			modelo.add(restricoes);
			cplex.solve();
			vez++;
		}

				
		soltime = cplex.getCplexTime() - soltime;

		ofstream resultados("resultados.csv", fstream::app);
		resultados << "," << cplex.getObjValue() << "," << cplex.getMIPRelativeGap() <<
			"," << soltime << "," << cplex.getStatus() << "," << "Fix And Optimize" << endl;
		resultados.close();

	}
	catch (IloException& e) {
		cplex.error() << "Erro: " << e.getMessage() << endl;
		std::cout << "\nErro na inteira" << endl;
		return;
	}
	catch (...) {
		cerr << "Outra excecao" << endl;
	}
}

list<list<variavel>> Modelo::RF_Pr1(int k, list<vector<variavel>> particoes_completas) {
	list<list<variavel>> particoes;
	IloInt i, l, s, t;

	for (auto part : particoes_completas) {
		std::sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.i > j.i;});
		int n_por_particao = part.size() / k;
		vector<int> demanda(N, 0);
		for (i = 0; i < N; i++) {
			for (t = 1; t < T; t++) {
				demanda[i] += d[i][t];
			}
		}

		std::sort(part.begin(), part.end(), [&](variavel i, variavel j) {return demanda[i.i] > demanda[j.i];});
		list<variavel> var_list;

		int cont = 1;
		for (auto var : part) {
			var_list.push_back(var);
			if (cont % n_por_particao == 0) {
				particoes.push_back(var_list);
				var_list.clear();
			}
			cont++;
		}
	}

	return particoes;
}

list<list<variavel>> Modelo::RF_Pr2(int k, list<vector<variavel>> particoes_completas) {
	list<list<variavel>> particoes;
	IloInt i, l, s, t;

	for (auto part : particoes_completas) {
		std::sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.i > j.i;});
		int n_por_particao = part.size() / k;
		vector<int> demanda(N, 0);
		for (i = 0; i < N; i++) {
			for (t = 1; t < T; t++) {
				demanda[i] += d[i][t];
			}
		}

		std::sort(part.begin(), part.end(), [&](variavel i, variavel j) {return demanda[i.i] < demanda[j.i];});
		list<variavel> var_list;

		int cont = 1;
		for (auto var : part) {
			var_list.push_back(var);
			if (cont % n_por_particao == 0) {
				particoes.push_back(var_list);
				var_list.clear();
			}
			cont++;
		}
	}

	return particoes;
}

list<list<variavel>> Modelo::RF_Pr3(int k, list<vector<variavel>> particoes_completas) {
	list<list<variavel>> particoes;
	IloInt i, l, s, t;

	for (auto part : particoes_completas) {
		std::sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.i > j.i;});
		int n_por_particao = part.size() / k;
		vector<int> criticidade(N, M);
		for (i = 0; i < N; i++) {
			for (auto maq : SP)
			{
				for (auto prod : maq)
				{
					if (i == prod) {
						criticidade[i] --;
						break;
					}
				}
			}
		}

		std::sort(part.begin(), part.end(), [&](variavel i, variavel j) {return criticidade[i.i] > criticidade[j.i];});
		list<variavel> var_list;

		int cont = 1;
		for (auto var : part) {
			var_list.push_back(var);
			if (cont % n_por_particao == 0) {
				particoes.push_back(var_list);
				var_list.clear();
			}
			cont++;
		}
	}

	return particoes;
}

list<list<variavel>> Modelo::RF_Mc1(int k, list<vector<variavel>> particoes_completas) {
	list<list<variavel>> particoes;
	IloInt i, l, s, t;

	for (auto part : particoes_completas) {

		std::sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.l > j.l;});
		int n_por_particao = part.size() / k;
		vector<float> eficiencia(M, 0);
		vector<float> EFICIENCIA(part.size(), 0);

		for (l = 0; l < M; l++) {
			for (auto& prod : SP[l]) {
				eficiencia[l] += p[prod][l];
			}
			eficiencia[l] /= SP[l].size();
		}


	
		std::sort(part.begin(), part.end(), [&](variavel i, variavel j) {return eficiencia[i.l] > eficiencia[j.l];});

		list<variavel> var_list;
		int cont = 1;
		for (auto var : part) {
			var_list.push_back(var);
			if (cont % n_por_particao == 0) {
				particoes.push_back(var_list);
				var_list.clear();
			}
			cont++;
		}
	}

	return particoes;
}

list<list<variavel>> Modelo::RF_Mc2(int k, list<vector<variavel>> particoes_completas) {
	list<list<variavel>> particoes;
	IloInt i, l, s, t;

	for (auto part : particoes_completas) {
		std::sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.l > j.l;});
		int n_por_particao = part.size() / k;
		vector<float> criticidade_maquina(M, 0);
		vector<int> criticidade(N, M);
		for (i = 0; i < N; i++) {

			for (auto maq : SP)
			{
				for (auto prod : maq)
				{
					if (i == prod) {
						criticidade[i] --;
						break;
					}
				}
			}
		}

		for (l = 0; l < M; l++) {
			for (auto prod : SP[l]) {
				criticidade_maquina[l] += criticidade[prod];
			}
		}


		std::sort(part.begin(), part.end(), [&](variavel i, variavel j) {return criticidade_maquina[i.l] > criticidade_maquina[j.l];});

		list<variavel> var_list;
		int cont = 1;
		for (auto var : part) {
			var_list.push_back(var);
			if (cont % n_por_particao == 0) {
				particoes.push_back(var_list);
				var_list.clear();
			}
			cont++;
		}
	}

	return particoes;
}

IloArray<IloArray<IloBoolArray>> Modelo::RELAX_AND_FIX(int estrategia, int k, bool _fix_opt) {
	list<list<variavel>> particao;
	IloInt i, l, s, t;
	list<vector<variavel>> particoes_completas;

	vector<variavel> var_list;
	for ( i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			for ( s = 1; s < W; s++) {
				var_list.push_back(variavel(i, l, s));
			}
		}
	}
	particoes_completas.push_back(var_list);

	switch (estrategia) {
	case 1:
		cout << "Estratégia RF_Mc1 escolhida! \n\n";
		particao = RF_Mc1(k, particoes_completas);
		break;
	case 2:
		cout << "Estratégia RF_Mc2 escolhida! \n\n";
		particao = RF_Mc2(k, particoes_completas);
		break;
	case 3:
		cout << "Estratégia RF_Tm1 escolhida! \n\n";
		particao = RF_Tm1(k, particoes_completas);
		break;
	case 4:
		cout << "Estratégia RF_Tm1 escolhida! \n\n";
		particao = RF_Tm2(k, particoes_completas);
		break;
	case 5:
		cout << "Estratégia RF_Pr1 escolhida! \n\n";
		particao = RF_Pr1(k, particoes_completas);
		break;
	case 6:
		cout << "Estratégia RF_Pr2 escolhida! \n\n";
		particao = RF_Pr2(k, particoes_completas);
		break;
	case 7:
		cout << "Estratégia RF_Pr3 escolhida! \n\n";
		particao = RF_Pr3(k, particoes_completas);
		break;
	case 8:
		cout << "Estratégia HRF_Hb1 escolhida! \n\n";
		particao = HRF_Hb1(particoes_completas, 2, 2, 2, 7, 3, 1);
		break;
	case 9:
		cout << "Estratégia HRF_Hb2 escolhida! \n\n";
		particao = HRF_Hb2(particoes_completas, 2, 4, 7, 3);
		break;
	default:
		cerr << "Erro: Nenhuma estrategia escolhdida!" << endl;
		exit(0);
	}

	try {
		criar_modelo();
		cplex = IloCplex(modelo);
		cplex.setParam(IloCplex::TiLim, 600 / particao.size());

		IloNum soltime;
		soltime = cplex.getCplexTime();


		int contador_particoes = 0;
		for (auto par : particao) {

			//restriçőes de integralidade
			cout << endl;
			for (auto par_i : par) {
				modelo.add(IloConversion(env, x[par_i.i][par_i.l][par_i.s], ILOBOOL));
			}


			//cplex = IloCplex(modelo);
			cplex.solve();

			contador_particoes++;
			if (contador_particoes == particao.size() || _fix_opt)
				break;


			//fixar variáveis
			IloConstraintArray restricoes(env);
			for (auto par_i : par) {
				IloNum val = round(cplex.getValue(x[par_i.i][par_i.l][par_i.s]));
				restricoes.add(IloConstraint(val == x[par_i.i][par_i.l][par_i.s]));
			}

			modelo.add(restricoes);
		}
		IloArray<IloArray<IloBoolArray>> x_hat(env, N);
		for (i = 0; i < N; i++)	{
			x_hat[i] = IloArray<IloBoolArray>(env, M);
			for ( l = 0; l < M; l++) {
				x_hat[i][l] = IloBoolArray(env, W);
				for ( s = 0; s < W; s++)
				{
					if (cplex.isExtracted(x[i][l][s])) {
						x_hat[i][l][s] = round(cplex.getValue(x[i][l][s]));
					}
				}
			}
		}


		soltime = cplex.getCplexTime() - soltime;

		ofstream resultados("resultados.csv", fstream::app);
		resultados << "," << cplex.getObjValue() << "," << cplex.getMIPRelativeGap() <<
			"," << soltime << "," << cplex.getStatus() << "," << estrategia << endl;
		resultados.close();

		return x_hat;

	}
	catch (IloException& e) {
		cplex.error() << "Erro: " << e.getMessage() << endl;
		cout << "\nErro na inteira" << endl;
		exit(0);
	}
	catch (...) {
		cerr << "Outra excecao" << endl;
		exit(0);
	}
}

void Modelo::criar_modelo() {
	modelo = IloModel(env, "modelo_linear");
	cplexvar_initiate();
	fo();
	restricoes();
}

void Modelo::cplexvar_initiate() {
	IloInt i, j, l, t, s;
	q = IloArray<IloArray<IloFloatVarArray>>(env, N);
	for (i = 0; i < N; i++) {
		q[i] = IloArray<IloFloatVarArray>(env, M);
		for (l = 0; l < M; l++) {
			q[i][l] = IloFloatVarArray(env, W, 0.0, IloInfinity);
		}
	}

	char strnum[30];
	x = IloArray<IloArray<IloFloatVarArray>>(env, N);
	for (i = 0; i < N; i++) {
		x[i] = IloArray<IloFloatVarArray>(env, M);
		for (l = 0; l < M; l++) {
			x[i][l] = IloFloatVarArray(env, W, 0.0, 1.0);
			for (s = 0; s < W; s++)	{
				sprintf_s(strnum, "x(%d,%d,%d)", i, l, s);
				x[i][l][s].setName(strnum);

			}
		}
	}


	y = IloArray<IloArray<IloArray<IloFloatVarArray>>>(env, N);
	for (i = 0; i < N; i++) {
		y[i] = IloArray<IloArray<IloFloatVarArray>>(env, N);
		for (j = 0; j < N; j++) {
			y[i][j] = IloArray<IloFloatVarArray>(env, M);
			for (l = 0; l < M; l++) {
				y[i][j][l] = IloFloatVarArray(env, W, 0.0, 1.0);
			}
		}
	}

	I_plus = IloArray<IloFloatVarArray>(env, N);
	I_minus = IloArray<IloFloatVarArray>(env, N);

	for (i = 0; i < N; i++) {
		I_plus[i] = IloFloatVarArray(env, T, 0, IloInfinity);
		I_minus[i] = IloFloatVarArray(env, T, 0, IloInfinity);
	}
}

void Modelo::fo() {
	IloInt i, j, t, s, l;
	OBJETIVO = IloExpr(env);

	C_setup = IloNumVar(env);
	C_prod = IloNumVar(env);
	C_est = IloNumVar(env);
	C_Bko = IloNumVar(env);

	for (i = 0; i < N; i++) {
		for (t = 1; t < T; t++) {
			OBJETIVO += h[i] * I_plus[i][t];
		}
	}
	modelo.add(OBJETIVO == C_est);
	OBJETIVO.clear();

	for (i = 0; i < N; i++) {
		for (t = 1; t < T; t++) {
			OBJETIVO += g[i] * I_minus[i][t];
		}
	}
	modelo.add(OBJETIVO == C_Bko);
	OBJETIVO.clear();

	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			for (l = 0; l < M; l++) {
				for (s = 1; s < W; s++) {
					OBJETIVO += cs[i][j][l] * y[i][j][l][s];
				}
			}
		}
	}

	modelo.add(OBJETIVO == C_prod);
	OBJETIVO.clear();

	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			for (s = 1; s < W; s++) {
				OBJETIVO += cp[i][l] * q[i][l][s];

			}
		}
	}
	modelo.add(OBJETIVO == C_setup);
	OBJETIVO.clear();

	modelo.add(IloMinimize(env, C_setup + C_prod + C_est + C_Bko)).setName("FO");

}

void Modelo::restricoes() {
	IloInt i, j, l, s, t;
	IloExpr soma(env);

	//(2)
	for (i = 0; i < N; i++)
	{
		for (t = 1; t < T; t++) {
			for (l = 0; l < M; l++) {
				for (s = (t - 1) * W_p + 1; s <= t * W_p; s++) {
					soma += q[i][l][s];
				}
			}
			modelo.add(I_plus[i][t - 1] - I_minus[i][t - 1] + soma - I_plus[i][t] + I_minus[i][t] == d[i][t]).setName("(02)");
			soma.clear();
		}
	}

	//(3)
	for (t = 0; t < T; t++) {
		for (i = 0; i < N; i++) {
			soma += I_plus[i][t];
		}
		modelo.add(soma <= CA).setName("(03)");
		soma.clear();
	}

	//(4)
	for (l = 0; l < M; l++) {
		for (t = 1; t < T; t++) {

			for (auto i : SP[l]) {
				for (s = (t - 1) * W_p + 1; s <= t * W_p; s++) {
					soma += p[i][l] * q[i][l][s];
				}
			}

			for (auto i : SP[l]) {
				for (auto j : SP[l]) {
					for (s = (t - 1) * W_p + 1; s <= t * W_p; s++) {
						soma += st[i][j][l] * y[i][j][l][s];
					}
				}
			}


			modelo.add(soma <= CP[l][t]).setName("(04)");
			soma.clear();
		}
	}

	//(5)
	for (l = 0; l < M; l++) {
		for (auto i : SP[l]) {
			for (t = 1; t < T; t++)
			{
				for (s = (t - 1) * W_p + 1; s <= t * W_p; s++) {
					modelo.add(p[i][l] * q[i][l][s] <= CP[l][t] * x[i][l][s]).setName("(05)");
				}
			}
		}

	}

	//(6)
	for (l = 0; l < M; l++) {
		for (auto i : SP[l]) {
			for (s = 1; s < W; s++) {
				modelo.add(q[i][l][s] >= lm[i][l] * (x[i][l][s] - x[i][l][s - 1])).setName("(06)");
			}
		}
	}

	//(7)
	for (l = 0; l < M; l++) {
		for (s = 1; s < W; s++) {

			for (auto i : SP[l]) {
				soma += x[i][l][s];
			}

			modelo.add(soma == 1).setName("(07)");;
			soma.clear();

		}
	}

	//(8)
	for (l = 0; l < M; l++) {
		for (auto i : SP[l]) {
			for (auto j : SP[l]) {
				for (s = 1; s < W; s++) {
					modelo.add(y[i][j][l][s] >= x[i][l][s - 1] + x[j][l][s] - 1).setName("(08)");
				}
			}
		}
	}

	//(9)

	for (l = 0; l < M; l++) {
		for (i = 0; i < N; i++) {
			modelo.add(x[i][l][0] == 0).setName("(09)");
		}
	}

	for (i = 0; i < N; i++) {
		modelo.add(I_minus[i][0] == I0_minus[i]).setName("(10)");
		modelo.add(I_plus[i][0] == I0_plus[i]).setName("(10)");
	}
}

list<list<variavel>> Modelo::HRF_Hb2(list<vector<variavel>> particoes_completas,
	int k1, int k2, int estrat1, int estrat2) {

	list<list<variavel>> particao;
	switch (estrat1) {
	case 1:
		cout << "Estratégia RF_Mc1 escolhida! \n\n";
		particao = RF_Mc1(k1, particoes_completas);
		break;
	case 2:
		cout << "Estratégia RF_Mc2 escolhida! \n\n";
		particao = RF_Mc2(k1, particoes_completas);
		break;
	case 3:
		cout << "Estratégia RF_Tm1 escolhida! \n\n";
		particao = RF_Tm1(k1, particoes_completas);
		break;
	case 4:
		cout << "Estratégia RF_Tm1 escolhida! \n\n";
		particao = RF_Tm2(k1, particoes_completas);
		break;
	case 5:
		cout << "Estratégia RF_Pr1 escolhida! \n\n";
		particao = RF_Pr1(k1, particoes_completas);
		break;
	case 6:
		cout << "Estratégia RF_Pr2 escolhida! \n\n";
		particao = RF_Pr2(k1, particoes_completas);
		break;
	case 7:
		cout << "Estratégia RF_Pr3 escolhida! \n\n";
		particao = RF_Pr3(k1, particoes_completas);
		break;
	default:
		cerr << "Erro: Nenhuma estrategia escolhdida!" << endl;
		exit(0);
	}

	for (auto part: particoes_completas){
		part.clear();
	}
	particoes_completas.clear();


	for (auto part: particao){
		particoes_completas.push_back(vector<variavel>(part.begin(), part.end()));
	}

	switch (estrat2) {
	case 1:
		cout << "Estratégia RF_Mc1 escolhida! \n\n";
		particao = RF_Mc1(k2, particoes_completas);
		break;
	case 2:
		cout << "Estratégia RF_Mc2 escolhida! \n\n";
		particao = RF_Mc2(k2, particoes_completas);
		break;
	case 3:
		cout << "Estratégia RF_Tm1 escolhida! \n\n";
		particao = RF_Tm1(k2, particoes_completas);
		break;
	case 4:
		cout << "Estratégia RF_Tm1 escolhida! \n\n";
		particao = RF_Tm2(k2, particoes_completas);
		break;
	case 5:
		cout << "Estratégia RF_Pr1 escolhida! \n\n";
		particao = RF_Pr1(k2, particoes_completas);
		break;
	case 6:
		cout << "Estratégia RF_Pr2 escolhida! \n\n";
		particao = RF_Pr2(k2, particoes_completas);
		break;
	case 7:
		cout << "Estratégia RF_Pr3 escolhida! \n\n";
		particao = RF_Pr3(k2, particoes_completas);
		break;
	default:
		cerr << "Erro: Nenhuma estrategia escolhdida!" << endl;
		exit(0);
	}

	return particao;
}

list<list<variavel>> Modelo::HRF_Hb1(list<vector<variavel>> particoes_completas,
	int k1, int k2, int k3, int estrat1, int estrat2, int estrat3) {

	list<list<variavel>> particao;
	switch (estrat1) {
	case 1:
		cout << "Estratégia RF_Mc1 escolhida! \n\n";
		particao = RF_Mc1(k1, particoes_completas);
		break;
	case 2:
		cout << "Estratégia RF_Mc2 escolhida! \n\n";
		particao = RF_Mc2(k1, particoes_completas);
		break;
	case 3:
		cout << "Estratégia RF_Tm1 escolhida! \n\n";
		particao = RF_Tm1(k1, particoes_completas);
		break;
	case 4:
		cout << "Estratégia RF_Tm1 escolhida! \n\n";
		particao = RF_Tm2(k1, particoes_completas);
		break;
	case 5:
		cout << "Estratégia RF_Pr1 escolhida! \n\n";
		particao = RF_Pr1(k1, particoes_completas);
		break;
	case 6:
		cout << "Estratégia RF_Pr2 escolhida! \n\n";
		particao = RF_Pr2(k1, particoes_completas);
		break;
	case 7:
		cout << "Estratégia RF_Pr3 escolhida! \n\n";
		particao = RF_Pr3(k1, particoes_completas);
		break;
	default:
		cerr << "Erro: Nenhuma estrategia escolhdida!" << endl;
		exit(0);
	}

	for (auto part : particoes_completas) {
		part.clear();
	}
	particoes_completas.clear();


	for (auto part : particao) {
		particoes_completas.push_back(vector<variavel>(part.begin(), part.end()));
	}

	switch (estrat2) {
	case 1:
		cout << "Estratégia RF_Mc1 escolhida! \n\n";
		particao = RF_Mc1(k2, particoes_completas);
		break;
	case 2:
		cout << "Estratégia RF_Mc2 escolhida! \n\n";
		particao = RF_Mc2(k2, particoes_completas);
		break;
	case 3:
		cout << "Estratégia RF_Tm1 escolhida! \n\n";
		particao = RF_Tm1(k2, particoes_completas);
		break;
	case 4:
		cout << "Estratégia RF_Tm1 escolhida! \n\n";
		particao = RF_Tm2(k2, particoes_completas);
		break;
	case 5:
		cout << "Estratégia RF_Pr1 escolhida! \n\n";
		particao = RF_Pr1(k2, particoes_completas);
		break;
	case 6:
		cout << "Estratégia RF_Pr2 escolhida! \n\n";
		particao = RF_Pr2(k2, particoes_completas);
		break;
	case 7:
		cout << "Estratégia RF_Pr3 escolhida! \n\n";
		particao = RF_Pr3(k2, particoes_completas);
		break;
	default:
		cerr << "Erro: Nenhuma estrategia escolhdida!" << endl;
		exit(0);
	}

	for (auto part : particoes_completas) {
		part.clear();
	}
	particoes_completas.clear();


	for (auto part : particao) {
		particoes_completas.push_back(vector<variavel>(part.begin(), part.end()));
	}

	switch (estrat3) {
	case 1:
		cout << "Estratégia RF_Mc1 escolhida! \n\n";
		particao = RF_Mc1(k3, particoes_completas);
		break;
	case 2:
		cout << "Estratégia RF_Mc2 escolhida! \n\n";
		particao = RF_Mc2(k3, particoes_completas);
		break;
	case 3:
		cout << "Estratégia RF_Tm1 escolhida! \n\n";
		particao = RF_Tm1(k3, particoes_completas);
		break;
	case 4:
		cout << "Estratégia RF_Tm1 escolhida! \n\n";
		particao = RF_Tm2(k3, particoes_completas);
		break;
	case 5:
		cout << "Estratégia RF_Pr1 escolhida! \n\n";
		particao = RF_Pr1(k3, particoes_completas);
		break;
	case 6:
		cout << "Estratégia RF_Pr2 escolhida! \n\n";
		particao = RF_Pr2(k3, particoes_completas);
		break;
	case 7:
		cout << "Estratégia RF_Pr3 escolhida! \n\n";
		particao = RF_Pr3(k3, particoes_completas);
		break;
	default:
		cerr << "Erro: Nenhuma estrategia escolhdida!" << endl;
		exit(0);
	}

	return particao;
}