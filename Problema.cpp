#include "Problema.h"

Problema::Problema(const char* nome){
	ifstream arquivo(nome);
	int i, j, l, t;

	//inteiros
	arquivo >> N >> T >> W >> M >> CA;

	W_p = W / T;

	W++;
	T++;

	//conjunto de produtos por máquina
	vector<string> vec_string;
	string str;
	l = 0;
	while (l < M){
		getline(arquivo, str);
		if (str.size() > 0) {
			vec_string.push_back(str);
			l++;
		}
	}
	for (auto stg: vec_string) 
	{
		std::stringstream iss(stg);
		float number;
		std::vector<float> myNumbers;
		while (iss >> number) {
			number--;
			myNumbers.push_back(number);
		}
		SP.push_back(myNumbers);
	}

	//lote mínimo
	l = 0;
	vec_string.clear();
	while (l < M) {
		getline(arquivo, str);
		if (str.size() > 0) {
			vec_string.push_back(str);
			l++;
		}
	}
	for (auto stg : vec_string)
	{
		std::stringstream iss(stg);
		float number;
		std::vector<float> myNumbers;
		while (iss >> number) {
			myNumbers.push_back(number);
		}
		AUX.push_back(myNumbers);
	}

	lm = vector< vector<float> >(N);
	for (int i = 0; i < N; i++)	{
		lm[i] = vector<float>(M, 0.0);
	}
	for (int l = 0; l < M; l++){
		int contador = 0;
		for (auto i: SP[l])
		{
			lm[i][l] = AUX[l][contador];
			contador++;
		}
	}
	AUX.clear();



	//capacidade de produção
	CP = vector< vector<float>>(M);
	for (int l = 0; l < M; l++) {
		CP[l] = vector<float>(T);
		for (int t = 1; t < T; t++) {
			arquivo >> CP[l][t];
		}
	}

	//tempo de processamento
	l = 0;
	vec_string.clear();
	while (l < M) {
		getline(arquivo, str);
		if (str.size() > 0) {
			vec_string.push_back(str);
			l++;
		}
	}
	for (auto stg : vec_string)
	{
		std::stringstream iss(stg);
		float number;
		std::vector<float> myNumbers;
		while (iss >> number) {
			myNumbers.push_back(number);
		}
		AUX.push_back(myNumbers);
	}

	p = vector< vector<float> >(N);
	for (int i = 0; i < N; i++) {
		p[i] = vector<float>(M, 0.0);
	}
	for (int l = 0; l < M; l++) {
		int contador = 0;
		for (auto i : SP[l])
		{
			p[i][l] = AUX[l][contador];
			contador++;
		}
	}
	AUX.clear();


	//Estoques iniciais
	I0_plus = vector<float>(N);
	I0_minus = vector<float>(N);
	for (i = 0; i < N; i++){
		arquivo >> I0_plus[i];
	}
	//backorder iniciais
	for (i = 0; i < N; i++) {
		arquivo >> I0_minus[i];
	}

	//demanda
	d = vector<vector<float>>(N);
	for (i = 0; i < N; i++){
		d[i] = vector<float>(T);
		for (t = 1; t < T; t++){
			arquivo >> d[i][t];
		}
	}


	//setup time
	st = vector<vector<vector<float>>>(N);
	for (i = 0; i < N; i++){
		st[i] = vector< vector< float>>(N);
		for (j = 0; j < N; j++) {
			st[i][j] =  vector< float>(M);
		}
	}

	for (l = 0; l < M; l++){	
		for (auto i : SP[l]){
			for (auto j : SP[l]){
				arquivo >> st[i][j][l];
			}
		}
	}

	//custos

	//custo estoque
	h = vector<float>(N);
	for (i = 0; i < N; i++){
		arquivo >> h[i];
	}

	//custo backorder
	g = vector<float>(N);
	for (i = 0; i < N; i++) {
		arquivo >> g[i];
	}


	//custo de produção dos produtos
	//CUIDADO, ESTÁ TRANSPOSTO
	l = 0;
	vec_string.clear();
	while (l < M) {
		getline(arquivo, str);
		if (str.size() > 0) {
			vec_string.push_back(str);
			l++;
		}
	}
	for (auto stg : vec_string)
	{
		std::stringstream iss(stg);
		float number;
		std::vector<float> myNumbers;
		while (iss >> number) {
			myNumbers.push_back(number);
		}
		AUX.push_back(myNumbers);
	}

	cp = vector< vector<float> >(N);
	for (int i = 0; i < N; i++) {
		cp[i] = vector<float>(M, 0.0);
	}
	for (int l = 0; l < M; l++) {
		int contador = 0;
		for (auto i : SP[l])
		{
			cp[i][l] = AUX[l][contador];
			contador++;
		}
	}
	AUX.clear();

	//custo setup
	cs = vector<vector<vector<float>>>(N);
	for (i = 0; i < N; i++) {
		cs[i] = vector< vector< float>>(N);
		for (j = 0; j < N; j++) {
			cs[i][j] = vector< float>(M);
		}
	}

	for (l = 0; l < M; l++) {
		for (auto i : SP[l]) {
			for (auto j : SP[l]) {
				arquivo >> cs[i][j][l];
			}
		}
	}

	
	arquivo.close();
}


Problema::~Problema()
{
}
