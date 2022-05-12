#include "RF.h"






void RF::RELAX_AND_FIX(int estrategia, const char* saida, int K, double BUDGET, int modo_divisao)
{

	IloInt i, j, l, t, s;
	vector<variavel> particoes_completas;

	vector<vector<variavel>> particao;

	int ind_geral = 0;
	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {
				t = 1;
				for (s = 1; s < W; s++) {
					particoes_completas.push_back(variavel(i, l, s, t, ind_geral));
					ind_geral++;
					if (s % W_p == 0)
						t++;

				}
			}
		}
	}
	//calculando influências na função objetivo
	for (auto& var : particoes_completas) {
		var.influ += cp[var.i][var.l];
		for (int j = 0; j < N; j++)
		{
			if (l_produz_i[var.l][j]) {
				var.influ += cs[var.i][j][var.l];
			}
		}
	}


	//relaxação linear
	{
		IloEnv env;
		IloModel modelo;
		IloCplex cplex;
		IloExpr OBJETIVO;
		int nvar_reais = 0,
			nconst = 0;
		try {

			IloArray<IloFloatVarArray> I_plus, I_minus;
			IloArray<IloArray<IloFloatVarArray>> q;
			IloArray<IloArray<IloArray<IloNumVar>>> x;
			IloArray<IloArray<IloArray<IloFloatVarArray>>> y;

			//declarar variáveis
			q = IloArray<IloArray<IloFloatVarArray>>(env, N);
			x = IloArray<IloArray<IloArray<IloNumVar>>>(env, N);
			for (i = 0; i < N; i++) {
				x[i] = IloArray<IloArray<IloNumVar>>(env, M);
				q[i] = IloArray<IloFloatVarArray>(env, M);
				for (l = 0; l < M; l++) {
					if (l_produz_i[l][i]) {
						x[i][l] = IloArray<IloNumVar>(env, W);

						x[i][l][0] = IloNumVar(env, 0.0, 1.0, ILOFLOAT); //tenho q definar as zero q já são zero mesmo

						q[i][l] = IloFloatVarArray(env, W, 0.0, IloInfinity);
						nvar_reais += W;
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
							nvar_reais += W;
						}
					}

				}
			}

			for (auto& var :particoes_completas) {
				x[var.i][var.l][var.s] = IloNumVar(env, 0.0, 1.0, ILOFLOAT);
			}

			I_plus = IloArray<IloFloatVarArray>(env, N);
			I_minus = IloArray<IloFloatVarArray>(env, N);

			for (i = 0; i < N; i++) {
				I_plus[i] = IloFloatVarArray(env, T, 0.0, IloInfinity);
				I_minus[i] = IloFloatVarArray(env, T, 0.0, IloInfinity);
				nvar_reais += T * 2;
			}

			modelo = IloModel(env, "modelo_linear");
			//Função objetivo
			OBJETIVO = IloExpr(env);	//Criar expressão auxilar para calcular os custos do objetivo separadamente

			//calculando custo de estoque
			for (i = 0; i < N; i++) {
				for (t = 1; t < T; t++) {	//não inclui período 0
					OBJETIVO += h[i] * I_plus[i][t];
				}
			}

			//calculando custo de backorder
			for (i = 0; i < N; i++) {
				for (t = 1; t < T; t++) {	//não inclui período 0
					OBJETIVO += g[i] * I_minus[i][t];
				}
			}

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

			//função objetivo propriamente diata, minimizar todos os custos
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
					nconst++;
					soma.clear();
				}
			}

			//(3)
			for (t = 1; t < T; t++) {
				for (i = 0; i < N; i++) {
					soma += I_plus[i][t];
				}
				modelo.add(soma <= CA).setName("(03)");
				nconst++;
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
					nconst++;
					soma.clear();
				}
			}

			//(5)
			for (l = 0; l < M; l++) {
				for (t = 1; t < T; t++) {
					for (s = (t - 1) * W_p + 1; s <= t * W_p; s++) {
						for (auto i : SP[l]) {
							modelo.add(p[i][l] * q[i][l][s] <= CP[l][t] * x[i][l][s]).setName("(05)"); nconst++;
							//(6)
							modelo.add(q[i][l][s] >= lm[i][l] * (x[i][l][s] - x[i][l][s - 1])).setName("(06)"); nconst++;
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

					modelo.add(soma == 1).setName("(07)"); nconst++;
					soma.clear();

				}
			}

			//(8)
			for (l = 0; l < M; l++) {
				for (s = 1; s < W; s++) {
					for (auto i : SP[l]) {
						for (auto j : SP[l]) {
							modelo.add(y[i][j][l][s] >= x[i][l][s - 1] + x[j][l][s] - 1).setName("(08)"); nconst++;
						}
					}
				}
			}

			//(fixação de variáveis
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
			cplex.solve();



			ofstream resultados("relax_linear.csv", fstream::app);
			resultados << instancia << "," << cplex.getObjValue() << "," << estrategia << "-" << modo_divisao << "," << K << endl;
			resultados.close();
			
			//calcular valores para S9
			for (auto& var : particoes_completas) {
				var.dist = min(cplex.getValue(x[var.i][var.l][var.s]), 1.0 - cplex.getValue(x[var.i][var.l][var.s]));
			}
			cout << endl;
			env.end();

		}
		catch (IloException& e) {
			cplex.error() << "Erro: " << e.getMessage() << endl;
			cout << "\nErro na Linear" << endl;

			ofstream resultados(saida, fstream::app);
			resultados << "Erro na linear" << endl;
			resultados.close();
			return;
		}
		catch (const exception& e) {
			ofstream resultados(saida, fstream::app);
			resultados << e.what() << endl;
			resultados.close();
		}
		catch (...) {
			ofstream resultados(saida, fstream::app);
			resultados << "Outra excecao" << endl;
			resultados.close();
			return;
		}

		double sum = 0.0;
		for (int i = 0; i < N; i++) {
			sum += I0_plus[i];
		}
		

		ofstream dados_variaves("dados_var.csv", fstream::app);
		dados_variaves << instancia << "," << particoes_completas.size() << "," << nvar_reais << "," << nconst << "," << sum/CA << endl;
		dados_variaves.close();
		//return;



	}

	switch (estrategia) {
	case 1:
		particao = RF_S1(particoes_completas, K);
		break;
	case 2:
		particao = RF_S2(particoes_completas, K);
		break;
	case 3:
		particao = RF_S3(particoes_completas, K);
		break;
	case 4:
		particao = RF_S4(particoes_completas, K);
		break;
	case 5:
		particao = RF_S5(particoes_completas, K);
		break;
	case 6:
		particao = RF_S6(particoes_completas, K);
		break;
	case 7:
		particao = RF_S7(particoes_completas, K);
		break;
	case 8:
		particao = RF_S8(particoes_completas, K);
		break;
	case 9:
		particao = RF_S9(particoes_completas, K);
		break;
	case 10:
		particao = RF_S10(particoes_completas, K);
		break;
	case 11:
		particao = HRF1_S1_S5(particoes_completas, K);
		break;
	case 12:
		particao = HRF1_S1_S8(particoes_completas, K);
		break;
	case 13:
		particao = HRF1_S5_S8(particoes_completas, K);
		break;
	case 14:
		particao = HRF1_S1_S5_S8(particoes_completas, K);
		break;
	case 15:
		particao = HRF3_S1_S8(particoes_completas, K);
		break;
	case 16:
		particao = HRF2_S1_S5(particoes_completas, K);
		break;
	case 17:
		particao = HRF2_S1_S8(particoes_completas, K);
		break;
	case 18:
		particao = HRF2_S5_S8(particoes_completas, K);
		break;
	case 19:
		particao = HRF2_S1_S5_S8(particoes_completas, K);
		break;
	case 20:
		particao = HRF4_S1_S5(particoes_completas, K);
		break;
	case 21:
		particao = HRF4_S1_S8(particoes_completas, K);
		break;
	case 22:
		particao = HRF4_S5_S1(particoes_completas, K);
		break;
	case 23:
		particao = HRF4_S5_S8(particoes_completas, K);
		break;
	case 24:
		particao = HRF4_S8_S1(particoes_completas, K);
		break;
	case 25:
		particao = HRF4_S8_S5(particoes_completas, K);
		break;
	case 111:
		particao = RF_S11(particoes_completas, K);
		break;
	case 112:
		particao = RF_S112(particoes_completas, K);
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


	vector<double> tempos(K, 0.0);
	double soma = 0.0;

	//divisao dos budgets
	switch (modo_divisao)
	{
	case 2:
		if (K == 1) {
			tempos[0] = BUDGET;
			break;
		}
		for (double ii = 0; ii < K; ii++)
		{
			tempos[ii] = (2.0/3.0 * BUDGET / K) * (2.0 - ii / (K - 1.0));
			soma += tempos[ii];
		}
		break;
	case 3:
		if (K == 1) {
			tempos[0] = BUDGET;
			break;
		}
		for (double ii = 0; ii < K; ii++)
		{
			tempos[ii] = (BUDGET / (2.0 * K)) * (3.0 - 2.0 * ii / (K - 1.0));
			soma += tempos[ii];
		}
		break;
	default:
		if (K == 1) {
			tempos[0] = BUDGET;
			break;
		}
		for (double ii = 0; ii < K; ii++)
		{
			tempos[ii] = (BUDGET / K);
			soma += tempos[ii];
		}
		break;
	}

	
	
	std::chrono::duration<double> elapsed_seconds = std::chrono::steady_clock::now() - std::chrono::steady_clock::now();

	for (int k = 0; k < K; k++)
	{
		//para cada k criar um subproblema

		IloEnv env;
		IloModel modelo;
		IloCplex cplex;
		IloNumVar OBJETIVO;
		IloExpr c1, c2, c3, c4;
		try {

			IloArray<IloFloatVarArray> I_plus, I_minus;
			IloArray<IloArray<IloFloatVarArray>> q;
			IloArray<IloArray<IloArray<IloNumVar>>> x;
			IloArray<IloArray<IloArray<IloFloatVarArray>>> y;

			//declarar variáveis
			q = IloArray<IloArray<IloFloatVarArray>>(env, N);
			x = IloArray<IloArray<IloArray<IloNumVar>>>(env, N);
			for (i = 0; i < N; i++) {
				x[i] = IloArray<IloArray<IloNumVar>>(env, M);
				q[i] = IloArray<IloFloatVarArray>(env, M);
				for (l = 0; l < M; l++) {
					if (l_produz_i[l][i]) {
						x[i][l] = IloArray<IloNumVar>(env, W);

						x[i][l][0] = IloNumVar(env, 0.0, 1.0, ILOFLOAT); //tenho q definir as zero q já são zero mesmo

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
			//Função objetivo
			OBJETIVO = IloNumVar(env,0.0, IloInfinity);	//Criar expressão auxilar para calcular os custos do objetivo separadamente
			c1 = IloExpr(env);
			c2 = IloExpr(env);
			c3 = IloExpr(env);
			c4 = IloExpr(env);
			//calculando custo de estoque
			for (i = 0; i < N; i++) {
				for (t = 1; t < T; t++) {	//não inclui período 0
					c1 += h[i] * I_plus[i][t];
				}
			}

			//calculando custo de backorder
			for (i = 0; i < N; i++) {
				for (t = 1; t < T; t++) {	//não inclui período 0
					c2 += g[i] * I_minus[i][t];
				}
			}

			//calculando custo de setup
			for (l = 0; l < M; l++) {
				for (i = 0; i < N; i++) {
					for (j = 0; j < N; j++) {
						if (l_produz_i[l][i] && l_produz_i[l][j]) { //somente se l produz i e j, caso contrário vai dar erro pq a variável não existe
							for (s = 1; s < W; s++) {	//não inclui subperíodo 0
								c3 += cs[i][j][l] * y[i][j][l][s];
							}
						}

					}
				}
			}

			//calculando custo de produção
			for (i = 0; i < N; i++) {
				for (l = 0; l < M; l++) {
					if (l_produz_i[l][i]) {	//somente se l produz i, caso contrário vai dar erro pq a variável não existe
						for (s = 1; s < W; s++) {	//não inclui subperíodo 0
							c4 += cp[i][l] * q[i][l][s];
						}
					}
				}
			}
			modelo.add((c1 + c2 + c3 + c4) == OBJETIVO);
			
			//função objetivo propriamente diata, minimizar todos os custos
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
			//cplex.setParam(IloCplex::Param::MIP::Display, 0);
			//cplex.setParam(IloCplex::TiLim, (double)BUDGET / K);
			

			cplex.setParam(IloCplex::TiLim, tempos[k]);
	

			//	cplex.setParam(IloCplex::Param::Preprocessing::Presolve, false);

			auto start = std::chrono::steady_clock::now();
			if (!cplex.solve()) {
				auto end = std::chrono::steady_clock::now();
				elapsed_seconds += end - start;
			}
			auto end = std::chrono::steady_clock::now();
			elapsed_seconds += end - start;

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
				resultados << instancia << "," << cplex.getObjValue() << "," << elapsed_seconds.count() << "," << estrategia <<"-"<< modo_divisao << "," << K << "," << cplex.getObjective() << endl;
				resultados.close();


				resultados.open("custos.txt", fstream::app);
				resultados << instancia << "," << cplex.getValue(c1) << "," << cplex.getValue(c2) << "," << cplex.getValue(c3) << "," << cplex.getValue(c4) << endl;
				resultados.close();

				bool viavel = teste_de_viabilidade(cplex, I_plus, I_minus, q, x, y);
				cout << (viavel ? "viavel" : "inviavel") << endl;
				break;
			}

			env.end();
			
		}
		catch (IloException& e) {
			cplex.error() << "Erro: " << e.getMessage() << endl;
			cout << "\nErro na inteira" << endl;

			ofstream resultados(saida, fstream::app);
			resultados << instancia << "," << cplex.getStatus() << "," << cplex.getCplexTime() - elapsed_seconds.count() << "," << estrategia << "-" << modo_divisao << "," << K << ",it" << k + 1 << endl;
			resultados.close();
			return;
		}
		catch (const exception& e) {
			ofstream resultados(saida, fstream::app);
			resultados << e.what() << endl;
			resultados.close();
		}
		catch (...) {
			ofstream resultados(saida, fstream::app);
			resultados << "Outra excecao" << endl;
			resultados.close();
			return;
		}



		

	}

	
	
	
}

bool RF::teste_de_viabilidade(IloCplex cplex, IloArray<IloFloatVarArray> I_plus, IloArray<IloFloatVarArray> I_minus, IloArray<IloArray<IloFloatVarArray>> q,
	IloArray<IloArray<IloArray<IloNumVar>>> x, IloArray<IloArray<IloArray<IloFloatVarArray>>> y)
{
	IloInt i, j, l, s, t;
	double soma = 0.0;
	string saida_solu = instancia;
	saida_solu += ".solu";
	ofstream solucao(saida_solu);


	

	solucao << "Iminus" << endl;
	for (i = 0; i < N; i++)
	{
		for (t = 1; t < T; t++) {
			solucao << cplex.getValue(I_minus[i][t]) << " ";
		}
		solucao << endl;
	}
	solucao << endl;

	vector<int> estoque_usado(T, 0);
	solucao << "I_plus" << endl;
	for (i = 0; i < N; i++)
	{
		for (t = 1; t < T; t++) {
			solucao << cplex.getValue(I_plus[i][t]) << " ";
			estoque_usado[t] += cplex.getValue(I_plus[i][t]);
		}
		solucao << endl;
	}
	solucao << "Estoque usado" << endl;
	for (t = 1; t < T; t++) {
		solucao << estoque_usado[t] << " ";
	}


	solucao << endl;
	solucao << endl;
	solucao << "q" << endl;
	for (l = 0; l < M; l++) {
		for (i = 0; i < N; i++)
		{
			if (l_produz_i[l][i]) {
				for (t = 1; t < T; t++) {
					for (s = (t - 1) * W_p + 1; s <= t * W_p; s++) {
						solucao << cplex.getValue(q[i][l][s]) << " ";
					}
				}
			}
			solucao << endl;
		}
		solucao << endl;
		solucao << endl;
	}

	solucao.close();


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

void RF::imprimirdadosgerais()
{
	int i, l, j, s, t;
	vector<int> demanda_periodo(T, 0);

	double
		min = INT_MAX,
		max = -INT_MAX,
		media = 0.0;
	for (t = 1; t < T; t++) {
		for (i = 0; i < N; i++) {
			demanda_periodo[t] += d[i][t];
		}
		if (demanda_periodo[t] > max) {
			max = demanda_periodo[t];
		}
		if (demanda_periodo[t] < min) {
			min = demanda_periodo[t];
		}
		media += demanda_periodo[t];
	}
	media /= (T-1);

	ofstream output("dados.csv", fstream::app);

	output << instancia << "," << min << "," << media << "," << max << ",";

	output << instancia << "," << min << "," << media << "," << max << ",";

	min = INT_MAX;
	max = -INT_MAX;
	media = 0.0;
	vector<int> demanda(N, 0);
	for (i = 0; i < N; i++) {
		for (t = 1; t < T; t++) {
			demanda[i] += d[i][t];
		}

		if (demanda[i] > max) {
			max = demanda[i];
		}
		if (demanda[i] < min) {
			min = demanda[i];
		}
		media += demanda[i];

	}
	media /= N;
	output << min << "," << media << "," << max << ",";


	vector<int> flexibilidade(N, 0);
	min = INT_MAX;
	max = -INT_MAX;
	media = 0.0;


	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++)
		{
			if (l_produz_i[l][i])
			{
				flexibilidade[i]++;
			}
		}

		if (flexibilidade[i] > max) {
			max = flexibilidade[i];
		}
		if (flexibilidade[i] < min) {
			min = flexibilidade[i];
		}
		media += flexibilidade[i];
	}
	media /= N;

	output << min << "," << media << "," << max << ",";



	min = INT_MAX;
	max = -INT_MAX;
	media = 0.0;
	vector<double> eficiencia(M, 0);
	for (l = 0; l < M; l++) {
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i]) {
				eficiencia[l] += p[i][l] + cp[i][l];
			}
		}
		eficiencia[l] /= (double)CP[l].size(); //cp tem a mesma cardinalidade de \cal I _ \ell por isso que usei ele

		if (eficiencia[l] > max) {
			max = eficiencia[l];
		}
		if (eficiencia[l] < min) {
			min = eficiencia[l];
		}
		media += eficiencia[l];
	}
	media /= M;
	output << min << "," << media << "," << max << ",";



	min = INT_MAX;
	max = -INT_MAX;
	media = 0.0;
	vector<int> f_i(N, 0);
	for (l = 0; l < M; l++) {
		for (auto sp : SP[l]) {
			f_i[sp]++;
		}
	}
	vector<int> criticidade(M, 0);
	for (l = 0; l < M; l++) {
		int min_int = INT_MAX;
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i] && f_i[i] < min_int) {
				min_int = f_i[i];
			}
		}
		criticidade[l] = M - min_int;
		if (criticidade[l] > max) {
			max = criticidade[l];
		}
		if (criticidade[l] < min) {
			min = criticidade[l];
		}
		media += criticidade[l];
	}
	media /= M;
	output << min << "," << media << "," << max << endl;

	output.close();

}


