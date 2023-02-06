#include <iostream>
#include <algorithm>
using namespace std;

#include "data.h"
#include "hungarian.h"
#include <vector>
#include <list>
#include <limits>
#include <time.h>
#include <queue>

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
	//cout << "Obj. value: " << node->lower_bound << endl;

	//cout << "Assignment" << endl;
	//hungarian_print_assignment(&p);
	
	/* Criação da matriz que retorna do problema de AP	*/	
	int rows = p.num_rows;
	int cols = p.num_cols;
	
	/* Fim da criação de matriz	*/
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

				if(p.assignment[line][j] == 1) {
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

	hungarian_free(&p);
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


	if(choice == 1) {

		return &tree->back();
	}

	return &tree->front();
}




/* Função que resolve o BNB	*/
double bnb_solve(Data* data, int valueChoice) {
	double obj_value;
	int n = data->getDimension(); // Tamanho da instancia
	
	NodeInfo root; // Raiz do problema

	double upper_bound = numeric_limits<double>::infinity();
	int k = 0;
	int choice = valueChoice; // 1 -> Back 2 -> front
	
	
	if(valueChoice == 1) {

		cout << "Funcionando no back" << endl;
	}
	else if (valueChoice == 2) {
		cout << "Funcionando no front" << endl;
	}
	else {

		cout << "Funcionando no lower bound" << endl;
	}

	getchar();
	
	if(choice == 1 or choice == 2) {
		NodeInfo nodeTemp;

		//hungarian_solve(data, &root);
		list < NodeInfo > tree; // Criação da nossa árvore

		tree.push_back(root); // Adiciona o primeiro nó que é a raiz

		while(!tree.empty()) {
			

			auto node = chooseNode(&tree, choice);
			
			hungarian_solve(data, node);
			nodeTemp = *node; // Para podermos apagar o ultimo caso seja o back	
			
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
		
				if(choice == 1) {
					
					tree.pop_back();
				}
				else {
					
					tree.pop_front();
				}
				
			
				continue;

			}
		
		
			/* Criamos um vector que contém os novos nós, pois colocamos aqui temporariamente, apagamentos de fato o ultimo nó e então colocamos na árvore	*/

			int j =  node->chosen;
			if(choice == 1) {

				tree.pop_back();
			}
			else {

				tree.pop_front();
			}
			
		

			for(int i = 0; i < nodeTemp.subtours[j].size() - 1; i++) {
					
				NodeInfo n;
				n.forbidden_arcs = nodeTemp.forbidden_arcs;
							
				n.forbidden_arcs.push_back(make_pair(nodeTemp.subtours[j][i], nodeTemp.subtours[j][i+1]));
				
				tree.push_back(n);

			}

			
			k++;

		}
	}
	
	/* Utiliza priority_queue	*/
	else {
		
		
		hungarian_solve(data, &root);
		
		/* Função lambda que retorna se o primeiro valor é maior que o segundo	*/
		auto compare = [](NodeInfo nodeA, NodeInfo nodeB){return nodeA.lower_bound >  nodeB.lower_bound;};
		
		/* Declração do priority_queue, para utilizar a função lambda anterior	*/
		priority_queue < NodeInfo, vector < NodeInfo >, decltype(compare) > q(compare);
		q.push(root);
		int k = 0;
		while(!q.empty()) {
			
			/* Pega-se o primeiro elemento, caso seja maior que o lower_bound deleta ou seja viável, pegamos sua região válida	*/
			auto node = q.top();
			if(node.lower_bound >= upper_bound) {

				q.pop();
				continue;
			}
			
			if(node.feasible) {

				upper_bound = min(upper_bound, node.lower_bound);

				q.pop();
				continue;
			}

			int j = node.chosen;
			NodeInfo nodeTemp = node;
			q.pop();
			
			/* Caso não, fazemos seus filhos e então resolvemos para armazenar no priority_qeue	*/
			for(int i = 0; i < nodeTemp.subtours[j].size() - 1; i++) {
				
				NodeInfo n;

				n.forbidden_arcs = nodeTemp.forbidden_arcs;

				n.forbidden_arcs.push_back(make_pair(nodeTemp.subtours[j][i], nodeTemp.subtours[j][i + 1]));
				hungarian_solve(data, &n);
				q.push(n);


			}
			k++;
		}
 

	}
	return upper_bound;
}


int main(int argc, char** argv) {

	Data * data = new Data(argc, argv[1]);
	data->readData();

	cout << "Digite o modo de busca: ";
	int choice;
	cin >> choice;
	cout << "-----------------------------" << endl;
	
	time_t t_ini = time(NULL);
		
	double upper_bound = bnb_solve(data, choice);

	time_t t_fim = time(NULL);
	float tempo = difftime(t_fim, t_ini);
	cout << upper_bound << " " <<  tempo << '\n';
	delete data;
	return 0;
}
