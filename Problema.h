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

	


protected:
	int
		N,	//N�mero de produtos
		T,	//N�mero de per�odos
		M,	//N�mero de m�quinas distintas
		W,	//N�mero de sub-per�odos
		CA, //Capacidade de Armazenagem
		W_p; //subperiodos por per�odo
	vector<vector<bool>> l_produz_i;

	vector<vector<int>>
		SP,
		d,	//Demanda do produto i no per�odo t
		CP,	//Capacidade de produ��o dispon�vel na m�quina l no per�odo t
		lm;	//lote m�nimo do produto i produzido na m�quina l

	vector< vector<double> >
		AUX,
		cp,	//Custo de produ��o do produto i na m�quina l
		p;	//tempo consumido para produ��o de uma unidade do produto i na m�quina l


	vector<int>
		I0_plus,	//qtde de produto i em estoque no inicio
		I0_minus;	//qtde de produto i com demanda n�o atendida no inicio
	vector<double>
		g,			//custo de atraso de entrega de uma unidade do produto i por per�odo
		h;			//custo de manuten��o de uma unidade do produto i em estoque por per�odo

	vector< vector< vector<double> > >
		st, //tempo de setup para produ��o do produto j imediatamente ap�s o produto i na m�quina l
		cs;	//custo de setup para produzir o produto j imediatamente ap�s o produto i na m�quina l

	const char* instancia;

public:
	Problema(const char* nome);

	~Problema();
};