vector<vector<variavel>> RF::RF_S1(vector<variavel> particoes_completas, int K) {
	vector<vector<variavel>> particoes;
	int n_var_part = ceil((double)particoes_completas.size() / K);

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	//influcencia S10
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.influ > j.influ;});
	//distancia S9
	//std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.dist > j.dist;});

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.s < j.s;});
		
	vector<variavel> var_list;

	int cont = 0;

	for (auto& var : particoes_completas) {
		cont++;
		var_list.push_back(var);

		if (cont % n_var_part == 0 || cont == particoes_completas.size()) { //tá adaptado para K = N, senão tem q mudar
			particoes.push_back(var_list);
			var_list.clear();
		}
	}


	return particoes;
}

vector<vector<variavel>> RF::RF_S2(vector<variavel> particoes_completas, int K) {
	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)particoes_completas.size() / K);

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	//influcencia S10
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.influ > j.influ;});
	//distancia S9
	//std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.dist > j.dist;});

	vector<int> demanda_periodo(T, 0);
	for (t = 0; t < T; t++) {
		for (i = 0; i < N; i++) {
			demanda_periodo[t] += d[i][t];
		}
	}
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return demanda_periodo[i.t] > demanda_periodo[j.t];});

	vector<variavel> var_list;

	int cont = 0;

	for (auto& var : particoes_completas) {
		cont++;
		var_list.push_back(var);

		if (cont % n_var_part == 0 || cont == particoes_completas.size()) { //tá adaptado para K = N, senão tem q mudar
			particoes.push_back(var_list);
			var_list.clear();
		}
	}

	return particoes;
}

