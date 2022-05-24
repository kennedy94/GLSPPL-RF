#pragma once

#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

struct variavel {
	int i, l, s, t, ind_geral, ranking;
	double influ, dist, valor_;
	variavel(int i, int l, int s, int t) {
		this->i = i;
		this->l = l;
		this->s = s;
		this->t = t;
		influ = 0.0;
		ind_geral = -1;
		ranking = 0;
	}
	variavel(int i, int l, int s, int t, int ind_geral) {
		this->i = i;
		this->l = l;
		this->s = s;
		this->t = t;
		this->ind_geral = ind_geral;
		influ = 0.0;
		ranking = 0;
	}

};

class Problem{
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
	Problem(const char* nome);


	~Problem() {
		I0_minus.end();
		I0_minus.end();
		g.end();
		h.end();
		for (auto &i: st)
		{
			i.end();
		}
		st.end();

		for (auto& i : cs)
		{
			i.end();
		}
		cs.end();
		for (auto& i : l_produz_i)
		{
			i.end();
		}
		l_produz_i.end();


		for (auto& i : AUX)
			i.end();
		AUX.end();

		for (auto& i : cp)
			i.end();
		cp.end();

		for (auto& i : p)
			i.end();
		p.end();

		for (auto& i : SP)
			i.end();
		SP.end();


		for (auto& i : d)
			i.end();
		d.end();

		for (auto& i : CP)
			i.end();
		CP.end();


		for (auto& i : lm)
			i.end();
		lm.end();
	}
};

