#include "RF_RUIM.h"

vector<vector<variavel>> RF_RUIM::RF_S4(vector<variavel> particoes_completas, int K) {
	vector<vector<variavel>> particoes;

	IloInt i, l, s, t;



	int n_var_part = ceil((double)particoes_completas.size() / K);

	vector<int> demanda(N, 0);
	for (i = 0; i < N; i++) {
		for (t = 1; t < T; t++) {
			demanda[i] += d[i][t];
		}
	}

	for (auto& var : particoes_completas) {
		var.influ += cp[var.i][var.l];
		for (int j = 0; j < N; j++)
		{
			if (l_produz_i[var.l][j]) {
				var.influ += cs[var.i][j][var.l];
			}
		}
	}
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	//influcencia S10
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.influ > j.influ;});
	//distancia S9
	//std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.dist > j.dist;});
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return demanda[i.i] < demanda[j.i];});


	vector<variavel> var_list;

	int cont = 0;

	for (auto& var : particoes_completas) {
		cont++;
		var_list.push_back(var);

		if (cont % n_var_part == 0 || cont == particoes_completas.size()) { //t� adaptado para K = N, sen�o tem q mudar
			particoes.push_back(var_list);
			var_list.clear();
		}
	}

	return particoes;
}


vector<vector<variavel>> RF_RUIM::RF_S8(vector<variavel> particoes_completas) {
	vector<vector<variavel>> particoes;
	int k = M;
	int N_iten_por_part = M / k;

	IloInt i, l, s, t;


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

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});


	vector<variavel> var_list;

	int ii_anterior = particoes_completas.front().l,
		ii = particoes_completas.front().l,
		cont = 0,
		proxima = 1;

	for (auto var : particoes_completas) {
		ii = var.l;
		cont++;
		if (ii != ii_anterior)
			proxima++;


		if (proxima > N_iten_por_part || cont == particoes_completas.size()) { //t� adaptado para K = N, sen�o tem q mudar
			particoes.push_back(var_list);
			var_list.clear();
			proxima = 1;
		}

		var_list.push_back(var);

		ii_anterior = ii;
	}

	return particoes;
}


