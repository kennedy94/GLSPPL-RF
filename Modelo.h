#pragma once
#include "Problema.h"
#include <numeric>
#include <algorithm>
#include <list>

struct variavel{
	int i, l, s;
	variavel(int i, int l, int s) {
		this->i = i;
		this->l = l;
		this->s = s;
	}

};
class Modelo :
	public Problema
{

private:
	IloModel modelo;
	IloEnv env;
	IloCplex cplex;
	IloExpr OBJETIVO;

	IloArray<IloFloatVarArray> I_plus, I_minus;
	IloArray<IloArray<IloFloatVarArray>> q;
	IloArray<IloArray<IloFloatVarArray>> x;
	IloArray<IloArray<IloArray<IloFloatVarArray>>> y;

	IloNumVar C_setup, C_prod, C_est, C_Bko;

	void criar_modelo();
	void cplexvar_initiate();
	void fo();
	void restricoes();

public:
	Modelo(const char* filename) : Problema(filename) {	}

	void resolver();	//Modelo default
	IloArray<IloArray<IloBoolArray>> RELAX_AND_FIX(int estrategia, int k = -1, bool _fix_opt = false); //Relax-and-fix por máquina com mais produtos criticos

	list<list<variavel>> RF_Pr1(int k, list<vector<variavel>> particoes_completas);	//Relax-and-fix por produto a partir de maior demanda
	list<list<variavel>> RF_Pr2(int k, list<vector<variavel>> particoes_completas);	//Relax-and-fix por produto a partir de menor demanda
	list<list<variavel>> RF_Pr3(int k, list<vector<variavel>> particoes_completas);	//Relax-and-fix por produto a partir de maior criticidade

	list<list<variavel>> RF_Mc1(int k, list<vector<variavel>> particoes_completas);	//Relax-and-fix por máquina mais rápidas e eficientes
	list<list<variavel>> RF_Mc2(int k, list<vector<variavel>> particoes_completas);
	list<list<variavel>> RF_Tm1(int k, list<vector<variavel>> particoes_completas);	//Relax-and-fix por tempo a partir de 0
	list<list<variavel>> RF_Tm2(int k, list<vector<variavel>> particoes_completas);	//Relax-and-fix por tempo a partir de T

	void FIX_AND_OPTIMIZE(IloArray<IloArray<IloBoolArray>> x_hat);

	list<list<variavel>> HRF_Hb2(list<vector<variavel>> particoes_completas,
		int k1, int k2, int estrat1, int estrat2);

	list<list<variavel>> HRF_Hb1(list<vector<variavel>> particoes_completas,
		int k1, int k2, int k3, int estrat1, int estrat2, int estrat3);
};

