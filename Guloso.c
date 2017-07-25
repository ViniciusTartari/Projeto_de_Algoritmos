#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//defines para teste(pre-processamento)
#define DEBUG
#define VERBOSE
#define V1
// #define V2
#define route
typedef struct nCity 
{
      int id;
      float distance;
} NextCity;

//Info da cidade
typedef struct city 
{
      float x, y;
      NextCity *nextCities;
} City;

City *map;                    //mapa de cidades
NextCity *tmpMerge;           //pro merge

#ifdef V1
int path[2000];                    //cidade 
int pastPath[2000];                //cidades visitadas
#endif

#ifdef V2
int *path;                    //cidade 
int *pastPath;                //cidades visitadas
#endif
float lengthPath;             //tamanho vetor
 
void printMap(int dimension) 
{
      int i, j;
      for (i = 0 ; i < dimension; i++) {
            printf("Cidade %d: x: %.2f - y: %.2f\n", i + 1, map[i].x, map[i].y);
            for (j = 0; j < dimension; j++) {
                  printf ("%d:  %.2f\t", map[i].nextCities[j].id, map[i].nextCities[j].distance);
            }
            printf("\n");
      }
}
//intercala e merge - acertar variaveis (alg2)
void intercala (int p, int q, int r, NextCity v[]) //p inf, q sup, r dimensao
{
      int i, j, k;

      for (i = p; i < q; ++i)  
            tmpMerge[i-p] = v[i];
      for (j = q; j < r; ++j)  
            tmpMerge[r-p+q-j-1] = v[j];
      i = 0; 
      j = r-p-1;
      for (k = p; k < r; ++k)
            if (tmpMerge[i].distance <= tmpMerge[j].distance)
                  v[k] = tmpMerge[i++];
            else 
                  v[k] = tmpMerge[j--];
}

void mergesort (int p, int r, NextCity v[])
{
      if (p < r-1) {
            int q = (p + r)/2;
            mergesort (p, q, v);
            mergesort (q, r, v);
            intercala (p, q, r, v);
      }
}

int input_info(void) 
{
      char buf[100];//tamanho palavra Leitura, 100 ja da
      int dimension = 0, i, j, tmp;
      float distX, distY;

      // Le ate encontrar a palavra DIMENSION
      while (scanf(" %[^\n]s", buf) > 0 && !(buf[0] == 'D' && buf[2] == 'M'));

      // Conversao de caractere para inteiro
      dimension = atoi(&buf[12]);

      #ifdef DEBUG
      printf("Dimensao: %d\n", dimension);
      #endif

      // Alocando mapa de posicoes
      map = (City*) malloc((dimension) * sizeof(City));
      if (!map) 
            return -1;

      // Le ate encontrar a palavra NODE_COORD_TYPE
      while (scanf(" %[^\n]s", buf) > 0 && !(buf[0] == 'N' && buf[1] == 'O'));

      // Le posicao das cidades 1 e 2
      scanf("%d %f %f", &tmp, &map[0].x, &map[0].y);
      scanf("%d %f %f", &tmp, &map[1].x, &map[1].y);

      // Aloca lista de cidades proximas de 1
      map[0].nextCities = (NextCity*) malloc((dimension - 1) * sizeof(NextCity));
      if (!map[0].nextCities) 
            return -1;

      // Calcula a distancia euclidiana entre
      // as cidades 1 e 2
      distX = pow((map[0].x-map[1].x),2);
      distY = pow((map[0].y-map[1].y),2);
      // Atribui id da cidade proxima
      map[0].nextCities[0].id = 2;

      // Armazena a distancia entre 1 e 2
      map[0].nextCities[0].distance = sqrt(distX + distY);

      // Le todas as cidades restantes
      // e calcula as distancias entre
      // a cidade 1 e estas
      for (i = 2; i < dimension; i++) 
      {
            // Aloca lista de cidades proximas a i-1
            map[i - 1].nextCities = (NextCity*) malloc((dimension) * sizeof(NextCity));
            if (!map[i - 1].nextCities) 
                  return -1;

            // Le e armazena posicao da cidade i
            scanf("%d %f %f", &tmp, &map[i].x, &map[i].y);
      }

      // Aloca lista de cidades proximas a ultima cidade
      map[dimension - 1].nextCities = (NextCity*) malloc((dimension) * sizeof(NextCity));
      if (!map[dimension - 1].nextCities) 
            return -1;

      // Alocacao de buffer usado no Merge Sort
      tmpMerge = (NextCity*) malloc((dimension - 1) * sizeof(NextCity));

      // Calcula as distancias entre o restante das cidades
      for (i = 0; i < dimension; i++) 
      {
            for (j = i; j < dimension; j++) 
            {
                  //#ifdef DEBUG
                  //printf("Cidade %d\n", j);
                  //#endif

                  // Calcula a distancia euclidiana entre
                  // as cidades i e j
                  distX = pow((map[i].x-map[j].x),2);
                  distY = pow((map[i].y-map[j].y),2);

                  // Atribui id da cidade proxima
                  map[i].nextCities[j].id = j + 1;
                  map[j].nextCities[i].id = i + 1;

                  // Armazena a distancia entre i e j
                  map[i].nextCities[j].distance = sqrt(distX + distY);
                  map[j].nextCities[i].distance = map[i].nextCities[j].distance;
            }
            // Ordenacao por mergesort das cidades proximas de i
            mergesort(0, dimension, map[i].nextCities);
            map[i].nextCities = &(map[i].nextCities[1]);
      }
      return dimension;
}

