#include "Modelo.h"

ILOINCUMBENTCALLBACK2(mycallback, IloNum&, tempo_incumbent, IloNum, starttime) {
	tempo_incumbent = IloCplex::IncumbentCallbackI::getCplexTime() - starttime;
}

void Modelo::resolver() {
	criar_modelo();

	//ofstream nconst;
	//nconst.open("nconstraints.csv", fstream::app);
	//for (int l = 0; l < M; l++)
	//{
	//	for (int i = 0; i < N; i++)
	//	{
	//		if (l_produz_i[l][i]) {
	//			for (int s = 1; s < W; s++) {
	//				nvar++;
	//			}
	//		}
	//	}
	//}

	//
	//nconst << instancia << "," << nconstraints << "," << nvar << endl;

	//nconst.close();
	//return;

	try {
		//desalocar_matrizes();
		criar_modelo();
		//restrições de integralidade
		for (IloInt i = 0; i < N; i++) {
			for (IloInt l = 0; l < M; l++) {
				if (l_produz_i[l][i])
				{
					for (IloInt s = 0; s < W; s++) {
						modelo.add(IloConversion(env, x[i][l][s], ILOBOOL));
					}
				}
			}
		}

		cplex = IloCplex(modelo);

		cplex.setParam(IloCplex::Param::MIP::Display, 0);
		cplex.setParam(IloCplex::TiLim, 3600);
		//cplex.setParam(IloCplex::Param::Preprocessing::RepeatPresolve, 0);

		IloNum soltime;
		ofstream resultados;
		soltime = cplex.getCplexTime();
		tempo_incumbent = soltime;
		cplex.use(mycallback(env, tempo_incumbent, soltime));


		if (!cplex.solve()) {
			env.error() << "Otimizacao do LP mal-sucedida." << endl;
			resultados.open("resultados_modelo.txt", fstream::app);
			resultados << "\t" << cplex.getStatus() << endl;
			resultados.close();
		}
		soltime = cplex.getCplexTime() - soltime;

		resultados.open("resultados_modelo.txt", fstream::app);
		resultados << instancia << "," << cplex.getBestObjValue() << "," << cplex.getObjValue() << "," << cplex.getMIPRelativeGap() <<
			"," << tempo_incumbent << "," << soltime << "," << cplex.getNnodes() << endl;
		resultados.close();

		bool viavel = teste_de_viabilidade();

		resultados.open("solucao.txt", fstream::app);
		resultados << "q =" << endl;
		for (IloInt l = 0; l < M; l++) {
			for (IloInt i = 0; i < N; i++) {
				if (l_produz_i[l][i])
				{
					for (IloInt s = 0; s < W; s++) {
						//if (cplex.isExtracted(x[i][l][s])) {
						//	if (cplex.getValue(x[i][l][s]) != 0.0) {
						//		resultados << "x " << i << "," << l << "," << s << "=" << cplex.getValue(x[i][l][s]) << endl;
						//	}
						//}
						if (cplex.isExtracted(q[i][l][s])) {
							if (cplex.getValue(q[i][l][s]) > 0.000001) {
								resultados << cplex.getValue(q[i][l][s]) << ",";
							}
							else {
								resultados << ",";
							}
						}
						else {
							resultados << ",";
						}

					}
				}
				resultados << endl;
			}
			resultados << endl << endl;
		}

		resultados << "x =" << endl;
		for (IloInt l = 0; l < M; l++) {
			for (IloInt i = 0; i < N; i++) {
				if (l_produz_i[l][i])
				{
					for (IloInt s = 0; s < W; s++) {

						if (cplex.isExtracted(x[i][l][s])) {
							if (cplex.getValue(x[i][l][s]) > 0.000001) {
								resultados << cplex.getValue(x[i][l][s]) << ",";
							}
							else {
								resultados << ",";
							}
						}
						else {
							resultados << ",";
						}

					}
				}
				resultados << endl;
			}
			resultados << endl << endl;
		}


		for (IloInt i = 0; i < N; i++) {
			for (IloInt j = 0; j < N; j++) {
				for (IloInt l = 0; l < M; l++) {
					if (l_produz_i[l][i] && l_produz_i[l][j])
					{
						for (IloInt s = 0; s < W; s++) {
							//if (cplex.isExtracted(y[i][j][l][s])) {
							//	if (cplex.getValue(y[i][j][l][s]) != 0.0) {
							//		resultados << "y " << i << "," << j << "," << l << "," << s << "=" << cplex.getValue(y[i][j][l][s]) << endl;
							//	}
							//}
						}
					}
				}
			}
		}

		resultados << "I_plus =" << endl;
		for (IloInt i = 0; i < N; i++) {
			for (IloInt t = 0; t < T; t++) {
				if (cplex.isExtracted(I_plus[i][t])) {
					resultados << cplex.getValue(I_plus[i][t]) << ",";
				}
				else {
					resultados << ",";
				}
			}
			resultados << endl;
		}

		resultados << "\n\nI_minus =" << endl;
		for (IloInt i = 0; i < N; i++) {
			for (IloInt t = 0; t < T; t++) {
				if (cplex.isExtracted(I_minus[i][t])) {
					resultados << cplex.getValue(I_minus[i][t]) << ",";
				}
				else {
					resultados << ",";
				}
			}
			resultados << endl;
		}


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

void Modelo::resolver_linear()
{
	criar_modelo();


	//cplex.setParam(IloCplex::TiLim, 3600);
	try {
		//desalocar_matrizes();
		criar_modelo();

		cplex = IloCplex(modelo);
		cplex.exportModel("P1.lp");
		//cplex.setParam(IloCplex::Param::Emphasis::Numerical, 1);
		//cplex.setParam(IloCplex::TiLim, 20);

		IloNum soltime;
		ofstream resultados;
		soltime = cplex.getCplexTime();

		if (!cplex.solve()) {
			env.error() << "Otimizacao do LP mal-sucedida." << endl;
			resultados.open("resultados_modelo.txt", fstream::app);
			resultados << "\t" << cplex.getStatus() << endl;
			resultados.close();
		}
		soltime = cplex.getCplexTime() - soltime;




		resultados.open("resultados_modelo.txt", fstream::app);
		resultados << instancia << "," << cplex.getObjValue() << "," << soltime << "," << cplex.getStatus() << endl;
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

list<list<variavel>> Modelo::RF_Tm1(int k, list<vector<variavel>> particoes_completas) {
	list<list<variavel>> particoes;
	IloInt i, l, s, t;

	for (auto part : particoes_completas) {
		int n_por_particao = part.size() / k;
		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.s < j.s;});
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

list<list<variavel>> Modelo::RF_Tm2(int k, list<vector<variavel>> particoes_completas) {

	list<list<variavel>> particoes;
	IloInt i, l, s, t;

	for (auto part : particoes_completas) {
		int n_por_particao = part.size() / k;
		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.s > j.s;});
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

list<list<variavel>> Modelo::RF_Pr1(int k, list<vector<variavel>> particoes_completas) {
	list<list<variavel>> particoes;
	IloInt i, l, s, t;

	for (auto part : particoes_completas) {
		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.i > j.i;});
		int n_por_particao = part.size() / k;
		vector<int> demanda(N, 0);
		for (i = 0; i < N; i++) {
			for (t = 1; t < T; t++) {
				demanda[i] += d[i][t];
			}
		}

		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return demanda[i.i] > demanda[j.i];});
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
		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.i > j.i;});
		bool divisivel = !(part.size() % k);
		int n_por_particao = part.size() / k;
		vector<int> demanda(N, 0);
		for (i = 0; i < N; i++) {
			for (t = 1; t < T; t++) {
				demanda[i] += d[i][t];
			}
		}

		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return demanda[i.i] < demanda[j.i];});
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
		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.i > j.i;});
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

		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return criticidade[i.i] > criticidade[j.i];});
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
	vector<double> processamento_medio(M, 0.0);
	vector<double> custo_medio(M, 0.0);

	for (l = 0; l < M; l++) {
		for (auto& prod : SP[l]) {
			processamento_medio[l] += p[prod][l];
			custo_medio[l] += cp[prod][l];
		}
		processamento_medio[l] /= (double)SP[l].size();
		processamento_medio[l] += custo_medio[l] / (double)SP[l].size();
	}



	for (auto part : particoes_completas) {

		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.l > j.l;});
		int n_por_particao = part.size() / k;


		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return processamento_medio[i.l] < processamento_medio[j.l];});

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

	vector<int> criticidade_maquina(M, 0);
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

	for (auto part : particoes_completas) {
		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.l > j.l;});
		int n_por_particao = part.size() / k;

		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return criticidade_maquina[i.l] > criticidade_maquina[j.l];});

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