vector<vector<variavel>> RF::RF_S3(vector<variavel> particoes_completas, int K) {
	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)particoes_completas.size() / K);

	vector<int> demanda(N, 0);
	for (i = 0; i < N; i++) {
		for (t = 1; t < T; t++) {
			demanda[i] += d[i][t];
		}
	}
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	//influcencia S10
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.influ > j.influ;});
	//distancia S9
	//std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.dist > j.dist;});

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return demanda[i.t] > demanda[j.t];});

	vector<variavel> var_list;

	int cont = 0;

	for (auto& var : particoes_completas) {
		cont++;
		var_list.push_back(var);

		if (cont % n_var_part == 0 || cont == particoes_completas.size()) { //tá adaptado para K = N, senão tem q mudar
			particoes.push_back(var_list);
			var_list.clear();
		}
	}

	return particoes;
}


vector<vector<variavel>> RF::RF_S4(vector<variavel> particoes_completas, int K) {
	vector<vector<variavel>> particoes;

	IloInt i, l, s, t;

	int n_var_part = ceil((double)particoes_completas.size() / K);

	vector<int> demanda(N, 0);
	for (i = 0; i < N; i++) {
		for (t = 1; t < T; t++) {
			demanda[i] += d[i][t];
		}
	}


	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i; });
	//influcencia S10
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.influ > j.influ; });
	//distancia S9
	//std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.dist > j.dist;});
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return demanda[i.i] < demanda[j.i]; });


	vector<variavel> var_list;

	int cont = 0;

	for (auto& var : particoes_completas) {
		cont++;
		var_list.push_back(var);

		if (cont % n_var_part == 0 || cont == particoes_completas.size()) { //tá adaptado para K = N, senão tem q mudar
			particoes.push_back(var_list);
			var_list.clear();
		}
	}

	return particoes;
}

vector<vector<variavel>> RF::RF_S5(vector<variavel> particoes_completas, int K) {
	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)particoes_completas.size() / K);


	vector<int> flexibilidade(N, 0);

	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++)
		{
			if (l_produz_i[l][i])
			{
				flexibilidade[i]++;
			}

		}
	}
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	//influcencia S10
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.influ > j.influ;});
	//distancia S9
	//std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.dist > j.dist;});
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return flexibilidade[i.i] > flexibilidade[j.i];});

	vector<variavel> var_list;

	int cont = 0;

	for (auto& var : particoes_completas) {
		cont++;
		var_list.push_back(var);

		if (cont % n_var_part == 0 || cont == particoes_completas.size()) { //tá adaptado para K = N, senão tem q mudar
			particoes.push_back(var_list);
			var_list.clear();
		}
	}

	return particoes;
}

