#include "Problema.h"

Problema::Problema(const char* nome){

	instancia = nome;
	ifstream arquivo(nome);

	//inteiros
	arquivo >> N >> T >> W >> M >> CA;

	W_p = W / T;

	W++;
	T++;

	//---------------------------------------------------------------
	//conjunto de produtos por máquina
	vector<string> vec_string;
	string str;
	int ll = 0;
	while (ll < M){
		getline(arquivo, str);
		if (str.size() > 0) {
			vec_string.push_back(str);
			ll++;
		}
	}

	l_produz_i = vector<vector<bool>>(M);
	for (int l = 0; l < M; l++) {
		l_produz_i[l] = vector<bool>(N, 0);
	}

	ll = 0;
	for (auto stg: vec_string) 
	{
		std::stringstream iss(stg);
		float number;
		std::vector<int> myNumbers;
		while (iss >> number) {
			number--;
			l_produz_i[ll][number] = true;
			myNumbers.push_back(number);
		}
		ll++;
		SP.push_back(myNumbers);
	}

	//---------------------------------------------------------------
	//lote mínimo
	ll = 0;
	vec_string.clear();
	while (ll < M) {
		getline(arquivo, str);
		if (str.size() > 0) {
			vec_string.push_back(str);
			ll++;
		}
	}
	for (auto stg : vec_string)
	{
		std::stringstream iss(stg);
		float number;
		std::vector<double> myNumbers;
		while (iss >> number) {
			myNumbers.push_back(number);
		}
		AUX.push_back(myNumbers);
	}

	lm = vector< vector<int> >(N);
	for (int i = 0; i < N; i++)	{
		lm[i] = vector<int>(M, 0);
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
	CP = vector< vector<int>>(M);
	for (int l = 0; l < M; l++) {
		CP[l] = vector<int>(T);
		CP[l][0] = 0;
		for (int t = 1; t < T; t++) {
			arquivo >> CP[l][t];
		}
	}

	//tempo de processamento
	ll = 0;
	vec_string.clear();
	while (ll < M) {
		getline(arquivo, str);
		if (str.size() > 0) {
			vec_string.push_back(str);
			ll++;
		}
	}
	for (auto stg : vec_string)
	{
		std::stringstream iss(stg);
		float number;
		std::vector<double> myNumbers;
		while (iss >> number) {
			myNumbers.push_back(number);
		}
		AUX.push_back(myNumbers);
	}

	p = vector< vector<double> >(N);
	for (int i = 0; i < N; i++) {
		p[i] = vector<double>(M, 0.0);
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
	I0_plus = vector<int>(N, 0);
	I0_minus = vector<int>(N, 0);
	for (int i = 0; i < N; i++){
		arquivo >> I0_plus[i];
	}
	//backorder iniciais
	for (int i = 0; i < N; i++) {
		arquivo >> I0_minus[i];
	}

	//demanda
	d = vector<vector<int>>(N);
	for (int i = 0; i < N; i++){
		d[i] = vector<int>(T);
		for (int t = 1; t < T; t++){
			arquivo >> d[i][t];
		}
	}


	//setup time
	st = vector<vector<vector<int>>>(N);
	for (int i = 0; i < N; i++){
		st[i] = vector< vector< int>>(N);
		for (int j = 0; j < N; j++) {
			st[i][j] =  vector< int>(M, 0);
		}
	}

	for (int l = 0; l < M; l++){	
		for (auto i : SP[l]) {
			for (auto j : SP[l]) {
				arquivo >> st[i][j][l];
			}
		}
	}

	//custos

	//custo estoque
	h = vector<double>(N);
	for (int i = 0; i < N; i++){
		arquivo >> h[i];
	}

	//custo backorder
	g = vector<double>(N);
	for ( int i = 0; i < N; i++) {
		arquivo >> g[i];
	}


	//custo de produção dos produtos
	//CUIDADO, ESTÁ TRANSPOSTO
	ll = 0;
	vec_string.clear();
	while (ll < M) {
		getline(arquivo, str);
		if (str.size() > 0) {
			vec_string.push_back(str);
			ll++;
		}
	}
	for (auto stg : vec_string)
	{
		std::stringstream iss(stg);
		float number;
		std::vector<double> myNumbers;
		while (iss >> number) {
			myNumbers.push_back(number);
		}
		AUX.push_back(myNumbers);
	}

	cp = vector< vector<double> >(N);
	for (int i = 0; i < N; i++) {
		cp[i] = vector<double>(M, 0.0);
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
	cs = vector<vector<vector<int>>>(N);
	for (int i = 0; i < N; i++) {
		cs[i] = vector< vector< int>>(N);
		for (int j = 0; j < N; j++) {
			cs[i][j] = vector< int>(M, 0);
		}
	}

	for (int l = 0; l < M; l++) {
		for (auto i : SP[l]) {
			for (auto j : SP[l]) {
				arquivo >> cs[i][j][l];
			}
		}		
	}

	
	arquivo.close();

	int n_prod_maq = 0;
	for (int l = 0; l < M; l++){
		n_prod_maq += SP[l].size();
	}

	ofstream saida("var_const.csv");

	saida << instancia << "," << n_prod_maq * W << ","
		<< N * T + T + M * T + 2 * (n_prod_maq * W) + M * W + 2 * (n_prod_maq * 2 * W);

	saida.close();
}


Problema::~Problema()
{
}
