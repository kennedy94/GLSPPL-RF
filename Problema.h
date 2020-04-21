#pragma once

//#include <ilcplex/ilocplex.h>
//ILOSTLBEGIN

#include <iostream>
#include <vector>

using namespace std;


class Problema{
protected:
	int
		N,	//Número de produtos
		T,	//Número de períodos
		M,	//Número de máquinas distintas
		W,	//Número de sub-períodos
		CA; //Capacidade de Armazenagem

	vector< vector<int> >
		d,	//Demanda do produto i no período t
		CP,	//Capacidade de produção disponível na máquina l no período t
		cp,	//Custo de produção do produto i na máquina l
		p,	//tempo consumido para produção de uma unidade do produto i na máquina l
		lm,	//lote mínimo do produto i produzido na máquina l
		x0;	//se maquina l esta prepara para produzir o produto i no inicio

	vector<int>
		l0_plus,	//qtde de produto i em estoque no inicio
		l0_minus,	//qtde de produto i com demanda não atendida no inicio
		g,			//custo de atraso de entrega de uma unidade do produto i por período
		h;			//custo de manutenção de uma unidade do produto i em estoque por período

	vector< vector< vector<int> > >
		st, //tempo de setup para produção do produto j imediatamente após o produto i na máquina l
		cs;	//custo de setup para produzir o produto j imediatamente após o produto i na máquina l

public:
	Problema();

	~Problema();
};