vector<vector<variavel>> RF::RF_S6(vector<variavel> particoes_completas, int K) {
	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)particoes_completas.size() / K);


	vector<double> eficiencia(M, 0);
	for (l = 0; l < M; l++) {
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i]) {
				eficiencia[l] += p[i][l] + cp[i][l];
			}
		}
		eficiencia[l] /= (double)CP[l].size(); //cp tem a mesma cardinalidade de \cal I _ \ell por isso que usei ele
	}
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	//influcencia S10
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.influ > j.influ;});
	//distancia S9
	//std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.dist > j.dist;});

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return eficiencia[i.l] < eficiencia[j.l];});

	vector<variavel> var_list;

	int cont = 0;

	for (auto& var : particoes_completas) {
		cont++;
		var_list.push_back(var);

		if (cont % n_var_part == 0 || cont == particoes_completas.size()) { //tá adaptado para K = N, senão tem q mudar
			particoes.push_back(var_list);
			var_list.clear();
		}
	}

	return particoes;
}


vector<vector<variavel>> RF::RF_S7(vector<variavel> particoes_completas, int K) {
	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)particoes_completas.size() / K);


	vector<double> eficiencia(M, 0);
	for (l = 0; l < M; l++) {
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i]) {
				eficiencia[l] += p[i][l] + cp[i][l];
			}
		}
		eficiencia[l] /= (double)CP[l].size(); //cp tem a mesma cardinalidade de \cal I _ \ell por isso que usei ele
	}
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	//influcencia S10
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.influ > j.influ;});
	//distancia S9
	//std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.dist > j.dist;});

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return eficiencia[i.l] > eficiencia[j.l];});

	vector<variavel> var_list;

	int cont = 0;

	for (auto& var : particoes_completas) {
		cont++;
		var_list.push_back(var);

		if (cont % n_var_part == 0 || cont == particoes_completas.size()) { //tá adaptado para K = N, senão tem q mudar
			particoes.push_back(var_list);
			var_list.clear();
		}
	}

	return particoes;
}

vector<vector<variavel>> RF::RF_S8(vector<variavel> particoes_completas, int K) {
	vector<vector<variavel>> particoes;
	int n_var_part = ceil((double)particoes_completas.size() / K);

	IloInt i, l, s, t;

	vector<int> f_i(N, 0);
	for (l = 0; l < M; l++) {
		for (auto sp : SP[l]) {
			f_i[sp]++;
		}
	}

	vector<int> criticidade(M, 0);
	for (l = 0; l < M; l++) {
		int min = INT_MAX;
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i] && f_i[i] < min) {
				min = f_i[i];
			}
		}
		criticidade[l] = M - min;
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	//influcencia S10
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.influ > j.influ;});
	//distancia S9
	//std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.dist > j.dist;});
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});


	vector<variavel> var_list;

	int cont = 0;

	for (auto& var : particoes_completas) {
		cont++;
		var_list.push_back(var);

		if (cont % n_var_part == 0 || cont == particoes_completas.size()) { //tá adaptado para K = N, senão tem q mudar
			particoes.push_back(var_list);
			var_list.clear();
		}
	}

	return particoes;
}

vector<vector<variavel>> RF::RF_S9(vector<variavel> particoes_completas, int K) {
	vector<vector<variavel>> particoes;
	int n_var_part = ceil((double)particoes_completas.size() / K);


	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	//distancia S9
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.dist > j.dist;});

	vector<variavel> var_list;

	int cont = 0;

	for (auto& var : particoes_completas) {
		cont++;
		var_list.push_back(var);

		if (cont % n_var_part == 0 || cont == particoes_completas.size()) { //tá adaptado para K = N, senão tem q mudar
			particoes.push_back(var_list);
			var_list.clear();
		}
	}

	return particoes;
}


vector<vector<variavel>> RF::RF_S10(vector<variavel> particoes_completas, int K) {
	vector<vector<variavel>> particoes;
	int n_var_part = ceil((double)particoes_completas.size() / K);


	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	//influcencia S10
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.influ > j.influ;});
	//distancia S9
	//std::stable_sort(part.begin(), part.end(), [&](variavel i, variavel j) {return i.dist > j.dist;});

	vector<variavel> var_list;

	int cont = 0;

	for (auto& var : particoes_completas) {
		cont++;
		var_list.push_back(var);

		if (cont % n_var_part == 0 || cont == particoes_completas.size()) { //tá adaptado para K = N, senão tem q mudar
			particoes.push_back(var_list);
			var_list.clear();
		}
	}

	return particoes;
}


//recebe um vector de variáveis e o número de partições K
//S11 com S10
vector<vector<variavel>> RF::RF_S112(vector<variavel> particoes_completas, int K){
	//declara vector de afinidades de tamanho de itens N
	vector<double> afinidade(N, 0.0);
	
	//calcula afinidade para cada item
	for (int i = 0; i < N; i++)	{
		double
			min1 = INFINITY,
			min2 = INFINITY;
		for (int l = 0; l < M; l++) {
			if (l_produz_i[l][i] && p[i][l] <= min1) {
				min2 = min1;
				min1 = p[i][l];
			}
		}
		afinidade[i] = min2 - min1;
	}
	//inicia variável de particoes que sera retornada
	vector<vector<variavel>> particoes;
	//calcula o número máximo de variáveis por partição
	int n_var_part = ceil((double)particoes_completas.size() / K);

	//ordena por item
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});

	//reordena por afinidade mantendo a ordenação base de itens
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return afinidade[i.i] > afinidade[j.i];}); 
	
	//Vector que guarda o número de variáveis associada a cada produto.
	vector<int> tamanhos(N, 1);
	//iteradores
	int
		it = 0,
		i = 0;
	while (it < particoes_completas.size())	{
		if (it + 1 == particoes_completas.size())
			break;
		if (particoes_completas[it].i == particoes_completas[it + 1].i)
			tamanhos[i]++;
		else
			i++;
		it++;
	}	


	//ordena cada subvector por tempo de processamento das máquinas
	int acumulado = 0;
	for (int i = 0; i < N; i++)
	{
		std::stable_sort(particoes_completas.begin()+acumulado, particoes_completas.begin()+acumulado+tamanhos[i], [&](variavel i, variavel j) {return  p[i.i][i.l] < p[j.i][j.l];});
		acumulado += tamanhos[i];
	}
	
	//montando partições seguindo as ordens pré-estabelecidas.
	vector<variavel> var_list;

	int cont = 0;
	for (auto& var : particoes_completas) {
		cont++;
		var_list.push_back(var);

		if (cont % n_var_part == 0 || cont == particoes_completas.size()) { //tá adaptado para K = N, senão tem q mudar
			particoes.push_back(var_list);
			var_list.clear();
		}
	}

	return particoes;
}

vector<vector<variavel>> RF::RF_S11(vector<variavel> particoes_completas, int K) {
	//declara vector de afinidades de tamanho de itens N
	vector<double> afinidade(N, 0.0);

	//calcula afinidade para cada item
	for (int i = 0; i < N; i++) {
		double
			min1 = INFINITY,
			min2 = INFINITY;
		for (int l = 0; l < M; l++) {
			if (l_produz_i[l][i] && p[i][l] <= min1) {
				min2 = min1;
				min1 = p[i][l];
			}
		}
		afinidade[i] = min2 - min1;
	}
	//inicia variável de particoes que sera retornada
	vector<vector<variavel>> particoes;
	//calcula o número máximo de variáveis por partição
	int n_var_part = ceil((double)particoes_completas.size() / K);

	//ordena por item
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.influ > j.influ;});

	//reordena por afinidade mantendo a ordenação base de itens
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return afinidade[i.i] > afinidade[j.i];});


	


	//montando partições seguindo as ordens pré-estabelecidas.
	vector<variavel> var_list;

	int cont = 0;
	for (auto& var : particoes_completas) {
		cont++;
		var_list.push_back(var);

		if (cont % n_var_part == 0 || cont == particoes_completas.size()) { //tá adaptado para K = N, senão tem q mudar
			particoes.push_back(var_list);
			var_list.clear();
		}
	}

	return particoes;
}

