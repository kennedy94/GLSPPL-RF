#pragma once
#include "Problema.h"
#include <numeric>
#include <algorithm>
#include <list>
#include <vector>
#include <chrono>
#define episilon 10e-8



class RF_RUIM :
	public Problema
{
public:
	RF_RUIM(const char* filename) : Problema(filename) {	};

	//vector<vector<variavel>> RF_S2(vector<variavel> particoes_completas);

	//vector<vector<variavel>> RF_S3(vector<variavel>  particoes_completas);

	//vector<vector<variavel>> RF_S1(vector<variavel>  particoes_completas);

	vector<vector<variavel>> RF_S4(vector<variavel>  particoes_completas, int K);

	//vector<vector<variavel>> RF_S5(vector<variavel>  particoes_completas, int K);

	//vector<vector<variavel>> RF_S6(vector<variavel>  particoes_completas);

	//vector<vector<variavel>> RF_S7(vector<variavel>  particoes_completas);

	vector<vector<variavel>> RF_S8(vector<variavel>  particoes_completas);

	//vector<vector<variavel>> RF_S10(vector<variavel> particoes_completas, int K);

	void RELAX_AND_FIX(int estrategia, const char* saida, int k = -1, double BUDGET = 3600.0); //Relax-and-fix por máquina com mais produtos criticos
	bool teste_de_viabilidade(IloCplex cplex, IloArray<IloFloatVarArray> I_plus, IloArray<IloFloatVarArray> I_minus,
		IloArray<IloArray<IloFloatVarArray>> q, IloArray<IloArray<IloArray<IloNumVar>>> x,
		IloArray<IloArray<IloArray<IloFloatVarArray>>> y);
};

