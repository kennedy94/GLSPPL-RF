#include "Modelo.h"

void Modelo::resolver(){
	criar_modelo();
	

	//cplex.setParam(IloCplex::TiLim, 3600);
	try {
		//desalocar_matrizes();
		criar_modelo();
		//restrições de integralidade
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
		//cplex.setParam(IloCplex::Param::Emphasis::Numerical, 1);
		//cplex.setParam(IloCplex::NodeFileInd, 1);
		IloNum soltime;
		soltime = cplex.getCplexTime();
		if (!cplex.solve()) {
			env.error() << "Otimizacao do LP mal-sucedida." << endl;
			//resultados.open("resultados_MCCP.txt", fstream::app);
			//resultados << "\t" << cplex.getStatus() << endl;
			//resultados.close();
			return;
		}
		soltime = cplex.getCplexTime() - soltime;

		ofstream resultados("resultados_modelo.txt", fstream::app);
		resultados << "," << cplex.getObjValue() << "," << cplex.getMIPRelativeGap() <<
			"," << soltime << "," << cplex.getStatus() << endl;
		resultados.close();


		//cout << cplex.getValue(C_setup) << endl;
		//cout << cplex.getValue(C_prod) << endl;
		//cout << cplex.getValue(C_est) << endl;
		//cout << cplex.getValue(C_Bko) << endl;

		for (int i = 0; i < N; i++) {
			for (int l = 0; l < M; l++) {
				for (int s = 1; s < W; s++) {
					if (cplex.getValue(q[i][l][s]) != 0) {
						cout << i << " " << l << " " << s << "=" << cplex.getValue(q[i][l][s]) << endl;
					}

				}
			}
		}

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

void Modelo::RF_Tm1(int k){

	IloInt i, l, s, t;
	int n_por_particao = (W - 1) / k;

	try {
		criar_modelo();
		cplex = IloCplex(modelo);

		IloNum soltime;
		soltime = cplex.getCplexTime();

		for (t = 0; t < T - 1; t++) {

			cout << "maior s ==>" << n_por_particao * (t + 1) << endl;
			//restrições de integralidade
			for (i = 0; i < N; i++) {
				for (l = 0; l < M; l++) {
					for (s = n_por_particao * t + 1; s <= n_por_particao * (t + 1); s++) {
						modelo.add(IloConversion(env, x[i][l][s], ILOBOOL));
					}
				}
			}

			//cplex = IloCplex(modelo);
			cplex.solve();
			
			if (t == T - 2)
				break;


			//fixar variáveis
			IloConstraintArray restricoes(env);
			for (i = 0; i < N; i++) {
				for (l = 0; l < M; l++) {
					IloNumArray vals(env);

					for (s = n_por_particao * t + 1; s <= n_por_particao * (t + 1); s++) {
						if ( cplex.isExtracted(x[i][l][s]) ) {
							IloNum val = round(cplex.getValue(x[i][l][s]));
							restricoes.add(IloConstraint(val == x[i][l][s]));
						}
					}
				}
			}
			modelo.add(restricoes);

			//cplex.exportModel("M1.lp");
		}
		soltime = cplex.getCplexTime() - soltime;
		
		ofstream resultados("resultado_RF_Tm1.csv", fstream::app);
		resultados << "," << cplex.getObjValue() << "," << cplex.getMIPRelativeGap() <<
			"," << soltime << "," << cplex.getStatus() << endl;
		resultados.close();

	}
	catch (IloException& e) {
		cplex.error() << "Erro: " << e.getMessage() << endl;
		cout << "\nErro na inteira" << endl;
		return;
	}
	catch (...) {
		cerr << "Outra excecao" << endl;
	}

}

void Modelo::RF_Tm2(int k){
	IloInt i, l, s, t;
	int n_por_particao = (W - 1) / k;
	try {
		criar_modelo();
		cplex = IloCplex(modelo);

		IloNum soltime;
		soltime = cplex.getCplexTime();

		for (t = T-2; t > 0; t--) {

			cout << "maior s ==>" << W_p * (t + 1) << endl;
			//restrições de integralidade
			for (i = 0; i < N; i++) {
				for (l = 0; l < M; l++) {
					for (s = n_por_particao * t + 1; s <= n_por_particao * (t + 1); s++) {
						modelo.add(IloConversion(env, x[i][l][s], ILOBOOL));
					}
				}
			}

			//cplex = IloCplex(modelo);
			cplex.solve();

			if (t == 1)
				break;


			//fixar variáveis
			IloConstraintArray restricoes(env);
			for (i = 0; i < N; i++) {
				for (l = 0; l < M; l++) {
					IloNumArray vals(env);

					for (s = n_por_particao * t + 1; s <= n_por_particao * (t + 1); s++) {
						if (cplex.isExtracted(x[i][l][s])) {
							IloNum val = round(cplex.getValue(x[i][l][s]));
							restricoes.add(IloConstraint(val == x[i][l][s]));
						}
					}
				}
			}
			modelo.add(restricoes);

			//cplex.exportModel("M1.lp");
		}
		soltime = cplex.getCplexTime() - soltime;

		ofstream resultados("resultado_RF_Tm2.csv", fstream::app);
		resultados << "," << cplex.getObjValue() << "," << cplex.getMIPRelativeGap() <<
			"," << soltime << "," << cplex.getStatus() << endl;
		resultados.close();

	}
	catch (IloException& e) {
		cplex.error() << "Erro: " << e.getMessage() << endl;
		cout << "\nErro na inteira" << endl;
		return;
	}
	catch (...) {
		cerr << "Outra excecao" << endl;
	}

}

void Modelo::RF_Pr1(int k) {

		IloInt i, l, s, particao_it, t;
		int n_por_particao = N / k;
		try {
			criar_modelo();
			cplex = IloCplex(modelo);
			cplex.setParam(IloCplex::TiLim, 600/k);

			IloNum soltime;
			soltime = cplex.getCplexTime();

			/*
			ORDENAR OS PRODUTOS POR DEMANDA EM ORDEM DECRESCENTE
			*/

			vector<int> demanda(N, 0);
			for (i = 0; i < N; i++)	{
				for (t = 1; t < T; t++)	{
					demanda[i] += d[i][t];
				}
			}
			vector<int> id(N);
			i = 0;
			iota(id.begin(), id.end(), i++); //Initializing
			sort(id.begin(), id.end(), [&](int i, int j) {return demanda[i] > demanda[j];});


			particao_it = 0;
			while(true){
				//restrições de integralidade
				for (i = particao_it; i < particao_it + n_por_particao; i++) {
					for (l = 0; l < M; l++) {
						for (s = 1; s < W; s++) {
							modelo.add(IloConversion(env, x[id[i]][l][s], ILOBOOL));
						}
					}
				}

				//cplex = IloCplex(modelo);
				cplex.solve();

				if (particao_it + n_por_particao == N)
					break;


				//fixar variáveis
				IloConstraintArray restricoes(env);
				for (i = particao_it; i < particao_it + n_por_particao; i++) {
					for (l = 0; l < M; l++) {
						IloNumArray vals(env);

						for (s = 1; s < W; s++) {
							if (cplex.isExtracted(x[id[i]][l][s])) {
								IloNum val = round(cplex.getValue(x[id[i]][l][s]));
								restricoes.add(IloConstraint(val == x[id[i]][l][s]));
							}
						}
					}
				}
				modelo.add(restricoes);

				//cplex.exportModel("M1.lp");
				particao_it += n_por_particao;
			}
			soltime = cplex.getCplexTime() - soltime;

			ofstream resultados("resultado_RF_Pr1.csv", fstream::app);
			resultados << "," << cplex.getObjValue() << "," << cplex.getMIPRelativeGap() <<
				"," << soltime << "," << cplex.getStatus() << endl;
			resultados.close();

		}
		catch (IloException& e) {
			cplex.error() << "Erro: " << e.getMessage() << endl;
			cout << "\nErro na inteira" << endl;
			return;
		}
		catch (...) {
			cerr << "Outra excecao" << endl;
		}
}

void Modelo::RF_Pr2(int k) {

	IloInt i, l, s, particao_it, t;
	int n_por_particao = N / k;
	try {
		criar_modelo();
		cplex = IloCplex(modelo);
		cplex.setParam(IloCplex::TiLim, 600 / k);

		IloNum soltime;
		soltime = cplex.getCplexTime();

		/*
		ORDENAR OS PRODUTOS POR DEMANDA EM ORDEM DECRESCENTE
		*/

		vector<int> demanda(N, 0);
		for (i = 0; i < N; i++) {
			for (t = 1; t < T; t++) {
				demanda[i] += d[i][t];
			}
		}
		vector<int> id(N);
		i = 0;
		iota(id.begin(), id.end(), i++); //Initializing
		sort(id.begin(), id.end(), [&](int i, int j) {return demanda[i] < demanda[j];});


		particao_it = 0;
		while (true) {
			//restrições de integralidade
			for (i = particao_it; i < particao_it + n_por_particao; i++) {
				for (l = 0; l < M; l++) {
					for (s = 1; s < W; s++) {
						modelo.add(IloConversion(env, x[id[i]][l][s], ILOBOOL));
					}
				}
			}

			//cplex = IloCplex(modelo);
			cplex.solve();

			if (particao_it + n_por_particao == N)
				break;


			//fixar variáveis
			IloConstraintArray restricoes(env);
			for (i = particao_it; i < particao_it + n_por_particao; i++) {
				for (l = 0; l < M; l++) {
					IloNumArray vals(env);

					for (s = 1; s < W; s++) {
						if (cplex.isExtracted(x[id[i]][l][s])) {
							IloNum val = round(cplex.getValue(x[id[i]][l][s]));
							restricoes.add(IloConstraint(val == x[id[i]][l][s]));
						}
					}
				}
			}
			modelo.add(restricoes);

			//cplex.exportModel("M1.lp");
			particao_it += n_por_particao;
		}
		soltime = cplex.getCplexTime() - soltime;

		ofstream resultados("resultado_RF_Pr2.csv", fstream::app);
		resultados << "," << cplex.getObjValue() << "," << cplex.getMIPRelativeGap() <<
			"," << soltime << "," << cplex.getStatus() << endl;
		resultados.close();

	}
	catch (IloException& e) {
		cplex.error() << "Erro: " << e.getMessage() << endl;
		cout << "\nErro na inteira" << endl;
		return;
	}
	catch (...) {
		cerr << "Outra excecao" << endl;
	}
}

void Modelo::RF_Pr3(int k) {

	IloInt i, l, s, particao_it, t;
	int n_por_particao = N / k;
	try {
		criar_modelo();
		cplex = IloCplex(modelo);
		cplex.setParam(IloCplex::TiLim, 600 / k);

		IloNum soltime;
		soltime = cplex.getCplexTime();

		/*
		ORDENAR OS PRODUTOS POR DEMANDA EM ORDEM DECRESCENTE
		*/

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
		vector<int> id(N);
		i = 0;
		iota(id.begin(), id.end(), i++); //Initializing
		sort(id.begin(), id.end(), [&](int i, int j) {return criticidade[i] > criticidade[j];});


		particao_it = 0;
		while (true) {
			//restrições de integralidade
			for (i = particao_it; i < particao_it + n_por_particao; i++) {
				for (l = 0; l < M; l++) {
					for (s = 1; s < W; s++) {
						modelo.add(IloConversion(env, x[id[i]][l][s], ILOBOOL));
					}
				}
			}

			//cplex = IloCplex(modelo);
			cplex.solve();

			if (particao_it + n_por_particao == N)
				break;


			//fixar variáveis
			IloConstraintArray restricoes(env);
			for (i = particao_it; i < particao_it + n_por_particao; i++) {
				for (l = 0; l < M; l++) {
					IloNumArray vals(env);

					for (s = 1; s < W; s++) {
						if (cplex.isExtracted(x[id[i]][l][s])) {
							IloNum val = round(cplex.getValue(x[id[i]][l][s]));
							restricoes.add(IloConstraint(val == x[id[i]][l][s]));
						}
					}
				}
			}
			modelo.add(restricoes);

			//cplex.exportModel("M1.lp");
			particao_it += n_por_particao;
		}
		soltime = cplex.getCplexTime() - soltime;

		ofstream resultados("resultado_RF_Pr3.csv", fstream::app);
		resultados << "," << cplex.getObjValue() << "," << cplex.getMIPRelativeGap() <<
			"," << soltime << "," << cplex.getStatus() << endl;
		resultados.close();

	}
	catch (IloException& e) {
		cplex.error() << "Erro: " << e.getMessage() << endl;
		cout << "\nErro na inteira" << endl;
		return;
	}
	catch (...) {
		cerr << "Outra excecao" << endl;
	}
}

void Modelo::RF_Mc1(int k) {

	IloInt i, l, s, particao_it, t;
	int n_por_particao = M / k;
	try {
		criar_modelo();
		cplex = IloCplex(modelo);
		cplex.setParam(IloCplex::TiLim, 600 / k);

		IloNum soltime;
		soltime = cplex.getCplexTime();

		/*
		ORDENAR OS PRODUTOS POR DEMANDA EM ORDEM DECRESCENTE
		*/

		vector<float> eficiencia(M, 0);

		for (l = 0; l < M; l++) {
			for (auto &prod : SP[l])	{
				eficiencia[l] += p[prod][l];
			}
			eficiencia[l] /= SP[l].size();
		}

		vector<int> id(M);
		l = 0;
		iota(id.begin(), id.end(), l++); //Initializing
		sort(id.begin(), id.end(), [&](int i, int j) {return eficiencia[i] > eficiencia[j];});


		particao_it = 0;
		while (true) {
			//restrições de integralidade
			for (i = 0; i < N; i++) {
				for (l = particao_it; l < particao_it + n_por_particao; l++) {
					for (s = 1; s < W; s++) {
						modelo.add(IloConversion(env, x[i][id[l]][s], ILOBOOL));
					}
				}
			}

			//cplex = IloCplex(modelo);
			cplex.solve();

			if (particao_it + n_por_particao == M)
				break;


			//fixar variáveis
			IloConstraintArray restricoes(env);
			for (i = 0; i < N; i++) {
				for (l = particao_it; l < particao_it + n_por_particao; l++) {
					IloNumArray vals(env);

					for (s = 1; s < W; s++) {
						if (cplex.isExtracted(x[i][id[l]][s])) {
							IloNum val = round(cplex.getValue(x[i][id[l]][s]));
							restricoes.add(IloConstraint(val == x[i][id[l]][s]));
						}
					}
				}
			}
			modelo.add(restricoes);

			//cplex.exportModel("M1.lp");
			particao_it += n_por_particao;
		}
		soltime = cplex.getCplexTime() - soltime;

		ofstream resultados("resultado_RF_Mc1.csv", fstream::app);
		resultados << "," << cplex.getObjValue() << "," << cplex.getMIPRelativeGap() <<
			"," << soltime << "," << cplex.getStatus() << endl;
		resultados.close();

	}
	catch (IloException& e) {
		cplex.error() << "Erro: " << e.getMessage() << endl;
		cout << "\nErro na inteira" << endl;
		return;
	}
	catch (...) {
		cerr << "Outra excecao" << endl;
	}
}


void Modelo::RF_Mc2(int k) {

	IloInt i, l, s, particao_it, t;
	int n_por_particao = M / k;
	try {
		criar_modelo();
		cplex = IloCplex(modelo);
		cplex.setParam(IloCplex::TiLim, 600 / k);

		IloNum soltime;
		soltime = cplex.getCplexTime();

		/*
		ORDENAR OS PRODUTOS POR DEMANDA EM ORDEM DECRESCENTE
		*/

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

		vector<int> id(M);
		l = 0;
		iota(id.begin(), id.end(), l++); //Initializing
		sort(id.begin(), id.end(), [&](int i, int j) {return criticidade_maquina[i] > criticidade_maquina[j];});


		particao_it = 0;
		while (true) {
			//restrições de integralidade
			for (i = 0; i < N; i++) {
				for (l = particao_it; l < particao_it + n_por_particao; l++) {
					for (s = 1; s < W; s++) {
						modelo.add(IloConversion(env, x[i][id[l]][s], ILOBOOL));
					}
				}
			}

			//cplex = IloCplex(modelo);
			cplex.solve();

			if (particao_it + n_por_particao == M)
				break;


			//fixar variáveis
			IloConstraintArray restricoes(env);
			for (i = 0; i < N; i++) {
				for (l = particao_it; l < particao_it + n_por_particao; l++) {
					IloNumArray vals(env);

					for (s = 1; s < W; s++) {
						if (cplex.isExtracted(x[i][id[l]][s])) {
							IloNum val = round(cplex.getValue(x[i][id[l]][s]));
							restricoes.add(IloConstraint(val == x[i][id[l]][s]));
						}
					}
				}
			}
			modelo.add(restricoes);

			//cplex.exportModel("M1.lp");
			particao_it += n_por_particao;
		}
		soltime = cplex.getCplexTime() - soltime;

		ofstream resultados("resultado_RF_Mc1.csv", fstream::app);
		resultados << "," << cplex.getObjValue() << "," << cplex.getMIPRelativeGap() <<
			"," << soltime << "," << cplex.getStatus() << endl;
		resultados.close();

	}
	catch (IloException& e) {
		cplex.error() << "Erro: " << e.getMessage() << endl;
		cout << "\nErro na inteira" << endl;
		return;
	}
	catch (...) {
		cerr << "Outra excecao" << endl;
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