vector<vector<variavel>> RF::HRF1_S1_S5(vector<variavel> particoes_completas, int K){

	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)particoes_completas.size() / K);


	vector<int> flexibilidade(N, 0);
	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++){
			if (l_produz_i[l][i]){
				flexibilidade[i]++;
			}

		}
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	vector<variavel>
		particoes_est1 = particoes_completas,
		particoes_est2 = particoes_completas;
	vector<bool> adicionado(particoes_completas.size(), false);



	//S1 desempatada com S5
	std::stable_sort(particoes_est2.begin(), particoes_est2.end(), [&](variavel i, variavel j) {return flexibilidade[i.i] > flexibilidade[j.i];});
	std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return i.s < j.s;});

	
	//S5 desempatada com S1
	std::stable_sort(particoes_est2.begin(), particoes_est2.end(), [&](variavel i, variavel j) {return i.s < j.s;});
	std::stable_sort(particoes_est2.begin(), particoes_est2.end(), [&](variavel i, variavel j) {return flexibilidade[i.i] > flexibilidade[j.i];});


	vector<variavel> var_list;
	int
		cont1 = 0,
		cont2 = 0,
		n_add = 0;

	particoes = vector<vector<variavel>>(K);

	for (int k = 0; k < K; k++){
		int it_var = 0;
		while (it_var < n_var_part && n_add < particoes_completas.size()) {
			bool add = false;
			if ((it_var % 2) == 0 && cont1 < particoes_completas.size()){
				if (!adicionado[particoes_est1[cont1].ind_geral]) {
					particoes[k].push_back(particoes_est1[cont1]);
					adicionado[particoes_est1[cont1].ind_geral] = true;
					add = true;
				}
				cont1++;
			}
			else {
				if (cont2 < particoes_completas.size()) {
					if (!adicionado[particoes_est2[cont2].ind_geral]) {
						particoes[k].push_back(particoes_est2[cont2]);
						adicionado[particoes_est2[cont2].ind_geral] = true;
						add = true;
					}
					cont2++;
				}
			}
			if (cont1 == particoes_completas.size() && cont2 == particoes_completas.size())
				break;
			if (add) {
				it_var++;
				n_add++;
			}
		}

	}

	return particoes;
}

vector<vector<variavel>> RF::HRF1_S1_S8(vector<variavel> particoes_completas, int K) {

	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)particoes_completas.size() / K);


	vector<int> f_i(N, 0);
	for (l = 0; l < M; l++) {
		for (auto sp : SP[l]) {
			f_i[sp]++;
		}
	}

	vector<int> criticidade(M, 0);
	for (l = 0; l < M; l++) {
		int min = INT_MAX;
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i] && f_i[i] < min) {
				min = f_i[i];
			}
		}
		criticidade[l] = M - min;
	}



	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	vector<variavel>
		particoes_est1 = particoes_completas,
		particoes_est2 = particoes_completas;
	vector<bool> adicionado(particoes_completas.size(), false);



	//S1 desempatada com S8
	std::stable_sort(particoes_est2.begin(), particoes_est2.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});
	std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return i.s < j.s;});


	//S8 desempatada com S1
	std::stable_sort(particoes_est2.begin(), particoes_est2.end(), [&](variavel i, variavel j) {return i.s < j.s;});
	std::stable_sort(particoes_est2.begin(), particoes_est2.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});


	vector<variavel> var_list;
	int
		cont1 = 0,
		cont2 = 0,
		n_add = 0;

	particoes = vector<vector<variavel>>(K);

	for (int k = 0; k < K; k++) {
		int it_var = 0;
		while (it_var < n_var_part && n_add < particoes_completas.size()) {
			bool add = false;
			if ((it_var % 2) == 0 && cont1 < particoes_completas.size()) {
				if (!adicionado[particoes_est1[cont1].ind_geral]) {
					particoes[k].push_back(particoes_est1[cont1]);
					adicionado[particoes_est1[cont1].ind_geral] = true;
					add = true;
				}
				cont1++;
			}
			else {
				if (cont2 < particoes_completas.size()) {
					if (!adicionado[particoes_est2[cont2].ind_geral]) {
						particoes[k].push_back(particoes_est2[cont2]);
						adicionado[particoes_est2[cont2].ind_geral] = true;
						add = true;
					}
					cont2++;
				}
			}
			if (cont1 == particoes_completas.size() && cont2 == particoes_completas.size())
				break;
			if (add) {
				it_var++;
				n_add++;
			}
		}

	}
	return particoes;
}

vector<vector<variavel>> RF::HRF1_S5_S8(vector<variavel> particoes_completas, int K)
{
	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)particoes_completas.size() / K);


	vector<int> f_i(N, 0);
	for (l = 0; l < M; l++) {
		for (auto sp : SP[l]) {
			f_i[sp]++;
		}
	}

	vector<int> criticidade(M, 0);
	for (l = 0; l < M; l++) {
		int min = INT_MAX;
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i] && f_i[i] < min) {
				min = f_i[i];
			}
		}
		criticidade[l] = M - min;
	}
	vector<int> flexibilidade(N, 0);
	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {
				flexibilidade[i]++;
			}

		}
	}



	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	vector<variavel>
		particoes_est1 = particoes_completas,
		particoes_est2 = particoes_completas;
	vector<bool> adicionado(particoes_completas.size(), false);



	//S5 desempatada com S8
	std::stable_sort(particoes_est2.begin(), particoes_est2.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});
	std::stable_sort(particoes_est2.begin(), particoes_est2.end(), [&](variavel i, variavel j) {return flexibilidade[i.i] > flexibilidade[j.i];});


	//S8 desempatada com S5
	std::stable_sort(particoes_est2.begin(), particoes_est2.end(), [&](variavel i, variavel j) {return flexibilidade[i.i] > flexibilidade[j.i];});
	std::stable_sort(particoes_est2.begin(), particoes_est2.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});


	vector<variavel> var_list;
	int
		cont1 = 0,
		cont2 = 0,
		n_add = 0;

	particoes = vector<vector<variavel>>(K);

	for (int k = 0; k < K; k++) {
		int it_var = 0;
		while (it_var < n_var_part && n_add < particoes_completas.size()) {
			bool add = false;
			if ((it_var % 2) == 0 && cont1 < particoes_completas.size()) {
				if (!adicionado[particoes_est1[cont1].ind_geral]) {
					particoes[k].push_back(particoes_est1[cont1]);
					adicionado[particoes_est1[cont1].ind_geral] = true;
					add = true;
				}
				cont1++;
			}
			else {
				if (cont2 < particoes_completas.size()) {
					if (!adicionado[particoes_est2[cont2].ind_geral]) {
						particoes[k].push_back(particoes_est2[cont2]);
						adicionado[particoes_est2[cont2].ind_geral] = true;
						add = true;
					}
					cont2++;
				}
			}
			if (cont1 == particoes_completas.size() && cont2 == particoes_completas.size())
				break;
			if (add) {
				it_var++;
				n_add++;
			}
		}

	}
	return particoes;
}