void Modelo::RELAX_AND_FIX(int estrategia, const char* saida, int k, double BUDGET) {
	list<list<variavel>> particao;
	

	IloInt i, l, s, t;
	list<vector<variavel>> particoes_completas;

	vector<variavel> var_list;
	vector < vector<vector< bool>>>
		eh_inteira(N),
		ja_add(N);
	for (i = 0; i < N; i++) {
		eh_inteira[i] = vector<vector<bool>>(M);
		ja_add[i] = vector<vector<bool>>(M);
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {
				eh_inteira[i][l] = vector<bool>(W, false);
				ja_add[i][l] = vector<bool>(W, false);
				t = 1;
				for (s = 1; s < W; s++) {
					var_list.push_back(variavel(i, l, s, t));
					if (s % W_p == 0)
						t++;

				}
			}
		}
	}
	particoes_completas.push_back(var_list);

	//calcular relaxação linear e distâncias da integralidade
	{
		criar_modelo();
		cplex = IloCplex(modelo);

		//resolver linear
		cplex.setParam(IloCplex::Threads, 1);
		cplex.setParam(IloCplex::Param::MIP::Display, 0);
		cplex.solve();


		for (auto& par : particoes_completas)
		{
			for (auto& var : par) {
				var.dist = min(cplex.getValue(x[var.i][var.l][var.s]), 1.0 - cplex.getValue(x[var.i][var.l][var.s]));
			}
		}
	}


	switch (estrategia) {
	case 1:
		particao = RF_S1(particoes_completas);
		break;
	case 2:
		particao = RF_S2(particoes_completas);
		break;
	case 3:
		particao = RF_S3(particoes_completas);
		break;
	case 4:
		particao = RF_S4(particoes_completas, k);
		break;
	case 5:
		particao = RF_S5(particoes_completas, k);
		break;
	case 6:
		particao = RF_S6(particoes_completas);
		break;
	case 7:
		particao = RF_S7(particoes_completas);
		break;
	case 8:
		particao = RF_S8(particoes_completas);
		break;
	case 10:
		particao = RF_S10(particoes_completas, k);
		break;
	default:
		cerr << "Erro: Nenhuma estrategia escolhdida!" << endl;
		exit(0);
	}
	IloNum soltime;
	int contador_particoes = 0,
		contador_variaveis_trans = 0;
	auto start = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds;

	try {
		criar_modelo();
		
		//cplex.setParam(IloCplex::Param::Preprocessing::Aggregator, false);
		//cplex.setParam(IloCplex::Param::Preprocessing::Presolve, false);
		//cplex.setParam(IloCplex::Param::MIP::Strategy::HeuristicFreq, -1);

		double soma_auxt = 0.0;
		vector<double> auxt(particao.size(), 0.0);
		if (k != 1) {
			for (int i = 0; i < particao.size(); i++) {
				auxt[i] = (2.0 - ((double)i / (k - 1)));
				soma_auxt += auxt[i];
			}
		}

		soltime = cplex.getCplexTime();

		int contador = 0;

		for (auto &par : particao) {
			
			//restrições de integralidade
			cout << endl;
			for (auto &par_i : par) {
				eh_inteira[par_i.i][par_i.l][par_i.s] = true;
				contador_variaveis_trans++;
			}

			for (i = 0; i < N; i++) {
				for (l = 0; l < M; l++) {
					if (l_produz_i[l][i]) {
						for (s = 0; s < W; s++) {
							if (eh_inteira[i][l][s] && !ja_add[i][l][s]) {
								modelo.add(IloConversion(env, x[i][l][s], ILOBOOL));
								contador++;
							}
						}
					}
				}
			}


			tempo_incumbent = soltime;
			/*
			if (k != 1) {
				cplex.setParam(IloCplex::TiLim, BUDGET * auxt[contador_particoes] / soma_auxt);
			}
			else {
				cplex.setParam(IloCplex::TiLim, BUDGET);
			}*/
			
			cplex = IloCplex(modelo);
			cplex.setParam(IloCplex::TiLim, (double)BUDGET/k);
			cplex.setParam(IloCplex::Threads, 1);
			cplex.setParam(IloCplex::Param::MIP::Display, 0);
			//cplex.setParam(IloCplex::Param::Preprocessing::Presolve, false);
			cplex.solve();

			auto end = std::chrono::steady_clock::now();
			elapsed_seconds = end - start;


			contador_particoes++;
			if (contador_particoes == k)
				break;

			//fixar variáveis
			IloConstraintArray restricoes(env);
			for (i = 0; i < N; i++) {
				for (l = 0; l < M; l++) {
					if (l_produz_i[l][i]) {
						for (s = 0; s < W; s++) {
							if (eh_inteira[i][l][s] && !ja_add[i][l][s]) {
								IloNum val = round(cplex.getValue(x[i][l][s]));
								restricoes.add(IloConstraint(val == x[i][l][s]));
								ja_add[i][l][s] = true;
							}
						}
					}
				}
			}

			modelo.add(restricoes);
			restricoes.end();
		}
		cout << "Numero de variaveis inteiras = " << contador_variaveis_trans << "!!!!!!!!!!\n\n\n";


		soltime = cplex.getCplexTime() - soltime;

		ofstream resultados(saida, fstream::app);
		resultados << instancia << "," << cplex.getObjValue() << "," << elapsed_seconds.count() << "," << estrategia << "," << k << "," << cplex.getNnodes() << endl;
		resultados.close();
	}
	catch (IloException& e) {
		cplex.error() << "Erro: " << e.getMessage() << endl;
		cout << "\nErro na inteira" << endl;
		ofstream resultados(saida, fstream::app);
		resultados << instancia << "," << cplex.getStatus() << "," << elapsed_seconds.count() << "," << estrategia << "," << k << ",it" << contador_particoes << endl;
		resultados.close();
		return;
	}
	catch (...) {
		cerr << "Outra excecao" << endl;
		return;
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

	char strnum[30];
	q = IloArray<IloArray<IloFloatVarArray>>(env, N);
	x = IloArray<IloArray<IloFloatVarArray>>(env, N);
	for (i = 0; i < N; i++) {
		x[i] = IloArray<IloFloatVarArray>(env, M);
		q[i] = IloArray<IloFloatVarArray>(env, M);
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {
				x[i][l] = IloFloatVarArray(env, W, 0.0, 1.0);
				q[i][l] = IloFloatVarArray(env, W, 0.0, IloInfinity);
				/*for (s = 0; s < W; s++) {
					sprintf_s(strnum, "x(%d,%d,%d)", i, l, s);
					x[i][l][s].setName(strnum);
					sprintf_s(strnum, "q(%d,%d,%d)", i, l, s);
					q[i][l][s].setName(strnum);
				}*/
			}
		}
	}


	y = IloArray<IloArray<IloArray<IloFloatVarArray>>>(env, N);

	for (i = 0; i < N; i++) {
		y[i] = IloArray<IloArray<IloFloatVarArray>>(env, N);
		for (j = 0; j < N; j++) {
			y[i][j] = IloArray<IloFloatVarArray>(env, M);
			for (l = 0; l < M; l++) {
				if (l_produz_i[l][i] && l_produz_i[l][j]) {
					y[i][j][l] = IloFloatVarArray(env, W, 0.0, 1.0);
					/*for (s = 0; s < W; s++)
					{
						sprintf_s(strnum, "y(%d,%d,%d,%d)", i, j, l, s);
						y[i][j][l][s].setName(strnum);
					}*/
				}
			}
		}
	}

	I_plus = IloArray<IloFloatVarArray>(env, N);
	I_minus = IloArray<IloFloatVarArray>(env, N);

	for (i = 0; i < N; i++) {
		I_plus[i] = IloFloatVarArray(env, T, 0.0, IloInfinity);
		I_minus[i] = IloFloatVarArray(env, T, 0.0, IloInfinity);
		/*for ( t = 0; t < T; t++){
			sprintf_s(strnum, "I_plus(%d,%d)", i, t);
			I_plus[i][t].setName(strnum);
			sprintf_s(strnum, "I_minus(%d,%d)", i, t);
			I_minus[i][t].setName(strnum);
		}*/
	}
}

