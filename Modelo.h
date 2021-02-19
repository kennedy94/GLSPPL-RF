#pragma once
#include "Problema.h"
#include <numeric>
#include <algorithm>
#include <list>
#include <vector>



struct variavel{
	int i, l, s, t, influ;
	variavel(int i, int l, int s, int t) {
		this->i = i;
		this->l = l;
		this->s = s;
		this->t = t;
		influ = 0;
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

	IloNum tempo_incumbent;

	int nconstraints = 0, nvar = 0;

	void criar_modelo();
	void cplexvar_initiate();
	void fo();
	void restricoes();

public:
	Modelo(const char* filename) : Problema(filename) {	}


	void
		resolver(),	//Modelo default
		resolver_linear(),
		RELAX_AND_FIX(int estrategia, string saida, int k = -1, bool _fix_opt = false); //Relax-and-fix por máquina com mais produtos criticos

	list<list<variavel>>
		RF_Pr1(int k, list<vector<variavel>> particoes_completas),	//Relax-and-fix por produto a partir de maior demanda
		RF_Pr2(int k, list<vector<variavel>> particoes_completas),	//Relax-and-fix por produto a partir de menor demanda
		RF_Pr3(int k, list<vector<variavel>> particoes_completas),	//Relax-and-fix por produto a partir de maior criticidade

		RF_Mc1(int k, list<vector<variavel>> particoes_completas),	//Relax-and-fix por máquina mais rápidas e eficientes
		RF_Mc2(int k, list<vector<variavel>> particoes_completas),
		RF_Tm1(int k, list<vector<variavel>> particoes_completas),	//Relax-and-fix por tempo a partir de 0
		RF_Tm2(int k, list<vector<variavel>> particoes_completas);	//Relax-and-fix por tempo a partir de T

	
	list<list<variavel>>
		RF_Hb2(list<vector<variavel>> particoes_completas, int k1, int k2, int estrat1, int estrat2),
		
		RF_Hb1(list<vector<variavel>> particoes_completas, int k1, int k2, int k3, int estrat1, int estrat2, int estrat3);

	void 
		RF_Hb2_Drt(list<vector<variavel>> particoes_completas, int k1, int k2, int estrat1, int estrat2),
		RF_Pr2_Drt(list<vector<variavel>> particoes_completas, int k),
		FIX_AND_OPTIMIZE(vector<vector<vector<bool>>>  x_hat);

	void RELAX_AND_FIX_Estrat1(int k);

	list<list<variavel>> RF_S2(list<vector<variavel>> particoes_completas);

	list<list<variavel>> RF_S3(list<vector<variavel>> particoes_completas);

	list<list<variavel>> RF_S1(list<vector<variavel>> particoes_completas);

	list<list<variavel>> RF_S4(list<vector<variavel>> particoes_completas, int K);

	list<list<variavel>> RF_S5(list<vector<variavel>> particoes_completas);

	list<list<variavel>> RF_S6(list<vector<variavel>> particoes_completas);

	list<list<variavel>> RF_S7(list<vector<variavel>> particoes_completas);

	list<list<variavel>> RF_S8(list<vector<variavel>> particoes_completas);

	list<list<variavel>> RF_S10(list<vector<variavel>> particoes_completas, int k);


	bool teste_de_viabilidade();
	
};

