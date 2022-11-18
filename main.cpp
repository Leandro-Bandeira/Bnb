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



NodeInfo* instanceNode(vector < vector < int > >& subtours, double obj_value) {
	
	NodeInfo* node = new NodeInfo();
	/* Inicializa os subtours e o valor do lower bound dados	*/
	node->subtours = subtours;
	node->lower_bound = obj_value;
	
	/* Indicação se a solução eh viável ou não	*/

	if(subtours.size() > 1) {
		node->feasible = false;
	}
	else {
		node->feasible = true;
	}

	int size_lower_arc = subtours[0].size();
	/* Verifica o tamanho do menor arco	*/
	for(auto line : subtours) {

		if(line.size() < size_lower_arc) {

			size_lower_arc = line.size();
		}
	}
	
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
	
	for(int i = 0; i  < node->forbidden_arcs.size(); i++) {

			cout << node->forbidden_arcs[i].first << " " << node->forbidden_arcs[i].second << endl;
	}

	cout << "lower_arc_ind: " << node->chosen << endl;

	return node;
}

/* Retorna a solução em formato de subtours	*/
vector < vector < int > >*  hungarian_solve(Data* data, int nodeChosenA, int nodeChosenB, double& obj_value) {
	
	vector < vector < int > >* subtours = new vector < vector < int >>();

	double **cost = new double*[data->getDimension()];
	for (int i = 0; i < data->getDimension(); i++){
		cost[i] = new double[data->getDimension()];
		for (int j = 0; j < data->getDimension(); j++){
			if((i == nodeChosenA && j == nodeChosenB) || (i == nodeChosenB && j == nodeChosenA)) {
				
				cost[i][j] = numeric_limits<double>::infinity();

			}
			else {
				cost[i][j] = data->getDistance(i, j);
			}
		}
	}
	
	hungarian_problem_t p;
	int mode = HUNGARIAN_MODE_MINIMIZE_COST;
	hungarian_init(&p, cost, data->getDimension(), data->getDimension(), mode); // Carregando o problema

	obj_value = hungarian_solve(&p);
	cout << "Obj. value: " << obj_value << endl;

	cout << "Assignment" << endl;
	hungarian_print_assignment(&p);
	
	cout << "here" << endl;
	
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
	
	/* Vamos percorrer todos as linhas e para cada linha vamos achar todos os subtours, ou seja até i = j	*/
	
	int alocTask;
	int index_subtour = 0;
	int it = 0;
	/* Itera linha por linha até a última, procura-se a alocação a partir da linha fixada dada por alocTask, após isso repete o loop com a matriz fixada na task alo	cada, se a iteração encontra o mesmo nó que o inicial, então temos um arco*/

	while(it < rows) {

		vector < int > subtour_i;
		subtour_i.push_back(index_subtour + 1);
		alocTask = index_subtour;
			
		while(alocTask < rows) {
			for(int j = 0; j < cols; j++) {

				if(apMatrix[alocTask][j]) {
					alocTask = j; // Armazena o indice da tarefa que i foi alocado
					break;
				}
			}
			subtour_i.push_back(alocTask + 1);
			
			if(alocTask == index_subtour) {
				subtours->push_back(subtour_i);
				break;
			}
		
		}

		
		subtour_i.clear();
		index_subtour += 1;
		it += 1;

	}
	
	sort(subtours->begin(), subtours->end(), [=](auto  A, auto B){
			return A.size() >  B.size(); 
	}
	);   
	


	
	for(auto line: *subtours) {

		for(int i = 0; i < line.size(); i++) {

			cout << line[i] << " ";
		}
		cout << endl;
	}
	


	return subtours;
	

}

int chooseNode(vector < NodeInfo * >& tree) {
	int i = 0;
	for(auto node : tree) {

		if(!node->feasible) {

			return i;
		}
		i++;
	}
	return -1;

}

void bnb_solve(Data* data) {
	double obj_value;
	vector < vector < int > >* subtours = hungarian_solve(data, -1, -1, obj_value);
	vector < NodeInfo* > tree; // lista de nós, que representa uma árvore
	
	NodeInfo* node = instanceNode(*subtours, obj_value);
	tree.push_back(node);
	/* Algoritmo BnB	*/
	double upperBound = numeric_limits <double>::infinity(); // Seta o upperbound como infinito
	
	while(!tree.empty()) {
		
			int  node_chosen = chooseNode(tree);
			if(node_chosen == -1) {
				break;
			}
			cout << "primeiro arco: " << tree[node_chosen]->forbidden_arcs[0].first << " " << "second arc: " <<  tree[node_chosen]->forbidden_arcs[0].second << endl			;

			getchar();
			/* Vamos solucionar o hungarian sempre com os primeiro arcos proibidos, pois eles tem o menor indice	*/
			subtours = hungarian_solve(data,tree[node_chosen]->forbidden_arcs[0].first - 1, tree[node_chosen]->forbidden_arcs[0].second - 1, obj_value);
			
			/* Se a minha solução relaxada, tiver o lower_bound maior do que o upperbound do BNB, essa solução eh inválida	*/
			if(tree[node_chosen]->lower_bound > upperBound) {
				tree.erase(tree.begin() + node_chosen);
				continue;

			}
			
			/* Se a minha solução não for viável, vamos pegar um nó e dividir em novos subproblemas	*/
			if(!tree[node_chosen]->feasible) {
				
				upperBound = min(upperBound, tree[node_chosen]->lower_bound);
				NodeInfo* node = instanceNode(*subtours, obj_value);
				tree.push_back(node);
			}
			tree.erase(tree.begin() + node_chosen);
			cout << "uperBound: " << upperBound << endl;
	}
	
	cout << "saiu" << endl;
	cout << "solucao Otima: " << upperBound << endl;
}


int main(int argc, char** argv) {

	Data * data = new Data(argc, argv[1]);
	data->readData();

	cout << "-----------------------------" << endl;
	
		
	bnb_solve(data);

	delete data;
	return 0;
}
