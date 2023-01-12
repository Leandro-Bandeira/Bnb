#include <iostream>
#include <algorithm>
using namespace std;

#include "data.h"
#include "hungarian.h"
#include <vector>
#include <list>
#include <limits>


/* Indica a estrutura do nó	*/
typedef struct {

	vector < pair<int, int >> forbidden_arcs; // Lista de arcos proibidos no nó
	vector < vector < int > > subtours; // Conjunto de subtours da solução
	double lower_bound; // Custo total da solução do algoritmo hungaro
	int chosen; // Indice do menor subtour
	double feasible; // Indica se a solução AP_TSP é viável

}NodeInfo;



NodeInfo* instanceNode(vector < vector < int > >& subtours, double obj_value, int n) {
	
	NodeInfo* node = new NodeInfo();
	/* Inicializa os subtours e o valor do lower bound dados	*/
	node->subtours = subtours;
	node->lower_bound = obj_value;
	
	/* Indicação se a solução eh viável ou não	*/

	if(subtours.size() > 1) {
		node->feasible = false;
	}
	else {
		for(auto tour : subtours) {
			
			if(tour.size() - 1 != n) {
				node->feasible = false;
			}
			else {
				node->feasible = true;
			}
		}
	}
	
	int size_lower_arc = subtours[0].size(); // A matriz de subtours organiza os tours de forma crescente, logo o primeiro é o menor arco
	
	/* Restringe todos os vértices que estão no menor arco, e vamos restringir todos os nós daquele subtour*/
	int j = 0;
	bool lower_arcf = false;
	for(vector < int > subtour : subtours) {
		
		if(subtour.size() == size_lower_arc) {
			if(!lower_arcf){

				node->chosen = j;
				lower_arcf = true;
			}
				
			for(int i = 0; i < subtour.size() - 1; i++) {
				node->forbidden_arcs.push_back(make_pair(subtour[i], subtour[i + 1]));
			} 

		}
		
		j++;
	}
	
	return node;
}

/* Retorna a solução em formato de subtours	*/
void hungarian_solve(Data* data, NodeInfo* node) {
		
	double **cost = new double*[data->getDimension()];
	for (int i = 0; i < data->getDimension(); i++){
		cost[i] = new double[data->getDimension()];
		for (int j = 0; j < data->getDimension(); j++){
			
			cost[i][j] = data->getDistance(i, j);
		}
	}
	
	hungarian_problem_t p;
	int mode = HUNGARIAN_MODE_MINIMIZE_COST;
	hungarian_init(&p, cost, data->getDimension(), data->getDimension(), mode); // Carregando o problema

	node->lower_bound = hungarian_solve(&p); // Armazena o valor da função objetivo do algoritmo hungaro
	cout << "Obj. value: " << node->lower_bound << endl;

	cout << "Assignment" << endl;
	hungarian_print_assignment(&p);
	
	
	/* Criação da matriz que retorna do problema de AP	*/	
	int rows = p.num_rows;
	int cols = p.num_cols;
	int ** apMatrix = new int *[p.num_rows];
	for(int i = 0; i < rows; i++) {

		for(int j = 0; j < cols; j++) {

			apMatrix[i] = new int[cols];
		}
	}

	for(int i = 0; i < rows; i++) {

		for(int j = 0; j < cols; j++) {

			apMatrix[i][j] = p.assignment[i][j];
		}
	}
	
	/* Fim da criação de matriz	*/
	hungarian_free(&p);
	for (int i = 0; i < data->getDimension(); i++) delete [] cost[i];
	delete [] cost;
	
	
	
	vector < vector < int > > subtours;

	  
	bool verticeFound = false;// Utilizado para verificar se o vértice está ou não em um subtour

	for(int i = 0; i < rows; i++) {
		std::vector < int > subtour;
		
		/* Verifica se o vertice i está ou não em algum subtour, caso não, podemos fazer um subtour com ele no inicio	*/
		for(auto line : subtours) {

			for(int k = 0; k < line.size(); k++) {
				
				if(i == line[k]) {
					
					verticeFound = true;

				}
			}
		}
		
		if(verticeFound) {
			verticeFound = false;
			continue;
		}

		/* Criando um novo subtour, percorremos as colunas até achar em qual foi alocado, após isso podemos parar o loop e verificar se temos um subtour
		 * Que é exatamente quando o ponto inicial é igual ao ponto final	*/
		subtour.push_back(i);
		int line = i;
		int j = 0;
		while(1) { 
					
			for(int j = 0; j < cols; j++) {

				if(apMatrix[line][j] == 1) {
					subtour.push_back(j);
					line = j;
					break;
				}
			}
			if(line == i) {

				break;
			}
		
		}
		
		//Adiciona ao subtour a coletania de subtours
		subtours.push_back(subtour);
	}

	
	// Organiza os subtours do menor para o maior
	std::sort(subtours.begin(), subtours.end(), [=](auto A, auto B) {

				return A.size() < B.size();
	});

	for(auto line: subtours) {

		for(int i = 0; i < line.size(); i++) {

			cout << line[i] << " ";
		}
		cout << endl;
	}
	
	cout << "here" << endl;
	node->subtours = subtours; // atualiza o subtours do nó
	
	/* Verifica se o nó é viável ou não	*/
	if(node->subtours.size() > 1) {

		node->feasible = true;
	}
	else {

		node->feasible = false;
	}
	
	int id = 0;
	/* Algoritmo para armzenar o subtour de menor índice	*/
	if(!node->feasible) {
		
		int lower_size = node->subtours[0].size();

		for(int i = 0; i < node->subtours.size(); i++) {

			if(node->subtours[i].size() < lower_size) {

				lower_size = node->subtours[i].size();
				id = i;
			}
		}

		node->chosen = id; // Armazena o subtour de menor indice

	}
	
	/* Cria os arcos proibidos	*/
	for(int i = 0; i < node->subtours[id].size() - 1; i++) {
		
		int first = node->subtours[id][i];
		int second = node->subtours[id][i + 1];

		node->forbidden_arcs.push_back(make_pair(first, second));
	}
}

/* Vamos retornar um nó de forma aleatoria	*/
NodeInfo* chooseNode(vector < NodeInfo* >& tree, int& loc) {
	unsigned seed = time(0);
	srand(seed);
	int i = rand() % tree.size();
	int j = 0;	
	for(auto node : tree) {

		if(j == i) {

			return node;
		}
		j++;
	}
	return NULL;
	loc = i;
}

/* Função que resolve o BNB	*/
void bnb_solve(Data* data) {
	double obj_value;
	int n = data->getDimension(); // Tamanho da instancia
	
	NodeInfo root; // Raiz do problema

	hungarian_solve(data, &root);

	list < NodeInfo > tree; // Criação da nossa árvore

	tree.push_back(root); // Adiciona o primeiro nó que é a raiz

	double upper_bound = numeric_limits<double>::infinity();

	while(!tree.empty()) {


	}
}


int main(int argc, char** argv) {

	Data * data = new Data(argc, argv[1]);
	data->readData();

	cout << "-----------------------------" << endl;
	
		
	bnb_solve(data);

	delete data;
	return 0;
}
