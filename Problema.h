#pragma once

//#include <ilcplex/ilocplex.h>
//ILOSTLBEGIN

#include <iostream>
#include <vector>

using namespace std;


class Problema{
protected:
	int
		N,	//N�mero de produtos
		T,	//N�mero de per�odos
		M,	//N�mero de m�quinas distintas
		W,	//N�mero de sub-per�odos
		CA; //Capacidade de Armazenagem

	vector< vector<int> >
		d,	//Demanda do produto i no per�odo t
		CP,	//Capacidade de produ��o dispon�vel na m�quina l no per�odo t
		cp,	//Custo de produ��o do produto i na m�quina l
		p,	//tempo consumido para produ��o de uma unidade do produto i na m�quina l
		lm,	//lote m�nimo do produto i produzido na m�quina l
		x0;	//se maquina l esta prepara para produzir o produto i no inicio

	vector<int>
		l0_plus,	//qtde de produto i em estoque no inicio
		l0_minus,	//qtde de produto i com demanda n�o atendida no inicio
		g,			//custo de atraso de entrega de uma unidade do produto i por per�odo
		h;			//custo de manuten��o de uma unidade do produto i em estoque por per�odo

	vector< vector< vector<int> > >
		st, //tempo de setup para produ��o do produto j imediatamente ap�s o produto i na m�quina l
		cs;	//custo de setup para produzir o produto j imediatamente ap�s o produto i na m�quina l

public:
	Problema();

	~Problema();
};

