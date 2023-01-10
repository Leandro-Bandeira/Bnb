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
vector < vector < int > >*  hungarian_solve(Data* data, vector < pair < int, int > >& tree_forbidden_arcs, double& obj_value) {
	
	//vector < vector < int > >* subtours = new vector < vector < int >>();

	double **cost = new double*[data->getDimension()];
	for (int i = 0; i < data->getDimension(); i++){
		cost[i] = new double[data->getDimension()];
		for (int j = 0; j < data->getDimension(); j++){
			
			cost[i][j] = data->getDistance(i, j);
		}
	}
	
	for(auto arc : tree_forbidden_arcs) {
		cout << arc.first << " " << arc.second << endl;
		cost[arc.first - 1][arc.second - 1] = 99999999;
		cost[arc.second - 1][arc.first - 1] = 99999999;
	}
	
	hungarian_problem_t p;
	int mode = HUNGARIAN_MODE_MINIMIZE_COST;
	hungarian_init(&p, cost, data->getDimension(), data->getDimension(), mode); // Carregando o problema

	obj_value = hungarian_solve(&p);
	cout << "Obj. value: " << obj_value << endl;

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
	
	
	/* Organiza os subtours do menor arco para o maior	*/
	//sort(subtours->begin(), subtours->end(), [=](auto  A, auto B){
	//		return A.size() <  B.size(); 
	//}
	//);   
	

	std::vector < std::vector < int > > subtours; // subtours da solução do problema APtsp
	  
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
	
	
	
	getchar();
	//return subtours;
	

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

void bnb_solve(Data* data) {
	double obj_value;
	int n = data->getDimension(); // Tamanho da instancia

	vector <pair < int, int >> tree_forbidden_arcs; // Conjunto de todos os arcos proibidos da arvore

	vector < vector < int > >* subtours = hungarian_solve(data, tree_forbidden_arcs, obj_value);
	vector < NodeInfo* > tree; // lista de nós, que representa uma árvore
	int loc; //Localização do nó
	/* instanceNode configura os dados da solução, enviamos a raiz que é o primeiro problema	*/
	NodeInfo* root = instanceNode(*subtours, obj_value, n);
	tree.push_back(root);

	/* Algoritmo BnB	*/
	double upper_bound = numeric_limits <double>::infinity(); // Seta o upperbound como infinito
	
	while(!tree.empty()) {
		
			NodeInfo*  node_chosen = chooseNode(tree, loc); // Retorna um nó de forma aleatoria
			
			/* Se a solução tiver um lower_bound maior do que upper_bound, vamos desconsiderá-la, deletamos ela da região de memória	*/
			if(node_chosen->lower_bound > upper_bound) {
				cout << "entrou aqui" << endl;
				tree.erase(tree.begin() + loc);
				delete node_chosen;
				continue;
			}
			// Se a minha solução é viável, mudamos o upper_bound
			if(node_chosen->feasible)
				upper_bound = min(upper_bound, node_chosen->lower_bound);
			
			// Salva todos os arcos proibidos
			tree_forbidden_arcs.push_back(make_pair(node_chosen->forbidden_arcs[0].first, node_chosen->forbidden_arcs[0].second));
			cout << "Arcos proibidos: ---------------------------------------" << endl;

			for(auto arc : tree_forbidden_arcs) {
				cout << arc.first << " " << arc.second << endl;
			}
			cout << "-----------------------------------------------" << endl;
			delete subtours;
			/* Vamos solucionar o hungarian sempre com os primeiro arcos proibidos, pois eles tem o menor indice	*/
			subtours = hungarian_solve(data, tree_forbidden_arcs, obj_value);
			/* Inicializa um novo nó com as caracteristicas do subtour do anterior	*/
			NodeInfo* node = instanceNode(*subtours, obj_value, n);	
			tree.erase(tree.begin() + loc); // Apaga o nó anterior utilzizado para criar o nó atual
			delete node_chosen; // deleta da regiao de memoria
			tree.push_back(node);
			cout << "uperBound: " << upper_bound << endl;
			getchar();
	}
	
	cout << "saiu" << endl;
	cout << "solucao Otima: " << upper_bound << endl;
}


int main(int argc, char** argv) {

	Data * data = new Data(argc, argv[1]);
	data->readData();

	cout << "-----------------------------" << endl;
	
		
	bnb_solve(data);

	delete data;
	return 0;
}
