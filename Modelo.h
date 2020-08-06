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
	void RELAX_AND_FIX(int k, int estrategia); //Relax-and-fix por m�quina com mais produtos criticos

	void RF_Pr1(int k);	//Relax-and-fix por produto a partir de maior demanda
	void RF_Pr2(int k);	//Relax-and-fix por produto a partir de menor demanda
	void RF_Pr3(int k);	//Relax-and-fix por produto a partir de maior criticidade

	list<list<variavel>> RF_Mc1(int k);	//Relax-and-fix por m�quina mais r�pidas e eficientes
	list<list<variavel>> RF_Mc2(int k);
	list<list<variavel>> RF_Tm1(int k);	//Relax-and-fix por tempo a partir de 0
	list<list<variavel>> RF_Tm2(int k);	//Relax-and-fix por tempo a partir de T
};

