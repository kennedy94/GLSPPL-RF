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
			//resultados << "\t" << cplex.getStatus();
			//resultados.close();
			return;
		}
		soltime = cplex.getCplexTime() - soltime;

		ofstream resultados("resultados_modelo.txt", fstream::app);
		resultados << "," << cplex.getObjValue() << "," << cplex.getMIPRelativeGap() <<
			"," << soltime << "," << cplex.getStatus();
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
			"," << soltime << "," << cplex.getStatus();
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
			"," << soltime << "," << cplex.getStatus();
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