int smallestPathGreedy(int dimension, int obj) 
{
      #ifdef V2
      // Vetor do caminho de cidades
      path = (int*) malloc((dimension) * sizeof(int));
      // 
      // Vetor de cidades ja visitadas
      pastPath = (int*) calloc((dimension + 1), sizeof(int));
      #endif
      // Contador do laco
      int i, j;

      if (obj < 1) 
            return 0;

      // Armazena primeira cidade no caminho
      path[0] = 1;
      // Marca primeira cidade como ja visitada
      pastPath[1]++;

      lengthPath = 0;

      // Ate que a cidade atual seja a de chegada
      for (i = 0; i < dimension; i++) 
      {
            while (pastPath[map[path[i]-1].nextCities[0].id]) 
                  map[path[i]-1].nextCities = &(map[path[i]-1].nextCities[1]);
            path[i+1] = map[path[i]-1].nextCities[0].id;
            pastPath[map[path[i]-1].nextCities[0].id]++;
            lengthPath += map[path[i]-1].nextCities[0].distance;
      }

      path[i] = 1;
      for (j = 0; j < dimension - 1 && map[1].nextCities[j].id != path[i-1]; j++);
            lengthPath += map[1].nextCities[j].distance;

      return ++i;
}

int main(int argc, char *argv[])
{
      // Criacao de variaveis
      int i, size, dimension;

      // Leitura e montagem da matriz de distancias
      dimension = input_info();

      // Erro na alocacao de memoria
      if (dimension == -1) {
            printf("Erro na alocacao de memoria!!!\n");
            return 1;
      }
      #ifdef VERBOSE
      // Imprime mapa de distancias ordenadas
      printf("Cidades proximas ordenadas:\n");
      printMap(dimension);
      #endif

      // Constroi menor caminho guloso e retorna seu tamanho
      size = smallestPathGreedy(dimension, 1);

      // Caminho de tamanho 0 não existe
      if (!size) {
            printf("Nao existe caminho! :(\n");
            return 1;
      }

      //Imprime cidades do menor caminho e seu tamanho
      printf("Distancia %f\n", lengthPath);
      #ifdef route
      //printf("Rota:");
      for (i = 0; i < size; i++)
            printf("%d ", path[i]);
      printf("\n");
      #endif
      return 0;
}