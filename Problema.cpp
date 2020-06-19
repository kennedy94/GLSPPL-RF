#include "Problema.h"

Problema::Problema(const char* nome){
	ifstream arquivo(nome);
	int i, j, l, t;

	arquivo >> N >> T >> W >> M >> CA;


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
		int number;
		std::vector<int> myNumbers;
		while (iss >> number) {
			number--;
			myNumbers.push_back(number);
		}
		SP.push_back(myNumbers);
	}

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
		int number;
		std::vector<int> myNumbers;
		while (iss >> number) {
			myNumbers.push_back(number);
		}
		lm.push_back(myNumbers);
	}

	CP = vector< vector<int>>(M);
	for (int l = 0; l < M; l++){
		CP[l] = vector<int>(T);
		for (int t = 0; t < T; t++){
			arquivo >> CP[l][t];
		}
	}

	//Estoques e demandas
	l0_plus = vector<int>(N);
	l0_minus = vector<int>(N);
	for (i = 0; i < N; i++){
		arquivo >> l0_plus[i];
	}
	for (i = 0; i < N; i++) {
		arquivo >> l0_minus[i];
	}

	for (i = 0; i < N; i++){
		for (t = 0; t < T; t++){
			arquivo >> d[i][t];
		}
	}


	//setup
	st = vector<vector<vector<int>>>(N);
	for (i = 0; i < N; i++){
		st[i] = vector< vector< int>>(N);
		for (i = 0; i < N; i++) {
			st[i][j] =  vector< int>(M);
		}
	}

	for (l = 0; l < M; l++){	
		for (i = 0; i < N; i++){
			for (j = 0; j < N; j++){
				arquivo >> st[i][j][l];
			}
		}
	}
	
	arquivo.close();
}

Problema::~Problema()
{
}
