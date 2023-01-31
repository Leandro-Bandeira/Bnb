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




/* Retorna a solução em formato de subtours	*/
void hungarian_solve(Data* data, NodeInfo* node) {
		
	
	double **cost = new double*[data->getDimension()];
	for (int i = 0; i < data->getDimension(); i++){
		cost[i] = new double[data->getDimension()];
		for (int j = 0; j < data->getDimension(); j++){
			
			cost[i][j] = data->getDistance(i, j);
		}
	}
	
	for(int i = 0; i < node->forbidden_arcs.size(); i++) {
		
		int first = node->forbidden_arcs[i].first;
		int second = node->forbidden_arcs[i].second;
		cost[first][second] = 999999999;
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
			continue;
		}
		cout << endl;
	}
	
	node->subtours = subtours; // atualiza o subtours do nó
	
	/* Verifica se o nó é viável ou não	*/
	if(node->subtours.size() > 1) {

		node->feasible = false;
	}
	else {
		node->chosen = 0;
		node->feasible = true;
	}
	
	int id = 0;

	/* Algoritmo para achar o subtour de menor indice	*/
	/* Como a matriz está organizada de subtours do menor para o maior, o primeiro subtour possui o menor tamanho	*/
	
	if(!node->feasible) {
		
		int lower_size = node->subtours[0].size();
		int lower_id = node->subtours[0][0];

		for(int i = 1; i < node->subtours.size(); i++) {

			if(node->subtours[i].size() == lower_size and node->subtours[i][0] < lower_id ) {

				id = i;
				lower_id = node->subtours[i][0];

			}
		}

		node->chosen = id; // indice do subtour de menor tamanho
		
	}
	
	
}

/* Vamos retornar um nó de forma aleatoria	*/
NodeInfo*  chooseNode(list < NodeInfo >* tree, int& choice) {
	
	unsigned seed = time(0);
	srand(seed);
	choice = (rand() % 2) + 1; // Escolhe um indice aleatorio da arvore
		
	

	switch(choice) {

		case 1:
			return &tree->back();
		case 2:
			return &tree->front();
	}
	

}

/* Função que resolve o BNB	*/
void bnb_solve(Data* data) {
	double obj_value;
	int n = data->getDimension(); // Tamanho da instancia
	
	NodeInfo root; // Raiz do problema

//	hungarian_solve(data, &root);

	list < NodeInfo > tree; // Criação da nossa árvore

	tree.push_back(root); // Adiciona o primeiro nó que é a raiz
	double upper_bound = numeric_limits<double>::infinity();
	int k = 0;
	while(!tree.empty()) {
		
		if(tree.size() == 1) {
			cout << "1" << endl;
			getchar();
		}
		int choice = 0;
		cout << "--------------------------------------------------------------------------------------------------------" << endl;
		cout << "interacao: " << k << endl;
		int p = 0;
		for(list < NodeInfo >::iterator it = tree.begin(); it != tree.end(); ++it) {
			
			cout << "Arcos probidio na nó: " << p << endl;

			for(int i = 0; i < (*it).forbidden_arcs.size(); i++) {
				
				cout << "( ";
				cout << (*it).forbidden_arcs[i].first << " " << (*it).forbidden_arcs[i].second << " ";
				cout << ")";
			}
			cout << endl;
			p++;
		}
		
		cout << "--------------------------------------------------------------------------------------------------------" << endl;
		auto node = chooseNode(&tree, choice);

		cout << "Arcos proibidos do nó escolhido: ";
		for(int i = 0; i < node->forbidden_arcs.size(); i++) {
			cout << "( ";
			cout << node->forbidden_arcs[i].first << " " << node->forbidden_arcs[i].second << " ";
			cout << ")";
		}
		cout << endl;
		hungarian_solve(data, node);

		if(node->lower_bound >= upper_bound) {
			

			if(choice == 1) {

				tree.pop_back();
			}
			else {

				tree.pop_front();
			}
			continue;
		}

		if(node->feasible) {
			upper_bound = min(upper_bound, node->lower_bound);
	
			cout << "upper_bound: " << upper_bound << endl;


			if(choice == 1) {
				
				tree.pop_back();
			}
			else {
				
				tree.pop_front();
			}
			continue;

		}
	
	
	/* Criamos um vector que contém os novos nós, pois colocamos aqui temporariamente, apagamentos de fato o ultimo nó e então colocamos na árvore	*/

		vector < NodeInfo > new_nodes;
		new_nodes.clear();
		int j =  node->chosen;
		for(int i = 0; i < node->subtours[j].size() - 1; i++) {
				
			NodeInfo n;
			n.forbidden_arcs = node->forbidden_arcs;
						
			n.forbidden_arcs.push_back(make_pair(node->subtours[j][i], node->subtours[j][i+1]));
			
			if(choice == 1) {

				new_nodes.push_back(n);
			}
			else {

				tree.push_back(n);
			}
				
			cout << "Arcos proibidos dos filhos: ";
			for(int i = 0; i < n.forbidden_arcs.size(); i++) {
					
				cout << "( ";
				cout << n.forbidden_arcs[i].first << " " << n.forbidden_arcs[i].second << " ";
				cout << ") ";
			}	
			cout << endl;
		}

			

		cout << "choice: " << choice << endl;
		cout << "tree Size: " << tree.size() << endl;
		
		
		if(choice == 1) {
			
			/* Apaga de fato o ultimo elemento e adiciona os novos	*/
			tree.pop_back();

			for(int i = 0; i < new_nodes.size(); i++) {

				tree.push_back(new_nodes[i]);
			}
			cout << "here" << endl;
		
		}
		else {

			tree.pop_front();
			cout << "here2" << endl;
		}
		k++;
		getchar();
	}

	cout << upper_bound << endl;
	getchar();
}


int main(int argc, char** argv) {

	Data * data = new Data(argc, argv[1]);
	data->readData();

	cout << "-----------------------------" << endl;
	
		
	bnb_solve(data);

	delete data;
	return 0;
}
