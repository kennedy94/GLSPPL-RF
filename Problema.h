#pragma once

#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;


class Problema{
private:
	IloModel modelo;
	IloEnv env;
	IloCplex cplex;
	IloExpr OBJETIVO;

	IloArray<IloIntVarArray> I_plus, I_minus;
	IloArray<IloArray<IloIntVarArray>> q;
	IloArray<IloArray<IloBoolVarArray>> x;
	IloArray<IloArray<IloArray<IloBoolVarArray>>> y;
	


protected:
	int
		N,	//N�mero de produtos
		T,	//N�mero de per�odos
		M,	//N�mero de m�quinas distintas
		W,	//N�mero de sub-per�odos
		CA, //Capacidade de Armazenagem
		W_p; //subperiodos por per�odo

	vector< vector<float> >
		AUX,
		SP,
		d,	//Demanda do produto i no per�odo t
		CP,	//Capacidade de produ��o dispon�vel na m�quina l no per�odo t
		cp,	//Custo de produ��o do produto i na m�quina l
		p,	//tempo consumido para produ��o de uma unidade do produto i na m�quina l
		lm,	//lote m�nimo do produto i produzido na m�quina l
		x0;	//se maquina l esta prepara para produzir o produto i no inicio

	vector<float>
		I0_plus,	//qtde de produto i em estoque no inicio
		I0_minus,	//qtde de produto i com demanda n�o atendida no inicio
		g,			//custo de atraso de entrega de uma unidade do produto i por per�odo
		h;			//custo de manuten��o de uma unidade do produto i em estoque por per�odo

	vector< vector< vector<float> > >
		st, //tempo de setup para produ��o do produto j imediatamente ap�s o produto i na m�quina l
		cs;	//custo de setup para produzir o produto j imediatamente ap�s o produto i na m�quina l

	void criar_modelo();
	void cplexvar_initiate();
	void fo();
	void restricoes();
public:
	Problema(const char* nome);

	void resolver();

	~Problema();
};