vector<vector<variavel>> RF::HRF1_S1_S5_S8(vector<variavel> particoes_completas, int K){
	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)particoes_completas.size() / K);

	vector<int> f_i(N, 0);
	for (l = 0; l < M; l++) {
		for (auto sp : SP[l]) {
			f_i[sp]++;
		}
	}

	vector<int> criticidade(M, 0);
	for (l = 0; l < M; l++) {
		int min = INT_MAX;
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i] && f_i[i] < min) {
				min = f_i[i];
			}
		}
		criticidade[l] = M - min;
	}
	vector<int> flexibilidade(N, 0);
	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {
				flexibilidade[i]++;
			}

		}
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.influ > j.influ;});
	vector<variavel>
		particoes_est1 = particoes_completas,
		particoes_est5 = particoes_completas,
		particoes_est8 = particoes_completas;
	vector<bool> adicionado(particoes_completas.size(), false);


	std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return i.s < j.s;});

	//S5 desempatada com S10
	std::stable_sort(particoes_est5.begin(), particoes_est5.end(), [&](variavel i, variavel j) {return flexibilidade[i.i] > flexibilidade[j.i];});

	//S8 desempatada com S10
	std::stable_sort(particoes_est8.begin(), particoes_est8.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});


	vector<variavel> var_list;
	int
		cont1 = 0,
		cont2 = 0,
		cont3 = 0,
		n_add = 0,
		aux = 1;

	particoes = vector<vector<variavel>>(K);
	
	for (int k = 0; k < K; k++) {
		int it_var = 0;
		while (it_var < n_var_part && n_add < particoes_completas.size()) {
			bool add = false;
			if (aux == 1 && cont1 < particoes_completas.size()) {
				if (!adicionado[particoes_est1[cont1].ind_geral]) {
					particoes[k].push_back(particoes_est1[cont1]);
					adicionado[particoes_est1[cont1].ind_geral] = true;
					add = true;
					aux++;
				}
				cont1++;
			}
			else{
				if (aux == 2 && cont2 < particoes_completas.size()) {
					if (!adicionado[particoes_est5[cont2].ind_geral]) {
						particoes[k].push_back(particoes_est5[cont2]);
						adicionado[particoes_est5[cont2].ind_geral] = true;
						add = true;
						aux++;
					}
					cont2++;
				}
				else {
					if (aux == 3 && cont3 < particoes_completas.size()) {
						if (!adicionado[particoes_est8[cont3].ind_geral]) {
							particoes[k].push_back(particoes_est8[cont3]);
							adicionado[particoes_est8[cont3].ind_geral] = true;
							add = true;
							aux = 1;
						}
						cont3++;
					}
				}
			}

			if (cont1 == particoes_completas.size() && cont2 == particoes_completas.size() && cont3 == particoes_completas.size())
				break;
			if (add) {
				it_var++;
				n_add++;
			}
		}

	}
	return particoes;
}

vector<vector<variavel>> RF::HRF2_S1_S5(vector<variavel> particoes_completas, int K)
{
	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int p_n = particoes_completas.size();
	int n_var_part = ceil((double)p_n / K);


	vector<int> flexibilidade(N, 0);
	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {
				flexibilidade[i]++;
			}

		}
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	vector<variavel>
		particoes_est1 = particoes_completas,
		particoes_est2 = particoes_completas;
	vector<bool> adicionado(p_n, false);


	//S1
	std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return i.s < j.s;});
	std::stable_sort(particoes_est2.begin(), particoes_est2.end(), [&](variavel i, variavel j) {return flexibilidade[i.i] > flexibilidade[j.i];});


	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.ind_geral < j.ind_geral;});
	for (int i = 0; i < p_n; i++){
		particoes_completas[particoes_est1[i].ind_geral].ranking += i;
		particoes_completas[particoes_est2[i].ind_geral].ranking += i;
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.ranking < j.ranking;});


	vector<variavel> var_list;

	particoes = vector<vector<variavel>>(K);

	int cont = 0;
	for (int k = 0; k < K; k++) {
		int it_var = 0;
		vector<variavel> particoes_aux;
		while (it_var < n_var_part && cont < particoes_completas.size()) {

			particoes_aux.push_back(particoes_completas[cont]);
			cont++;
			it_var++;
		}
		particoes[k] = particoes_aux;
		particoes_aux.end();
	}

	return particoes;
}

vector<vector<variavel>> RF::HRF2_S5_S8(vector<variavel> particoes_completas, int K)
{
	vector<vector<variavel>> particoes;
	int p_n = particoes_completas.size();
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)p_n / K);

	vector<int> f_i(N, 0);
	for (l = 0; l < M; l++) {
		for (auto sp : SP[l]) {
			f_i[sp]++;
		}
	}

	vector<int> flexibilidade(N, 0);
	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {
				flexibilidade[i]++;
			}
		}
	}


	vector<int> criticidade(M, 0);
	for (l = 0; l < M; l++) {
		int min = INT_MAX;
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i] && f_i[i] < min) {
				min = f_i[i];
			}
		}
		criticidade[l] = M - min;
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	vector<variavel>
		particoes_est1 = particoes_completas,
		particoes_est2 = particoes_completas;
	vector<bool> adicionado(p_n, false);


	//S1
	std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return flexibilidade[i.i] > flexibilidade[j.i];});
	std::stable_sort(particoes_est2.begin(), particoes_est2.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});


	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.ind_geral < j.ind_geral;});
	for (int i = 0; i < p_n; i++) {
		particoes_completas[particoes_est1[i].ind_geral].ranking += i;
		particoes_completas[particoes_est2[i].ind_geral].ranking += i;
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.ranking < j.ranking;});


	vector<variavel> var_list;

	particoes = vector<vector<variavel>>(K);

	int cont = 0;
	for (int k = 0; k < K; k++) {
		int it_var = 0;
		vector<variavel> particoes_aux;
		while (it_var < n_var_part && cont < particoes_completas.size()) {

			particoes_aux.push_back(particoes_completas[cont]);
			cont++;
			it_var++;
		}
		particoes[k] = particoes_aux;
		particoes_aux.end();
	}

	return particoes;
}

vector<vector<variavel>> RF::HRF2_S1_S8(vector<variavel> particoes_completas, int K)
{
	vector<vector<variavel>> particoes;
	int p_n = particoes_completas.size();
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)p_n / K);

	vector<int> f_i(N, 0);
	for (l = 0; l < M; l++) {
		for (auto sp : SP[l]) {
			f_i[sp]++;
		}
	}

	vector<int> criticidade(M, 0);
	for (l = 0; l < M; l++) {
		int min = INT_MAX;
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i] && f_i[i] < min) {
				min = f_i[i];
			}
		}
		criticidade[l] = M - min;
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	vector<variavel>
		particoes_est1 = particoes_completas,
		particoes_est2 = particoes_completas;
	vector<bool> adicionado(p_n, false);


	//S1
	std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return i.s < j.s;});
	std::stable_sort(particoes_est2.begin(), particoes_est2.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});


	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.ind_geral < j.ind_geral;});
	for (int i = 0; i < p_n; i++) {
		particoes_completas[particoes_est1[i].ind_geral].ranking += i;
		particoes_completas[particoes_est2[i].ind_geral].ranking += i;
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.ranking < j.ranking;});


	vector<variavel> var_list;

	particoes = vector<vector<variavel>>(K);

	int cont = 0;
	for (int k = 0; k < K; k++) {
		int it_var = 0;
		vector<variavel> particoes_aux;
		while (it_var < n_var_part && cont < particoes_completas.size()) {

			particoes_aux.push_back(particoes_completas[cont]);
			cont++;
			it_var++;
		}
		particoes[k] = particoes_aux;
		particoes_aux.end();
	}

	return particoes;
}