void Modelo::fo() {
	IloInt i, j, t, s, l;		//índices das variáveis
	OBJETIVO = IloExpr(env);	//Criar expressão auxilar para calcular os custos do objetivo separadamente


	C_setup = IloNumVar(env, 0.0, IloInfinity);	//referente a custo de setup, com env sendo ambiente do cplex, e bounds de 0.0 a Infinito
	C_setup.setName("Csetup");					//Dando nome para variável
	C_prod = IloNumVar(env, 0.0, IloInfinity);	//tal como C_setup
	C_prod.setName("Cprod");
	C_est = IloNumVar(env, 0.0, IloInfinity);	//tal como C_setup
	C_est.setName("Cest");
	C_Bko = IloNumVar(env, 0.0, IloInfinity);	//tal como C_setup
	C_Bko.setName("C_Bko");

	/*ATENÇÃO
	Como existe um tempo 0 a variável t vai de 0 a T
	Incrimentei T = T + 1 no momento de criar o modelo para facilidade a escrita do código
	Então T no código é diferente do número de períodos na instância, ou seja, se T = 16 na instância, T = 16 + 1 = 17, neste código
	O restante das variáveis são todas zero based com exceção destas referentes a períodos e subperíodos, que forem modificadas para incluir o período/subperíodo 0
	*/
	//calculando custo de estoque
	for (i = 0; i < N; i++) {
		for (t = 1; t < T; t++) {	//não inclui período 0
			OBJETIVO += h[i] * I_plus[i][t];
		}
	}
	modelo.add(OBJETIVO == C_est);
	OBJETIVO.clear(); //limpar o que tinha na expressão

	//calculando custo de backorder
	for (i = 0; i < N; i++) {
		for (t = 1; t < T; t++) {	//não inclui período 0
			OBJETIVO += g[i] * I_minus[i][t];
		}
	}
	modelo.add(OBJETIVO == C_Bko);
	OBJETIVO.clear();

	//calculando custo de setup
	for (l = 0; l < M; l++) {
		for (i = 0; i < N; i++) {
			for (j = 0; j < N; j++) {
				if (l_produz_i[l][i] && l_produz_i[l][j]) { //somente se l produz i e j, caso contrário vai dar erro pq a variável não existe
					for (s = 1; s < W; s++) {	//não inclui subperíodo 0
						OBJETIVO += cs[i][j][l] * y[i][j][l][s];
					}
				}

			}
		}
	}
	modelo.add(OBJETIVO == C_setup);
	OBJETIVO.clear();


	//calculando custo de produção
	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {	//somente se l produz i, caso contrário vai dar erro pq a variável não existe
				for (s = 1; s < W; s++) {	//não inclui subperíodo 0
					OBJETIVO += cp[i][l] * q[i][l][s];
				}
			}
		}
	}
	modelo.add(OBJETIVO == C_prod);
	OBJETIVO.clear();

	//função objetivo propriamente diata, minimizar todos os custos
	modelo.add(IloMinimize(env, (C_setup + C_prod + C_est + C_Bko))).setName("FO");
}

