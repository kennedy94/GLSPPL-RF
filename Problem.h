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
		N,	//Número de produtos
		T,	//Número de períodos
		M,	//Número de máquinas distintas
		W,	//Número de sub-períodos
		CA, //Capacidade de Armazenagem
		W_p; //subperiodos por período
	vector<vector<bool>> l_produz_i;


	vector<vector<int>>
		SP,
		d,	//Demanda do produto i no período t
		CP,	//Capacidade de produção disponível na máquina l no período t
		lm;	//lote mínimo do produto i produzido na máquina l

	vector< vector<double> >
		AUX,
		cp,	//Custo de produção do produto i na máquina l
		p;	//tempo consumido para produção de uma unidade do produto i na máquina l


	vector<int>
		I0_plus,	//qtde de produto i em estoque no inicio
		I0_minus;	//qtde de produto i com demanda não atendida no inicio
	vector<double>
		g,			//custo de atraso de entrega de uma unidade do produto i por período
		h;			//custo de manutenção de uma unidade do produto i em estoque por período

	vector< vector< vector<double> > >
		st, //tempo de setup para produção do produto j imediatamente após o produto i na máquina l
		cs;	//custo de setup para produzir o produto j imediatamente após o produto i na máquina l

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