vector<vector<variavel>> RF::HRF2_S1_S5_S8(vector<variavel> particoes_completas, int K)
{
	vector<vector<variavel>> particoes;
	int p_n = particoes_completas.size();
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)p_n / K);

	vector<int> f_i(N, 0);
	for (l = 0; l < M; l++) {
		for (auto sp : SP[l]) {
			f_i[sp]++;
		}
	}

	vector<int> flexibilidade(N, 0);
	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {
				flexibilidade[i]++;
			}
		}
	}

	vector<int> criticidade(M, 0);
	for (l = 0; l < M; l++) {
		int min = INT_MAX;
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i] && f_i[i] < min) {
				min = f_i[i];
			}
		}
		criticidade[l] = M - min;
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	vector<variavel>
		particoes_est1 = particoes_completas,
		particoes_est2 = particoes_completas,
		particoes_est3 = particoes_completas;
	vector<bool> adicionado(p_n, false);



	std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return i.s < j.s;});
	std::stable_sort(particoes_est2.begin(), particoes_est2.end(), [&](variavel i, variavel j) {return flexibilidade[i.i] > flexibilidade[j.i];});
	std::stable_sort(particoes_est3.begin(), particoes_est3.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});


	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.ind_geral < j.ind_geral;});
	for (int i = 0; i < p_n; i++) {
		particoes_completas[particoes_est1[i].ind_geral].ranking += i;
		particoes_completas[particoes_est2[i].ind_geral].ranking += i;
		particoes_completas[particoes_est3[i].ind_geral].ranking += i;
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.ranking < j.ranking;});


	vector<variavel> var_list;

	particoes = vector<vector<variavel>>(K);

	int cont = 0;
	for (int k = 0; k < K; k++) {
		int it_var = 0;
		vector<variavel> particoes_aux;
		while (it_var < n_var_part && cont < particoes_completas.size()) {

			particoes_aux.push_back(particoes_completas[cont]);
			cont++;
			it_var++;
		}
		particoes[k] = particoes_aux;
		particoes_aux.end();
	}

	return particoes;
}

vector<vector<variavel>> RF::HRF3_S1_S8(vector<variavel> particoes_completas, int K)
{
	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)particoes_completas.size() / K);

	vector<int> f_i(N, 0);
	for (l = 0; l < M; l++) {
		for (auto sp : SP[l]) {
			f_i[sp]++;
		}
	}

	vector<int> criticidade(M, 0);
	for (l = 0; l < M; l++) {
		int min = INT_MAX;
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i] && f_i[i] < min) {
				min = f_i[i];
			}
		}
		criticidade[l] = M - min;
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	vector<variavel>
		particoes_est1 = particoes_completas,
		particoes_est2 = particoes_completas;
	vector<bool> adicionado(particoes_completas.size(), false);

	int
		cont1 = 0,
		cont2 = 0,
		n_add = 0;

	//S1
	std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});
	std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return i.s < j.s;});


	particoes = vector<vector<variavel>>(K);
	int soma = 0;
	for (int k = 0; k < K; k++) {
		int it_var = 0,
			cont1 = 0;
		vector<variavel> particoes_aux;
		while (it_var < 2 * n_var_part && cont1 < particoes_completas.size()) {
			bool add = false;
			if (!adicionado[particoes_est1[cont1].ind_geral]) {
				particoes_aux.push_back(particoes_est1[cont1]);
				add = true;
			}
			cont1++;
			if (add)
				it_var++;
		}
		//S8
		std::stable_sort(particoes_aux.begin(), particoes_aux.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});
		it_var = 0;
		while (it_var < n_var_part && it_var < particoes_aux.size()) {
			particoes[k].push_back(particoes_aux[it_var]);
			it_var++;
			adicionado[particoes[k].back().ind_geral] = true;
			soma += adicionado[particoes[k].back().ind_geral];
		}

		k++;
		while (it_var < 2* n_var_part && it_var < particoes_aux.size()) {
			particoes[k].push_back(particoes_aux[it_var]);
			it_var++;
			adicionado[particoes[k].back().ind_geral] = true;
			soma += adicionado[particoes[k].back().ind_geral];
		}

		particoes_aux.end();
	}

	return particoes;
}

vector<vector<variavel>> RF::HRF4_S1_S5(vector<variavel> particoes_completas, int K)
{
	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)particoes_completas.size() / K);

	vector<int> f_i(N, 0);
	for (l = 0; l < M; l++) {
		for (auto sp : SP[l]) {
			f_i[sp]++;
		}
	}
	vector<int> flexibilidade(N, 0);
	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {
				flexibilidade[i]++;
			}
		}
	}

	vector<int> criticidade(M, 0);
	for (l = 0; l < M; l++) {
		int min = INT_MAX;
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i] && f_i[i] < min) {
				min = f_i[i];
			}
		}
		criticidade[l] = M - min;
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	vector<variavel>
		particoes_est1 = particoes_completas,
		particoes_est2 = particoes_completas;
	vector<bool> adicionado(particoes_completas.size(), false);

	int
		cont1 = 0,
		cont2 = 0,
		n_add = 0;

	//S1
	//std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});
	std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return i.s < j.s;});


	particoes = vector<vector<variavel>>(K);
	int soma = 0;
	for (int k = 0; k < K; k++) {
		int it_var = 0,
			cont1 = 0;
		vector<variavel> particoes_aux;
		while (it_var < 2 * n_var_part && cont1 < particoes_completas.size()) {
			bool add = false;
			if (!adicionado[particoes_est1[cont1].ind_geral]) {
				particoes_aux.push_back(particoes_est1[cont1]);
				add = true;
			}
			cont1++;
			if (add)
				it_var++;
		}
		//S5
		std::stable_sort(particoes_aux.begin(), particoes_aux.end(), [&](variavel i, variavel j) {return flexibilidade[i.i] > flexibilidade[j.i];});
		it_var = 0;
		while (it_var < n_var_part && it_var < particoes_aux.size()) {
			particoes[k].push_back(particoes_aux[it_var]);
			it_var++;
			adicionado[particoes[k].back().ind_geral] = true;
			soma += adicionado[particoes[k].back().ind_geral];
		}

		particoes_aux.end();
	}

	return particoes;
}

vector<vector<variavel>> RF::HRF4_S1_S8(vector<variavel> particoes_completas, int K)
{
	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)particoes_completas.size() / K);

	vector<int> f_i(N, 0);
	for (l = 0; l < M; l++) {
		for (auto sp : SP[l]) {
			f_i[sp]++;
		}
	}
	vector<int> flexibilidade(N, 0);
	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {
				flexibilidade[i]++;
			}
		}
	}

	vector<int> criticidade(M, 0);
	for (l = 0; l < M; l++) {
		int min = INT_MAX;
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i] && f_i[i] < min) {
				min = f_i[i];
			}
		}
		criticidade[l] = M - min;
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	vector<variavel>
		particoes_est1 = particoes_completas,
		particoes_est2 = particoes_completas;
	vector<bool> adicionado(particoes_completas.size(), false);

	int
		cont1 = 0,
		cont2 = 0,
		n_add = 0;

	//S1
	//std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});
	std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return i.s < j.s;});


	particoes = vector<vector<variavel>>(K);
	int soma = 0;
	for (int k = 0; k < K; k++) {
		int it_var = 0,
			cont1 = 0;
		vector<variavel> particoes_aux;
		while (it_var < 2 * n_var_part && cont1 < particoes_completas.size()) {
			bool add = false;
			if (!adicionado[particoes_est1[cont1].ind_geral]) {
				particoes_aux.push_back(particoes_est1[cont1]);
				add = true;
			}
			cont1++;
			if (add)
				it_var++;
		}
		//S8
		std::stable_sort(particoes_aux.begin(), particoes_aux.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});
		it_var = 0;
		while (it_var < n_var_part && it_var < particoes_aux.size()) {
			particoes[k].push_back(particoes_aux[it_var]);
			it_var++;
			adicionado[particoes[k].back().ind_geral] = true;
			soma += adicionado[particoes[k].back().ind_geral];
		}

		particoes_aux.end();
	}

	return particoes;
}

