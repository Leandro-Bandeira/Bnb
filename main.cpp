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



class Graph {
	

	public:
		Graph(int size, list < NodeInfo > *tree);
		NodeInfo* DFS(NodeInfo* vertex);
		void addVertexAdj(NodeInfo* vertex);		
	
	private:

		int numVertices;
		list < NodeInfo > *adjList;
		vector < NodeInfo > *visited; // Indica pelo indice se foi visitado ou não
		list < NodeInfo > *tree; // Aponta para a árvore do problema


	
};


Graph::Graph(int size, list < NodeInfo > *tree) {
	int numVertices = size;
	this->tree = tree;
	adjList = new list < NodeInfo >[size];
	visited = new vector < NodeInfo > [size];
	
}


void Graph::addVertexAdj(NodeInfo* vertex) {
	
	for(auto vertexA : *tree) {
		
		bool verification_visited = false;

		/* Verifica se o vértice vizinho já foi visitado ou não	*/
		for(auto visitedVertex : *visited) {

			if(visitedVertex.forbidden_arcs == vertexA.forbidden_arcs) {

				verification_visited = true;
			} 
		} 
		/* Verifica se os arcos proibidos deles são iguais, dizendo que são adjacentes	*/
		if(vertexA.forbidden_arcs == vertex->forbidden_arcs and !verification_visited ) {
		
			/* Por fim verificamos se o vértice está ou não na lista de adjacentes	*/
			bool verification_adjVertex = false;

			for(auto adjVertex : *adjList) {

				if(adjVertex.forbidden_arcs == vertexA.forbidden_arcs) {

					verification_adjVertex = true;
				}
			}
			
			if(!verification_adjVertex) {

				adjList->push_back(vertexA);
			}
		}

	}
}


/* Inicializa a partir de um nó aleatorio que chamaremos de root	*/
NodeInfo* Graph::DFS(NodeInfo *root) {
	
	visited->push_back(*root); // Adiciona o vértice a aqueles que foram visitados
	addVertexAdj(root);

	while(!adjList->empty()) {
		/* Adiciona o ultimo elemento do stack aos visitados	*/
		/* Além disso, adiciona a lista de vértices adjacentes	*/
		NodeInfo nodeLast = adjList->back();
		visited->push_back(nodeLast);
		adjList->pop_back(); // Remove da lista o ultimo elemento
		addVertexAdj(&nodeLast);
		cout << "oi" << endl;
	}

	NodeInfo* lastNode = new NodeInfo;

	*lastNode = visited->back();

	return lastNode;




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
	
	/* Cria os arcos proibidos	*/
	for(int i = 0; i < node->subtours[id].size() - 1; i++) {
		
		int first = node->subtours[id][i];
		int second = node->subtours[id][i + 1];

		node->forbidden_arcs.push_back(make_pair(first, second));
	}
	
	cout << "Node Chosen: ";
	for(int i = 0; i < node->subtours[node->chosen].size(); i++) {

		cout << node->subtours[node->chosen][i] << " ";
	}
	cout << endl;
	
}

/* Vamos retornar um nó de forma aleatoria	*/
NodeInfo* chooseNode(list < NodeInfo >& tree) {
	unsigned seed = time(0);
	srand(seed);
	int i = rand() % tree.size(); // Escolhe um indice aleatorio da arvore
	int j = 0;

	/* Escolhemos um vértice aleatorio para iniciar o problema */
	list < NodeInfo >::iterator it;
	Graph graph(tree.size(), &tree);
	NodeInfo* chosenNode;
	for(it = tree.begin(); it != tree.end(); ++it) {

		if(j == i) {

			chosenNode = graph.DFS(&(*it));
			break;
		}
		j++;
	}
	
	return chosenNode;

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
		auto node = chooseNode(tree);
		
		if(node->lower_bound > upper_bound) {
			
			continue;
		}
		break;
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
