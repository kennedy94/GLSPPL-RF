#include "Problema.h"

void Problema::criar_modelo(){
	modelo = IloModel(env);
	cplexvar_initiate();
	fo();
	restricoes();
}

void Problema::cplexvar_initiate(){
	IloInt i, j, l, t, s;
	q = IloArray<IloArray<IloIntVarArray>>(env, N);
	for (i = 0; i < N; i++){
		q[i] = IloArray<IloIntVarArray>(env, M);
		for (l = 0; l < M; l++){
			q[i][l] = IloIntVarArray(env, W, 0, IloInfinity);
		}
	}

	x = IloArray<IloArray<IloBoolVarArray>>(env, N);
	for (i = 0; i < N; i++) {
		x[i] = IloArray<IloBoolVarArray>(env, M);
		for (l = 0; l < M; l++) {
			x[i][l] = IloBoolVarArray(env, W);
		}
	}


	y = IloArray<IloArray<IloArray<IloBoolVarArray>>>(env, N);
	for (i = 0; i < N; i++) {
		y[i] = IloArray<IloArray<IloBoolVarArray>>(env, N);
		for (j = 0; j < N; j++) {
			y[i][j] = IloArray<IloBoolVarArray>(env, M);
			for (l = 0; l < M; l++) {
				y[i][j][l] = IloBoolVarArray(env, W);
			}
		}
	}
	
	I_plus = IloArray<IloIntVarArray>(env, N);
	I_minus = IloArray<IloIntVarArray>(env, N);

	for (i = 0; i < N; i++) {
		I_plus[i] = IloIntVarArray(env, T, 0, IloInfinity);
		I_minus[i] = IloIntVarArray(env, T, 0, IloInfinity);
	}
	IloExpr expr(env);

	expr += I_minus[0][0];

}

void Problema::fo(){
	IloInt i, j, t, s, l;
	OBJETIVO = IloExpr(env);


	for (i = 0; i < N; i++) {
		for (t = 1; t < T; t++) {
			OBJETIVO += h[i] * I_plus[i][t] + g[i] * I_minus[i][t];
		}
	}


	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			for (l = 0; l < M; l++) {
				for (s = 0; s < W; s++) {
					OBJETIVO += cs[i][j][l] * y[i][j][l][s];
				}
			}
		}
	}

	for (i = 0; i < N; i++) {
		for (l = 0; l < M; l++) {
			for (s = 0; s < W; s++) {
				OBJETIVO += cp[i][l] * q[i][l][s];
				
			}
		}
	}

	modelo.add(IloMinimize(env, OBJETIVO)).setName("FO");

}

void Problema::restricoes(){
	IloInt i, j, l, s, t;
	IloExpr soma(env);

	//(2)
	for (i = 0; i < N; i++)
	{
		for (t = 1; t < T; t++){
			for (l = 0; l < M; l++){
				for (s = t * W_p; s < (t + 1) * W_p; s++) {
					soma += q[i][l][s];
				}
			}
			modelo.add(I_plus[i][t - 1] - I_minus[i][t - 1] + soma - I_plus[i][t] + I_minus[i][t] == d[i][t]);
			soma.clear();
		}
	}

	//(3)
	for (t = 0; t < T; t++) {
		for (i = 0; i < N; i++) {
			soma += I_plus[i][t];
		}
		modelo.add(soma <= CA);
		soma.clear();
	}

	//(4)
	for (l = 0; l < M; l++){
		for (t = 1; t < T; t++){
			for (auto i : SP[l]) {
				for (s = t * W_p; s < (t + 1) * W_p; s++)
				{
					soma += p[i][l] * q[i][l][s];
				}
			}

			for (auto i : SP[l]) {
				for (auto j : SP[l]) {
					for (s = t * W_p; s < (t + 1) * W_p; s++)
					{
						soma += st[i][j][l] * y[i][j][l][s];
					}
				}
			}


			modelo.add(soma <= CP[l][t]);
			soma.clear();
		}
	}

	//(5)
	for ( l = 0; l < M; l++){
		for (auto i : SP[l]) {
			for ( t = 1; t < T; t++)
			{
				for (s = t * W_p; s < (t + 1) * W_p; s++) {
					modelo.add(p[i][l] * q[i][l][s] <= CP[l][t] * x[i][l][s]);
				}
			}
		}

	}
	
	//(6)
	for (l = 0; l < M; l++) {
		for (auto i : SP[l]) {
			for (s = t * W_p; s < (t + 1) * W_p; s++) {
				modelo.add(q[i][l][s] >= lm[i][l] * (x[i][l][s] - x[i][l][s - 1]));
			}
		}
	}
}

Problema::Problema(const char* nome){
	ifstream arquivo(nome);
	int i, j, l, t;

	//inteiros
	arquivo >> N >> T >> W >> M >> CA;

	W_p = W / T;

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

void Problema::resolver(){
	criar_modelo();

	cplex = IloCplex(env);
	cplex.solve();
	getchar();


}

Problema::~Problema()
{
}
