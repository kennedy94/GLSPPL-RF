#pragma once
#include "Problema.h"
#include <numeric>
#include <algorithm>
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
	Modelo(const char* filename) : Problema(filename) {

	}

	void resolver();	//Modelo default

	void RF_Tm1(int k);	//Relax-and-fix por tempo a partir de 0
	void RF_Tm2(int k);	//Relax-and-fix por tempo a partir de T

	void RF_Pr1(int k);	//Relax-and-fix por produto a partir de maior demanda
	void RF_Pr2(int k);	//Relax-and-fix por produto a partir de menor demanda
	void RF_Pr3(int k);	//Relax-and-fix por produto a partir de maior criticidade

	void RF_Mc1(int k);	//Relax-and-fix por máquina mais rápidas e eficientes
	void RF_Mc2(int k); //Relax-and-fix por máquina com mais produtos criticos

};