vector<vector<variavel>> RF::HRF4_S5_S1(vector<variavel> particoes_completas, int K)
{
	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)particoes_completas.size() / K);

	vector<int> f_i(N, 0);
	for (l = 0; l < M; l++) {
		for (auto sp : SP[l]) {
			f_i[sp]++;
		}
	}
	vector<int> flexibilidade(N, 0);
	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {
				flexibilidade[i]++;
			}
		}
	}

	vector<int> criticidade(M, 0);
	for (l = 0; l < M; l++) {
		int min = INT_MAX;
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i] && f_i[i] < min) {
				min = f_i[i];
			}
		}
		criticidade[l] = M - min;
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	vector<variavel>
		particoes_est1 = particoes_completas,
		particoes_est2 = particoes_completas;
	vector<bool> adicionado(particoes_completas.size(), false);

	int
		cont1 = 0,
		cont2 = 0,
		n_add = 0;

	//S5
	std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return flexibilidade[i.i] > flexibilidade[j.i];});


	particoes = vector<vector<variavel>>(K);
	int soma = 0;
	for (int k = 0; k < K; k++) {
		int it_var = 0,
			cont1 = 0;
		vector<variavel> particoes_aux;
		while (it_var < 2 * n_var_part && cont1 < particoes_completas.size()) {
			bool add = false;
			if (!adicionado[particoes_est1[cont1].ind_geral]) {
				particoes_aux.push_back(particoes_est1[cont1]);
				add = true;
			}
			cont1++;
			if (add)
				it_var++;
		}
		//S1
		std::stable_sort(particoes_aux.begin(), particoes_aux.end(), [&](variavel i, variavel j) {return i.s < j.s;});
		it_var = 0;
		while (it_var < n_var_part && it_var < particoes_aux.size()) {
			particoes[k].push_back(particoes_aux[it_var]);
			it_var++;
			adicionado[particoes[k].back().ind_geral] = true;
			soma += adicionado[particoes[k].back().ind_geral];
		}

		particoes_aux.end();
	}

	return particoes;
}

vector<vector<variavel>> RF::HRF4_S5_S8(vector<variavel> particoes_completas, int K)
{
	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)particoes_completas.size() / K);

	vector<int> f_i(N, 0);
	for (l = 0; l < M; l++) {
		for (auto sp : SP[l]) {
			f_i[sp]++;
		}
	}
	vector<int> flexibilidade(N, 0);
	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {
				flexibilidade[i]++;
			}
		}
	}

	vector<int> criticidade(M, 0);
	for (l = 0; l < M; l++) {
		int min = INT_MAX;
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i] && f_i[i] < min) {
				min = f_i[i];
			}
		}
		criticidade[l] = M - min;
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	vector<variavel>
		particoes_est1 = particoes_completas,
		particoes_est2 = particoes_completas;
	vector<bool> adicionado(particoes_completas.size(), false);

	int
		cont1 = 0,
		cont2 = 0,
		n_add = 0;

	//S5
	std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return flexibilidade[i.i] > flexibilidade[j.i];});


	particoes = vector<vector<variavel>>(K);
	int soma = 0;
	for (int k = 0; k < K; k++) {
		int it_var = 0,
			cont1 = 0;
		vector<variavel> particoes_aux;
		while (it_var < 2 * n_var_part && cont1 < particoes_completas.size()) {
			bool add = false;
			if (!adicionado[particoes_est1[cont1].ind_geral]) {
				particoes_aux.push_back(particoes_est1[cont1]);
				add = true;
			}
			cont1++;
			if (add)
				it_var++;
		}
		//S1
		std::stable_sort(particoes_aux.begin(), particoes_aux.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});
		it_var = 0;
		while (it_var < n_var_part && it_var < particoes_aux.size()) {
			particoes[k].push_back(particoes_aux[it_var]);
			it_var++;
			adicionado[particoes[k].back().ind_geral] = true;
			soma += adicionado[particoes[k].back().ind_geral];
		}

		particoes_aux.end();
	}

	return particoes;
}

vector<vector<variavel>> RF::HRF4_S8_S1(vector<variavel> particoes_completas, int K)
{
	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)particoes_completas.size() / K);

	vector<int> f_i(N, 0);
	for (l = 0; l < M; l++) {
		for (auto sp : SP[l]) {
			f_i[sp]++;
		}
	}
	vector<int> flexibilidade(N, 0);
	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {
				flexibilidade[i]++;
			}
		}
	}

	vector<int> criticidade(M, 0);
	for (l = 0; l < M; l++) {
		int min = INT_MAX;
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i] && f_i[i] < min) {
				min = f_i[i];
			}
		}
		criticidade[l] = M - min;
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	vector<variavel>
		particoes_est1 = particoes_completas,
		particoes_est2 = particoes_completas;
	vector<bool> adicionado(particoes_completas.size(), false);

	int
		cont1 = 0,
		cont2 = 0,
		n_add = 0;

	//S8
	std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});


	particoes = vector<vector<variavel>>(K);
	int soma = 0;
	for (int k = 0; k < K; k++) {
		int it_var = 0,
			cont1 = 0;
		vector<variavel> particoes_aux;
		while (it_var < 2 * n_var_part && cont1 < particoes_completas.size()) {
			bool add = false;
			if (!adicionado[particoes_est1[cont1].ind_geral]) {
				particoes_aux.push_back(particoes_est1[cont1]);
				add = true;
			}
			cont1++;
			if (add)
				it_var++;
		}
		//S1
		std::stable_sort(particoes_aux.begin(), particoes_aux.end(), [&](variavel i, variavel j) {return i.s < j.s;});
		it_var = 0;
		while (it_var < n_var_part && it_var < particoes_aux.size()) {
			particoes[k].push_back(particoes_aux[it_var]);
			it_var++;
			adicionado[particoes[k].back().ind_geral] = true;
			soma += adicionado[particoes[k].back().ind_geral];
		}

		particoes_aux.end();
	}

	return particoes;
}

vector<vector<variavel>> RF::HRF4_S8_S5(vector<variavel> particoes_completas, int K)
{
	vector<vector<variavel>> particoes;
	IloInt i, l, j, s, t;
	int n_var_part = ceil((double)particoes_completas.size() / K);

	vector<int> f_i(N, 0);
	for (l = 0; l < M; l++) {
		for (auto sp : SP[l]) {
			f_i[sp]++;
		}
	}
	vector<int> flexibilidade(N, 0);
	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			if (l_produz_i[l][i]) {
				flexibilidade[i]++;
			}
		}
	}

	vector<int> criticidade(M, 0);
	for (l = 0; l < M; l++) {
		int min = INT_MAX;
		for (i = 0; i < N; i++) {
			if (l_produz_i[l][i] && f_i[i] < min) {
				min = f_i[i];
			}
		}
		criticidade[l] = M - min;
	}

	std::stable_sort(particoes_completas.begin(), particoes_completas.end(), [&](variavel i, variavel j) {return i.i < j.i;});
	vector<variavel>
		particoes_est1 = particoes_completas,
		particoes_est2 = particoes_completas;
	vector<bool> adicionado(particoes_completas.size(), false);

	int
		cont1 = 0,
		cont2 = 0,
		n_add = 0;

	//S8
	std::stable_sort(particoes_est1.begin(), particoes_est1.end(), [&](variavel i, variavel j) {return criticidade[i.l] > criticidade[j.l];});


	particoes = vector<vector<variavel>>(K);
	int soma = 0;
	for (int k = 0; k < K; k++) {
		int it_var = 0,
			cont1 = 0;
		vector<variavel> particoes_aux;
		while (it_var < 2 * n_var_part && cont1 < particoes_completas.size()) {
			bool add = false;
			if (!adicionado[particoes_est1[cont1].ind_geral]) {
				particoes_aux.push_back(particoes_est1[cont1]);
				add = true;
			}
			cont1++;
			if (add)
				it_var++;
		}
		//S1
		std::stable_sort(particoes_aux.begin(), particoes_aux.end(), [&](variavel i, variavel j) {return flexibilidade[i.i] > flexibilidade[j.i];});
		it_var = 0;
		while (it_var < n_var_part && it_var < particoes_aux.size()) {
			particoes[k].push_back(particoes_aux[it_var]);
			it_var++;
			adicionado[particoes[k].back().ind_geral] = true;
			soma += adicionado[particoes[k].back().ind_geral];
		}

		particoes_aux.end();
	}

	return particoes;
}