void RF_RUIM::RELAX_AND_FIX(int estrategia, const char* saida, int K, double BUDGET)
{

	IloInt i, j, l, t, s;
	vector<variavel> particoes_completas;

	vector<vector<variavel>> particao;

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
					particoes_completas.push_back(variavel(i, l, s, t));
					if (s % W_p == 0)
						t++;

				}
			}
		}
	}

	switch (estrategia) {

	case 4:
		particao = RF_S4(particoes_completas, K);
		break;

	case 8:
		particao = RF_S8(particoes_completas);
		break;

	default:
		cerr << "Erro: Nenhuma estrategia escolhdida!" << endl;
		exit(0);
	}

	
	vector<vector<vector<int>>> x_bar(N);
	for (i = 0; i < N; i++) {
		x_bar[i] = vector<vector<int>>(M);
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {
				x_bar[i][l] = vector<int>(W);
			}
		}
	}
	

	double soma_auxt = 0.0;
	vector<double> auxt(particao.size(), 0.0);
	if (K != 1) {
		for (int i = 0; i < particao.size(); i++) {
			auxt[i] = (2.0 - ((double)i / (K - 1)));
			soma_auxt += auxt[i];
		}
	}
	auto start = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds;

	for (int k = 0; k < K; k++)
	{

		IloEnv env;
		IloModel modelo;
		IloCplex cplex;
		IloExpr OBJETIVO;

		IloArray<IloFloatVarArray> I_plus, I_minus;
		IloArray<IloArray<IloFloatVarArray>> q;
		IloArray<IloArray<IloArray<IloNumVar>>> x;
		IloArray<IloArray<IloArray<IloFloatVarArray>>> y;

		//declarar vari�veis
		q = IloArray<IloArray<IloFloatVarArray>>(env, N);
		x = IloArray<IloArray<IloArray<IloNumVar>>>(env, N);
		for (i = 0; i < N; i++) {
			x[i] = IloArray<IloArray<IloNumVar>>(env, M);
			q[i] = IloArray<IloFloatVarArray>(env, M);
			for (l = 0; l < M; l++) {
				if (l_produz_i[l][i]) {
					x[i][l] = IloArray<IloNumVar>(env, W);

					x[i][l][0] = IloNumVar(env, 0.0, 1.0, ILOFLOAT); //tenho q definar as zero q j� s�o zero mesmo

					q[i][l] = IloFloatVarArray(env, W, 0.0, IloInfinity);
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
					}
				}
			}
		}



		int contador = 0;
		for (IloInt k1 = 0; k1 < k; k1++) {
			for (auto& var : particao[k1]) {
				x[var.i][var.l][var.s] = IloNumVar(env, x_bar[var.i][var.l][var.s], x_bar[var.i][var.l][var.s], ILOFLOAT);
				contador++;

			}
		}
		for (auto& var : particao[k]) {
			x[var.i][var.l][var.s] = IloNumVar(env, 0.0, 1.0, ILOINT);
			contador++;
		}

		for (IloInt k1 = k + 1; k1 < K; k1++)
		{
			for (auto& var : particao[k1]) {
				x[var.i][var.l][var.s] = IloNumVar(env, 0.0, 1.0, ILOFLOAT);
				contador++;
			}
		}

		I_plus = IloArray<IloFloatVarArray>(env, N);
		I_minus = IloArray<IloFloatVarArray>(env, N);

		for (i = 0; i < N; i++) {
			I_plus[i] = IloFloatVarArray(env, T, 0.0, IloInfinity);
			I_minus[i] = IloFloatVarArray(env, T, 0.0, IloInfinity);
		}



		modelo = IloModel(env, "modelo_linear");
		//Fun��o objetivo
		OBJETIVO = IloExpr(env);	//Criar express�o auxilar para calcular os custos do objetivo separadamente

		//calculando custo de estoque
		for (i = 0; i < N; i++) {
			for (t = 1; t < T; t++) {	//n�o inclui per�odo 0
				OBJETIVO += h[i] * I_plus[i][t];
			}
		}

		//calculando custo de backorder
		for (i = 0; i < N; i++) {
			for (t = 1; t < T; t++) {	//n�o inclui per�odo 0
				OBJETIVO += g[i] * I_minus[i][t];
			}
		}

		//calculando custo de setup
		for (l = 0; l < M; l++) {
			for (i = 0; i < N; i++) {
				for (j = 0; j < N; j++) {
					if (l_produz_i[l][i] && l_produz_i[l][j]) { //somente se l produz i e j, caso contr�rio vai dar erro pq a vari�vel n�o existe
						for (s = 1; s < W; s++) {	//n�o inclui subper�odo 0
							OBJETIVO += cs[i][j][l] * y[i][j][l][s];
						}
					}

				}
			}
		}



		//calculando custo de produ��o
		for (i = 0; i < N; i++) {
			for (l = 0; l < M; l++) {
				if (l_produz_i[l][i]) {	//somente se l produz i, caso contr�rio vai dar erro pq a vari�vel n�o existe
					for (s = 1; s < W; s++) {	//n�o inclui subper�odo 0
						OBJETIVO += cp[i][l] * q[i][l][s];
					}
				}
			}
		}

		//fun��o objetivo propriamente diata, minimizar todos os custos
		modelo.add(IloMinimize(env, OBJETIVO)).setName("FO");

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
			}
		}

		//(3)
		for (t = 1; t < T; t++) {
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

						for (auto j : SP[l]) {
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
			for (t = 1; t < T; t++) {
				for (s = (t - 1) * W_p + 1; s <= t * W_p; s++) {
					for (auto i : SP[l]) {
						modelo.add(p[i][l] * q[i][l][s] <= CP[l][t] * x[i][l][s]).setName("(05)");
						//(6)
						modelo.add(q[i][l][s] >= lm[i][l] * (x[i][l][s] - x[i][l][s - 1])).setName("(06)");
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

			}
		}

		//(8)
		for (l = 0; l < M; l++) {
			for (s = 1; s < W; s++) {
				for (auto i : SP[l]) {
					for (auto j : SP[l]) {
						modelo.add(y[i][j][l][s] >= x[i][l][s - 1] + x[j][l][s] - 1).setName("(08)");
					}
				}
			}
		}

		//(9)
		for (l = 0; l < M; l++) {
			for (auto i : SP[l]) {
				modelo.add(x[i][l][0] == 0).setName("(09)");
			}
		}

		for (i = 0; i < N; i++) {
			modelo.add(I_minus[i][0] == I0_minus[i]).setName("(10)");
			modelo.add(I_plus[i][0] == I0_plus[i]).setName("(10)");
		}


		//Resolver Modelo
		cplex = IloCplex(modelo);
		cplex.setParam(IloCplex::Threads, 1);
		cplex.setParam(IloCplex::Param::MIP::Display, 0);
		cplex.setParam(IloCplex::TiLim, (double)BUDGET / K);
		//	cplex.setParam(IloCplex::Param::Preprocessing::Presolve, false);

		try {
			cplex.solve();
			auto end = std::chrono::steady_clock::now();
			elapsed_seconds = end - start;

			for (auto& var : particao[k]) {
				if (cplex.isExtracted(x[var.i][var.l][var.s])) {
					x_bar[var.i][var.l][var.s] = round(cplex.getValue(x[var.i][var.l][var.s]));
				}
				else {
					x_bar[var.i][var.l][var.s] = 0.0;
				}
			}

			if (k == K - 1) {
				ofstream resultados(saida, fstream::app);
				resultados << instancia << "," << cplex.getObjValue() << "," << elapsed_seconds.count() << "," << estrategia << "," << K << "," << cplex.getNnodes() << endl;
				resultados.close();
				bool viavel = teste_de_viabilidade(cplex, I_plus, I_minus, q, x, y);
				cout << endl;
			}
		}
		catch (IloException& e) {
			cplex.error() << "Erro: " << e.getMessage() << endl;
			cout << "\nErro na inteira" << endl;

			ofstream resultados(saida, fstream::app);
			resultados << instancia << "," << cplex.getStatus() << "," << cplex.getCplexTime() - elapsed_seconds.count() << "," << estrategia << "," << K << ",it" << k+1 << endl;
			resultados.close();
			return;
		}
		catch (...) {
			cerr << "Outra excecao" << endl;
			return;
		}

		

	}

	
	
	
}

bool RF_RUIM::teste_de_viabilidade(IloCplex cplex, IloArray<IloFloatVarArray> I_plus, IloArray<IloFloatVarArray> I_minus, IloArray<IloArray<IloFloatVarArray>> q,
	IloArray<IloArray<IloArray<IloNumVar>>> x, IloArray<IloArray<IloArray<IloFloatVarArray>>> y)
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

