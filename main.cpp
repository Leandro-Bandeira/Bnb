#include <iostream>
#include <algorithm>
using namespace std;

#include "data.h"
#include "hungarian.h"
#include <vector>

int main(int argc, char** argv) {

	Data * data = new Data(argc, argv[1]);
	data->readData();

	double **cost = new double*[data->getDimension()];
	for (int i = 0; i < data->getDimension(); i++){
		cost[i] = new double[data->getDimension()];
		for (int j = 0; j < data->getDimension(); j++){
			cost[i][j] = data->getDistance(i,j);
		}
	}

	hungarian_problem_t p;
	int mode = HUNGARIAN_MODE_MINIMIZE_COST;
	hungarian_init(&p, cost, data->getDimension(), data->getDimension(), mode); // Carregando o problema

	double obj_value = hungarian_solve(&p);
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
	delete data;
	
	/* Vamos percorrer todos as linhas e para cada linha vamos achar todos os subtours, ou seja até i = j	*/
	
	int alocTask;
	int index_subtour = 0;
	vector < vector < int > > subtours;
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
				subtours.push_back(subtour_i);
				break;
			}
		
		}

		
		subtour_i.clear();
		index_subtour += 1;
		it += 1;

	}
	
	sort(subtours.begin(), subtours.end(), [=](auto  A, auto B){
			return A.size() >  B.size();
	}
	);

	for(auto line : subtours) {
		
		for(int i = 0; i < line.size(); i++) {

			cout << line[i] << " ";
		}
		cout << endl;
			
	}
	return 0;
}