void Modelo::restricoes() {
	IloInt i, j, l, s, t;
	IloExpr soma(env);



	//(2)
	for (i = 0; i < N; i++)
	{
		for (t = 1; t < T; t++) {
			for (l = 0; l < M; l++) {
				if (l_produz_i[l][i]) {
					for (s = (t - 1) * W_p + 1; s <= t * W_p; s++) {
						soma += q[i][l][s];
					}
				}
			}
			modelo.add(I_plus[i][t - 1] - I_minus[i][t - 1] + soma - I_plus[i][t] + I_minus[i][t] == d[i][t]).setName("(02)");
			soma.clear();
			nconstraints++;
		}
	}

	//(3)
	for (t = 1; t < T; t++) {
		for (i = 0; i < N; i++) {
			soma += I_plus[i][t];
		}
		modelo.add(soma <= CA).setName("(03)");
		soma.clear();
		nconstraints++;
	}

	//(4)
	for (l = 0; l < M; l++) {
		for (t = 1; t < T; t++) {

			for (auto i : SP[l]) {
				for (s = (t - 1) * W_p + 1; s <= t * W_p; s++) {

					soma += p[i][l] * q[i][l][s];

					for (auto j : SP[l]) {
						soma += st[i][j][l] * y[i][j][l][s];

					}
				}
			}

			modelo.add(soma <= CP[l][t]).setName("(04)");
			soma.clear();
			nconstraints++;
		}
	}

	//(5)
	for (l = 0; l < M; l++) {
		for (t = 1; t < T; t++) {
			for (s = (t - 1) * W_p + 1; s <= t * W_p; s++) {
				for (auto i : SP[l]) {
					modelo.add(p[i][l] * q[i][l][s] <= CP[l][t] * x[i][l][s]).setName("(05)");
					nconstraints++;
					//(6)
					modelo.add(q[i][l][s] >= lm[i][l] * (x[i][l][s] - x[i][l][s - 1])).setName("(06)");
					nconstraints++;
				}
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
			nconstraints++;

		}
	}

	//(8)
	for (l = 0; l < M; l++) {
		for (s = 1; s < W; s++) {
			for (auto i : SP[l]) {
				for (auto j : SP[l]) {
					modelo.add(y[i][j][l][s] >= x[i][l][s - 1] + x[j][l][s] - 1).setName("(08)");
					nconstraints++;
				}
			}
		}
	}



	//(9)

	for (l = 0; l < M; l++) {
		for (auto i : SP[l]) {
			nconstraints++;
			modelo.add(x[i][l][0] == 0).setName("(09)");
		}
	}

	for (i = 0; i < N; i++) {
		modelo.add(I_minus[i][0] == I0_minus[i]).setName("(10)");
		nconstraints++;
		modelo.add(I_plus[i][0] == I0_plus[i]).setName("(10)");
		nconstraints++;
	}
}

bool Modelo::teste_de_viabilidade()
{
	IloInt i, j, l, s, t;
	double soma = 0.0;

	//(2)
	for (i = 0; i < N; i++)
	{
		for (t = 1; t < T; t++) {
			for (l = 0; l < M; l++) {
				if (l_produz_i[l][i]) {
					for (s = (t - 1) * W_p + 1; s <= t * W_p; s++) {
						soma += cplex.getValue(q[i][l][s]);
					}
				}
			}
			if (cplex.getValue(I_plus[i][t - 1]) - cplex.getValue(I_minus[i][t - 1])
				+ soma - cplex.getValue(I_plus[i][t]) + cplex.getValue(I_minus[i][t]) > d[i][t] + episilon
				|| cplex.getValue(I_plus[i][t - 1]) - cplex.getValue(I_minus[i][t - 1])
				+ soma - cplex.getValue(I_plus[i][t]) + cplex.getValue(I_minus[i][t]) < d[i][t] - episilon) {
				return false;
			}
			soma = 0.0;
		}
	}

	//(3)
	for (t = 0; t < T; t++) {
		for (i = 0; i < N; i++) {
			soma += cplex.getValue(I_plus[i][t]);
		}
		if (soma > CA)
			return false;
		soma = 0.0;
	}

	//(4)
	for (l = 0; l < M; l++) {
		for (t = 1; t < T; t++) {

			for (auto i : SP[l]) {
				for (s = (t - 1) * W_p + 1; s <= t * W_p; s++) {
					soma += p[i][l] * cplex.getValue(q[i][l][s]);
				}
			}

			for (auto i : SP[l]) {
				for (auto j : SP[l]) {
					for (s = (t - 1) * W_p + 1; s <= t * W_p; s++) {
						soma += st[i][j][l] * cplex.getValue(y[i][j][l][s]);
					}
				}
			}


			if (soma > CP[l][t] + episilon)
				return false;
			soma = 0.0;
		}
	}

	//(5)
	for (l = 0; l < M; l++) {
		for (auto i : SP[l]) {
			for (t = 1; t < T; t++)
			{
				for (s = (t - 1) * W_p + 1; s <= t * W_p; s++) {
					if (p[i][l] * cplex.getValue(q[i][l][s]) > CP[l][t] * cplex.getValue(x[i][l][s]) + episilon)
						return false;
				}
			}
		}

	}

	//(6)
	for (l = 0; l < M; l++) {
		for (auto i : SP[l]) {
			for (s = 1; s < W; s++) {
				cout << l << "," << i << "," << s << endl;
				if (cplex.getValue(q[i][l][s]) + episilon < lm[i][l] * (cplex.getValue(x[i][l][s]) -
					cplex.getValue(x[i][l][s - 1]))) {
					return false;
				}
			}
		}
	}

	//(7)
	for (l = 0; l < M; l++) {
		for (s = 1; s < W; s++) {

			for (auto i : SP[l]) {
				soma += cplex.getValue(x[i][l][s]);
			}

			if (soma < 1 - episilon || soma > 1 + episilon)
				return false;

			soma = 0.0;

		}
	}

	//(8)
	for (l = 0; l < M; l++) {
		for (auto i : SP[l]) {
			for (auto j : SP[l]) {
				for (s = 1; s < W; s++) {
					if (cplex.getValue(y[i][j][l][s]) + episilon < cplex.getValue(x[i][l][s - 1])
						+ cplex.getValue(x[j][l][s]) - 1)
						return false;
				}
			}
		}
	}

	//(9)

	for (l = 0; l < M; l++) {
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i] != 0) {
				if (cplex.getValue(x[i][l][0]) < -episilon || cplex.getValue(x[i][l][0]) > episilon)
					return false;
			}
		}
	}

	for (i = 0; i < N; i++) {
		if (cplex.getValue(I_minus[i][0]) != I0_minus[i])
			return false;
		if (cplex.getValue(I_plus[i][0]) != I0_plus[i])
			return false;
	}
	return true;
}


