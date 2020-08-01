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
		N,	//Número de produtos
		T,	//Número de períodos
		M,	//Número de máquinas distintas
		W,	//Número de sub-períodos
		CA, //Capacidade de Armazenagem
		W_p; //subperiodos por período

	vector< vector<float> >
		AUX,
		SP,
		d,	//Demanda do produto i no período t
		CP,	//Capacidade de produção disponível na máquina l no período t
		cp,	//Custo de produção do produto i na máquina l
		p,	//tempo consumido para produção de uma unidade do produto i na máquina l
		lm,	//lote mínimo do produto i produzido na máquina l
		x0;	//se maquina l esta prepara para produzir o produto i no inicio

	vector<float>
		I0_plus,	//qtde de produto i em estoque no inicio
		I0_minus,	//qtde de produto i com demanda não atendida no inicio
		g,			//custo de atraso de entrega de uma unidade do produto i por período
		h;			//custo de manutenção de uma unidade do produto i em estoque por período

	vector< vector< vector<float> > >
		st, //tempo de setup para produção do produto j imediatamente após o produto i na máquina l
		cs;	//custo de setup para produzir o produto j imediatamente após o produto i na máquina l

	void criar_modelo();
	void cplexvar_initiate();
	void fo();
	void restricoes();
public:
	Problema(const char* nome);

	void resolver();

	~Problema();
};

