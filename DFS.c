class Graph {
	

	public:
		Graph(int size, list < NodeInfo > *tree);
		NodeInfo* DFS(NodeInfo* vertex);
		void addVertexAdj(NodeInfo* vertex);
		~Graph();
	
	private:

		int numVertices;
		list < NodeInfo* > *adjList;
		vector < NodeInfo* > *visited; // Indica pelo indice se foi visitado ou não
		list < NodeInfo > *tree; // Aponta para a árvore do problema


	
};


Graph::Graph(int size, list < NodeInfo > *tree) {
	int numVertices = size;
	this->tree = tree;
	adjList = new list < NodeInfo* >[size];
	visited = new vector < NodeInfo* > [size];
	
}


void Graph::addVertexAdj(NodeInfo* vertex) {
	
	
	for(list<NodeInfo>::iterator vertexA  = (*tree).begin(); vertexA != (*tree).end(); ++vertexA) {
		
	
		bool verification_visited = false;
		/* Verifica se o vértice vizinho já foi visitado ou não	*/
		for(auto visitedVertex : *visited) {
			if(visitedVertex->forbidden_arcs == vertexA->forbidden_arcs) {
				verification_visited = true;
			} 
		} 
		/* Verifica se os arcos proibidos deles são iguais, dizendo que são adjacentes	*/

		bool verification_equal_arcs = false;
		
		
		for(int i = 0; i < vertex->forbidden_arcs.size(); i++) {
			
			int first = vertex->forbidden_arcs[i].first;
			int second = vertex->forbidden_arcs[i].second;
			
			int flag = 0;
			for(int j = 0; j < vertexA->forbidden_arcs.size(); j++) {

				int firstA = vertexA->forbidden_arcs[j].first;
				int secondA = vertexA->forbidden_arcs[j].second;

				if(firstA == first || firstA == second) {

					flag++;
				} 
				if(secondA == first || secondA == second) {

					flag++;
				}
			}

			if(flag == 2) {

				verification_equal_arcs = true;
				break;
			}
			
		}

		
		if(verification_equal_arcs and !verification_visited ) {
		
			/* Por fim verificamos se o vértice está ou não na lista de adjacentes	*/
			bool verification_adjVertex = false;

			for(auto adjVertex : *adjList) {

				if(adjVertex->forbidden_arcs == vertexA->forbidden_arcs) {

					verification_adjVertex = true;
				}
			}
			
			if(!verification_adjVertex) {
				cout << "adicionou" << endl;
				adjList->push_back(&(*vertexA));
			}
		}

	}
}


/* Inicializa a partir de um nó aleatorio que chamaremos de root	*/
NodeInfo* Graph::DFS(NodeInfo *root) {
	
	cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<Inicio do DFS>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
	cout << "Root escolhido: ";

	for(int i = 0; i < root->forbidden_arcs.size(); i++) 
			cout << root->forbidden_arcs[i].first << " " << root->forbidden_arcs[i].second << " ";

	cout << endl;

	visited->push_back(root); // Adiciona o vértice a aqueles que foram visitados
	addVertexAdj(root);
	
	cout << "tamanho da lista adjacente: " << adjList->size() << endl;
	while(!adjList->empty()) {
		/* Pega o primeiro elemento da lista adjacente e então coloca-o como últomo na lista de visitados	*/
		NodeInfo* nodeLast = adjList->front();
		visited->push_back(nodeLast);
		
		
		cout << "Lista adjacente: ";

		for(auto adj : *adjList) {

			for(int i = 0; i < adj->forbidden_arcs.size(); i++) {
				cout << "(" << adj->forbidden_arcs[i].first << " " << adj->forbidden_arcs[i].second << " ";
			}
			cout << ") " << endl;
		}
		adjList->pop_front(); // Remove da lista o primeiro elemento
		addVertexAdj(nodeLast);

		
		cout << endl;

		cout << "Lista de Visitados: ";
		for(auto visit : *visited) {
			
			for(int i = 0; i < visit->forbidden_arcs.size(); i++)
				cout << "(" << visit->forbidden_arcs[i].first << " " << visit->forbidden_arcs[i].second << " ";

			cout << ") " << endl;
		}
		
		cout << endl;
	}
	
	cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<End do DFS>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
	return visited->back();
	



}

Graph::~Graph() {
	
	delete[] visited;

	delete[] adjList;

}