void Modelo::RELAX_AND_FIX_Estrat1(int k) {
	k = N;

	IloInt i, l, s, t;

	vector<variavel> particoes_completas;
	vector<vector<vector<double>>> dist(N);

	for (i = 0; i < N; i++) {
		dist[i] = vector<vector<double>>(M);
		for (l = 0; l < M; l++) {
			dist[i][l] = vector<double>(W, 0.0);
			if (l_produz_i[l][i]) {
				for (s = 1; s < W; s++) {
					particoes_completas.push_back(variavel(i, l, s, s / W_p));
				}
			}
		}
	}

	try {
		criar_modelo();
		cplex = IloCplex(modelo);
		cplex.setParam(IloCplex::TiLim, 3600 / k);

		IloNum soltime;
		soltime = cplex.getCplexTime();

		//resolver linear
		cplex.setParam(IloCplex::Param::MIP::Display, 0);
		cplex.solve();

		for (i = 0; i < N; i++) {
			for (l = 0; l < M; l++) {
				if (l_produz_i[l][i]) {
					for (s = 1; s < W; s++) {
						dist[i][l][s] = min(cplex.getValue(x[i][l][s]), 1.0 - cplex.getValue(x[i][l][s]));
					}
				}
			}
		}
		//ordenar partições por distância
		std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return dist[i.i][i.l][i.s] < dist[j.i][j.l][j.s];});
		int nvar = ceil((double)particoes_completas.size() / k);


		while (true) {

			vector<variavel> aux;
			//restrições de integralidade
			for (int it = 0; it < min(nvar, (int)particoes_completas.size()); it++)
			{
				auto par = particoes_completas.back();
				modelo.add(IloConversion(env, x[par.i][par.l][par.s], ILOBOOL));
				aux.push_back(par);
				particoes_completas.pop_back();
			}

			tempo_incumbent = soltime;
			cplex.use(mycallback(env, tempo_incumbent, soltime));
			cplex.solve();
			if (particoes_completas.empty())
				break;

			for (i = 0; i < N; i++) {
				for (l = 0; l < M; l++) {
					if (l_produz_i[l][i]) {
						for (s = 1; s < W; s++) {
							dist[i][l][s] = min(cplex.getValue(x[i][l][s]), 1 - cplex.getValue(x[i][l][s]));
						}
					}
				}
			}

			std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return dist[i.i][i.l][i.s] > dist[j.i][j.l][j.s];});


			//fixar variáveis
			IloConstraintArray restricoes(env);
			for (auto par_i : aux) {
				IloNum val = cplex.getValue(x[par_i.i][par_i.l][par_i.s]);
				restricoes.add(IloConstraint(val == x[par_i.i][par_i.l][par_i.s]));
			}

			modelo.add(restricoes);
		}


		soltime = cplex.getCplexTime() - soltime;

		ofstream resultados("resultados.csv", fstream::app);
		resultados << instancia << "," << cplex.getBestObjValue() << "," << cplex.getObjValue() << "," << cplex.getMIPRelativeGap() <<
			"," << tempo_incumbent << "," << soltime << "," << cplex.getNnodes() << endl;
		resultados.close();
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

