#pragma once
#include "Problema.h"
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
	IloArray<IloArray<IloBoolVarArray>> x;
	IloArray<IloArray<IloArray<IloFloatVarArray>>> y;

	IloNumVar C_setup, C_prod, C_est, C_Bko;

	void criar_modelo();
	void cplexvar_initiate();
	void fo();
	void restricoes();

public:
	Modelo(const char* filename) : Problema(filename) {

	}

	void resolver();

};