list<list<variavel>> Modelo::RF_S1(list<vector<variavel>> particoes_completas) {
	list<list<variavel>> particoes;
	int k = (T - 1);

	int N_iten_por_part = (T - 1) / k;

	IloInt i, l, s, t;

	for (auto part : particoes_completas) {
		int n_por_particao = ceil((double)part.size() / k);

		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.s < j.s;});
		list<variavel> var_list;

		int ii_anterior = part.front().t,
			ii = part.front().t,
			cont = 0,
			proxima = 1;

		for (auto var : part) {
			ii = var.t;
			cont++;
			if (ii != ii_anterior)
				proxima++;


			if (proxima > N_iten_por_part || cont == part.size()) { //tá adaptado para K = N, senão tem q mudar
				particoes.push_back(var_list);
				var_list.clear();
				proxima = 1;
			}

			var_list.push_back(var);

			ii_anterior = ii;
		}
	}

	return particoes;
}


list<list<variavel>> Modelo::RF_S2(list<vector<variavel>> particoes_completas) {
	list<list<variavel>> particoes;
	int k = (T - 1);
	int N_iten_por_part = (T - 1) / k;
	IloInt i, l, s, t;

	for (auto part : particoes_completas) {
		int n_por_particao = ceil((double)part.size() / k);
		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.t < j.t;});
		vector<int> demanda_periodo(T, 0);
		for (t = 0; t < T; t++) {
			for (i = 0; i < N; i++) {
				demanda_periodo[t] += d[i][t];
			}
		}



		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return demanda_periodo[i.t] > demanda_periodo[j.t];});
		list<variavel> var_list;

		int ii_anterior = part.front().t,
			ii = part.front().t,
			cont = 0,
			proxima = 1;

		for (auto var : part) {
			ii = var.t;
			cont++;
			if (ii != ii_anterior)
				proxima++;


			if (proxima > N_iten_por_part || cont == part.size()) { //tá adaptado para K = N, senão tem q mudar
				particoes.push_back(var_list);
				var_list.clear();
				proxima = 1;
			}

			var_list.push_back(var);

			ii_anterior = ii;
		}
	}

	return particoes;
}


list<list<variavel>> Modelo::RF_S3(list<vector<variavel>> particoes_completas) {
	list<list<variavel>> particoes;
	int k = N;
	int N_iten_por_part = N / k;

	IloInt i, l, s, t;

	for (auto part : particoes_completas) {

		vector<int> demanda(N, 0);
		for (i = 0; i < N; i++) {
			for (t = 1; t < T; t++) {
				demanda[i] += d[i][t];
			}
		}

		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.i < j.i;});

		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return demanda[i.i] > demanda[j.i];});


		list<variavel> var_list;

		int ii_anterior = part.front().i,
			ii = part.front().i,
			cont = 0,
			proxima = 1;

		for (auto var : part) {
			ii = var.i;
			cont++;
			if (ii != ii_anterior)
				proxima++;


			if (proxima > N_iten_por_part || cont == part.size()) { //tá adaptado para K = N, senão tem q mudar
				particoes.push_back(var_list);
				var_list.clear();
				proxima = 1;
			}

			var_list.push_back(var);

			ii_anterior = ii;
		}
	}

	return particoes;
}


list<list<variavel>> Modelo::RF_S4(list<vector<variavel>> particoes_completas, int K) {
	list<list<variavel>> particoes;

	IloInt i, l, s, t;


	for (auto& part : particoes_completas) {

		int n_var_part = ceil((double)part.size() / K);

		vector<int> demanda(N, 0);
		for (i = 0; i < N; i++) {
			for (t = 1; t < T; t++) {
				demanda[i] += d[i][t];
			}
		}

		for (auto& var : part) {
			var.influ += cp[var.i][var.l];
			for (int j = 0; j < N; j++)
			{
				if (l_produz_i[var.l][j]) {
					var.influ += cs[var.i][j][var.l];
				}
			}
		}
		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.i < j.i;});
		//influcencia S10
		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.influ > j.influ;});
		//distancia S9
		//std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.dist > j.dist;});
		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return demanda[i.i] < demanda[j.i];});


		list<variavel> var_list;

		int cont = 0;

		for (auto& var : part) {
			cont++;
			var_list.push_back(var);

			if (cont % n_var_part == 0 || cont == part.size()) { //tá adaptado para K = N, senão tem q mudar
				particoes.push_back(var_list);
				var_list.clear();
			}
		}
	}

	return particoes;
}

list<list<variavel>> Modelo::RF_S5(list<vector<variavel>> particoes_completas, int K) {
	list<list<variavel>> particoes;
	int k = N;
	int N_iten_por_part = N / k;

	IloInt i, l, s, t;

	for (auto part : particoes_completas) {
		int n_var_part = ceil((double)part.size() / K);

		vector<int> flexibilidade(N, 0);

		for (auto& var : part) {
			var.influ += cp[var.i][var.l];
			for (int j = 0; j < N; j++)
			{
				if (l_produz_i[var.l][j]) {
					var.influ += cs[var.i][j][var.l];
				}
			}
		}

		for (i = 0; i < N; i++) {
			for (l = 0; l < M; l++)
			{
				if (l_produz_i[l][i])
				{
					flexibilidade[i]++;
				}

			}
		}

		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.i < j.i;});
		//influcencia S10
		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.influ > j.influ;});
		//distancia S9
		//std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.dist > j.dist;});

		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return flexibilidade[i.i] > flexibilidade[j.i];});


		list<variavel> var_list;

		int cont = 0;

		for (auto& var : part) {
			cont++;
			var_list.push_back(var);

			if (cont % n_var_part == 0 || cont == part.size()) { //tá adaptado para K = N, senão tem q mudar
				particoes.push_back(var_list);
				var_list.clear();
			}
		}
	}

	return particoes;
}


list<list<variavel>> Modelo::RF_S6(list<vector<variavel>> particoes_completas) {
	list<list<variavel>> particoes;
	int k = M;
	int N_iten_por_part = M / k;

	IloInt i, l, s, t;

	for (auto part : particoes_completas) {

		vector<double> eficiencia(M, 0);
		for (l = 0; l < M; l++) {
			for (i = 0; i < N; i++) {
				if (l_produz_i[l][i]) {
					eficiencia[l] += p[i][l] + cp[i][l];
				}
			}
			eficiencia[l] /= (double)CP[l].size(); //cp tem a mesma cardinalidade de \cal I _ \ell por isso que usei ele
		}

		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return eficiencia[i.l] < eficiencia[j.l];});


		list<variavel> var_list;

		int ii_anterior = part.front().l,
			ii = part.front().l,
			cont = 0,
			proxima = 1;

		for (auto var : part) {
			ii = var.l;
			cont++;
			if (ii != ii_anterior)
				proxima++;


			if (proxima > N_iten_por_part || cont == part.size()) { //tá adaptado para K = N, senão tem q mudar
				particoes.push_back(var_list);
				var_list.clear();
				proxima = 1;
			}

			var_list.push_back(var);

			ii_anterior = ii;
		}
	}

	return particoes;
}


list<list<variavel>> Modelo::RF_S7(list<vector<variavel>> particoes_completas) {
	list<list<variavel>> particoes;
	int k = M;
	int N_iten_por_part = M / k;

	IloInt i, l, s, t;

	for (auto part : particoes_completas) {

		vector<double> eficiencia(M, 0);
		for (l = 0; l < M; l++) {
			for (i = 0; i < N; i++) {
				if (l_produz_i[l][i]) {
					eficiencia[l] += p[i][l] + cp[i][l];
				}
			}
			eficiencia[l] /= (double)CP[l].size(); //cp tem a mesma cardinalidade de \cal I _ i por isso que usei ele
		}

		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return eficiencia[i.l] > eficiencia[j.l];});


		list<variavel> var_list;

		int ii_anterior = part.front().l,
			ii = part.front().l,
			cont = 0,
			proxima = 1;

		for (auto var : part) {
			ii = var.l;
			cont++;
			if (ii != ii_anterior)
				proxima++;


			if (proxima > N_iten_por_part || cont == part.size()) { //tá adaptado para K = N, senão tem q mudar
				particoes.push_back(var_list);
				var_list.clear();
				proxima = 1;
			}

			var_list.push_back(var);

			ii_anterior = ii;
		}
	}

	return particoes;
}


list<list<variavel>> Modelo::RF_S8(list<vector<variavel>> particoes_completas) {
	list<list<variavel>> particoes;
	int k = M;
	int N_iten_por_part = M / k;

	IloInt i, l, s, t;

	for (auto part : particoes_completas) {

		vector<int> criticidade(M, 0);
		for (l = 0; l < M; l++) {
			int min = INT_MAX;

			for (i = 0; i < N; i++) {
				if (l_produz_i[l][i] && SP[i].size() < min) {
					min = SP[i].size();
				}
			}
			criticidade[l] = M - min;
		}

		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});


		list<variavel> var_list;

		int ii_anterior = part.front().l,
			ii = part.front().l,
			cont = 0,
			proxima = 1;

		for (auto var : part) {
			ii = var.l;
			cont++;
			if (ii != ii_anterior)
				proxima++;


			if (proxima > N_iten_por_part || cont == part.size()) { //tá adaptado para K = N, senão tem q mudar
				particoes.push_back(var_list);
				var_list.clear();
				proxima = 1;
			}

			var_list.push_back(var);

			ii_anterior = ii;
		}
	}

	return particoes;
}


list<list<variavel>> Modelo::RF_S10(list<vector<variavel>> particoes_completas, int k) {
	list<list<variavel>> particoes;


	IloInt i, l, s, t;

	for (auto part : particoes_completas) {
		int N_iten_por_part = ceil((double)part.size() / k);


		for (auto& var : part) {
			var.influ += cp[var.i][var.l];
			for (int j = 0; j < N; j++)
			{
				if (l_produz_i[var.l][j]) {
					var.influ += cs[var.i][j][var.l];
				}
			}
		}

		std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.influ > j.influ;});


		list<variavel> var_list;

		int cont = 0;


		for (auto var : part) {
			cont++;
			var_list.push_back(var);

			if (((cont % N_iten_por_part) == 0) || (cont == part.size())) {
				particoes.push_back(var_list);
				var_list.clear();

			}
		}
	}

	return particoes;
}





void Modelo::RF_K_HIBRIDO(int estrategia, const char* saida, int k, bool _fix_opt) {
	list<list<variavel>> particao_pt1, particao_pt2;
	IloInt i, l, s, t;
	list<vector<variavel>> particoes_completas;

	vector<variavel> var_list;
	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {
				t = 1;
				for (s = 1; s < W; s++) {
					var_list.push_back(variavel(i, l, s, t));
					if (s % W_p == 0)
						t++;
				}
			}
		}
	}
	particoes_completas.push_back(var_list);

	//calcular relaxação linear e distâncias da integralidade
	{
		criar_modelo();
		cplex = IloCplex(modelo);

		//resolver linear
		cplex.setParam(IloCplex::Param::MIP::Display, 0);
		cplex.solve();


		for (auto& par : particoes_completas)
		{
			for (auto& var : par) {
				var.dist = min(cplex.getValue(x[var.i][var.l][var.s]), 1.0 - cplex.getValue(x[var.i][var.l][var.s]));
			}
		}
	}


	switch (estrategia) {
	case 4:
		particao_pt1 = RF_S4(particoes_completas, k);
		if (k == 1)
			exit(-1);
		particao_pt2 = RF_S4(particoes_completas, (k - 1));
		break;
	case 10:
		particao_pt1 = RF_S10(particoes_completas, k);
		particao_pt2 = RF_S10(particoes_completas, k - 1);
		break;
	default:
		cerr << "Erro: Nenhuma estrategia escolhdida!" << endl;
		exit(0);
	}

	IloNum soltime;
	try {
		criar_modelo();
		cplex = IloCplex(modelo);
		cplex.setParam(IloCplex::TiLim, 3600 / particao_pt1.size());

		soltime = cplex.getCplexTime();

		int contador_particoes = 0,
			contador_variaveis_trans = 0;
		for (auto& par : particao_pt1) {
			//restrições de integralidade
			for (auto& par_i : par) {
				modelo.add(IloConversion(env, x[par_i.i][par_i.l][par_i.s], ILOBOOL));
				contador_variaveis_trans++;
			}

			tempo_incumbent = soltime;
			//cplex.use(mycallback(env, tempo_incumbent, soltime));
			cplex.setParam(IloCplex::Param::MIP::Display, 0);
			//cplex = IloCplex(modelo);
			cplex.solve();

			contador_particoes++;
			if (contador_particoes == particao_pt1.size() || _fix_opt)
				break;

			//fixar variáveis
			IloConstraintArray restricoes(env);
			for (auto par_i : par) {
				IloNum val = cplex.getValue(x[par_i.i][par_i.l][par_i.s]);
				restricoes.add(IloConstraint(val == x[par_i.i][par_i.l][par_i.s]));
			}

			modelo.add(restricoes);
		}

		soltime = cplex.getCplexTime() - soltime;

		ofstream resultados(saida, fstream::app);
		resultados << instancia << "," << cplex.getObjValue() << "," << soltime << "," << estrategia << "," << k << endl;
		resultados.close();
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

	IloNum soltime0 = soltime;
	try {
		IloNumVarArray startVar(env);
		IloNumArray startVal(env);
		for (auto& par : particoes_completas.front()) {
			startVar.add(x[par.i][par.l][par.s]);
			startVal.add(cplex.getValue(x[par.i][par.l][par.s]));
		}



		//startVal.end();
		//startVar.end();

		//reiniciar restrições do modelo, inclue as de integralidade
		modelo.end();
		modelo = IloModel(env);
		fo();
		restricoes();
		cplex = IloCplex(modelo);

		//----------------------------------------------------------


		//cplex.setParam(IloCplex::Param::MIP::Display, 1);
		cplex.setParam(IloCplex::TiLim, (3600.0 - soltime) / particao_pt2.size());

		soltime = cplex.getCplexTime();
		int contador_particoes = 0;
		for (auto& par : particao_pt2) {
			//restrições de integralidade
			for (auto& par_i : par) {
				modelo.add(IloConversion(env, x[par_i.i][par_i.l][par_i.s], ILOBOOL));
			}

			tempo_incumbent = soltime;
			cplex.addMIPStart(startVar, startVal);
			cplex.solve();
			contador_particoes++;
			if (contador_particoes == particao_pt2.size() || _fix_opt)
				break;

			//fixar variáveis
			IloConstraintArray restricoes(env);
			for (auto par_i : par) {
				IloNum val = cplex.getValue(x[par_i.i][par_i.l][par_i.s]);
				restricoes.add(IloConstraint(val == x[par_i.i][par_i.l][par_i.s]));
			}

			modelo.add(restricoes);
		}

		soltime = cplex.getCplexTime() - soltime;

		ofstream resultados(saida, fstream::app);
		resultados << instancia << "," << cplex.getObjValue() << "," << soltime + soltime0 << "," << estrategia << "," << k - 1 << endl;
		resultados.close();
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
