#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_ITERATIONS 1000
#define ABSTEMP 0.00001
#define COOLING 0.99999
#define TEMPERATURE 1000

//Thread Pool Size
#define POOL 8

// #define DEBUG

// dimension
int dimension;

//Distance Matrix
double **matrix;

//Best Tour
int *bestTour;

//Current and Next Tour
int *currentTour;
int *nextTour;

//Temperature
double temperature;

//Mutex - multithread

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void printTour(int *tour)
{
      int i;
      printf("TOUR: ");
      for (i = 0; i < dimension; i++)
            printf("%d ", tour[i]);
      printf("\n");
}

void printMatrix()
{
      int i,j;
      
      for (i = 0; i < dimension; i++)
      {
            for (j = 0; j < dimension; j++)
                  printf("%lf ", matrix[i][j]);
            printf("\n");
      }
}

double** readMatrix(char* name)
{
      FILE *file;
      double **matrix;
      double **coord;
      int i,j;
      char buf[100];//read buffer
      dimension = 0;

      file = fopen(name, "r");

      while (fscanf(file," %[^\n]s", buf) > 0 && !(buf[0] == 'D' && buf[2] == 'M'));
      dimension = atoi(&buf[12]);

      #ifdef DEBUG
      printf("Dimension: %d\n", dimension);
      #endif

      while (fscanf(file," %[^\n]s", buf) > 0 && !(buf[0] == 'N' && buf[1] == 'O'));

      coord = (double **)malloc(dimension*sizeof(double*));
      for(i=0;i<dimension;i++) 
            coord[i] = (double*) malloc(2*sizeof(double));
      for(i=0;i<dimension;i++)
            fscanf(file, "\n %*[^ ] %lf %lf", &coord[i][0], &coord[i][1]);

      matrix = (double **)malloc(sizeof(double*)*(dimension));
      for (i = 0; i < dimension; i++)
            matrix[i] = (double *)malloc(sizeof(double)*(dimension));
      for (i = 0; i < dimension; i++)
      {
            for (j = i + 1 ; j < dimension; j++)
            {
                  matrix[i][j] = sqrt(pow(coord[i][0] - coord[j][0],2) + pow(coord[i][1] - coord[j][1],2)); //without round
                  matrix[j][i] = matrix[i][j];
            }
      }
      free(coord);
      return matrix;
}

double tourCost(int *tour)
{
      int i;
      double value = 0.0;
      
      for (i = 0; i < dimension - 1; i++)
            value += matrix[tour[i]][tour[i+1]];
      
      return value + matrix[tour[0]][tour[dimension-1]];
}

int* nearestNeighbour()
{
      int *tour;
      int *visited;
      double nearestDistance;
      int nearestIndex;
      int i,j;
      
      visited = (int*) malloc(dimension*sizeof(int));
      tour    = (int*) malloc(dimension*sizeof(int));
      for(i=0;i<dimension;i++) visited[i] = 0;
      
      int start; 
      
      pthread_mutex_lock(&mutex);
      start = rand()%dimension;
      pthread_mutex_unlock(&mutex);
      
      visited[start] = 1;
      tour[0] = start;
      
      for(i=1;i<dimension;i++)
      {
            nearestDistance = FLT_MAX;
            nearestIndex = 0;
            for(j=0;j<dimension;j++)
            {
                  if(!visited[j] && matrix[tour[i-1]][j] < nearestDistance)
                  {
                        nearestDistance = matrix[tour[i-1]][j];
                        nearestIndex = j;
                  }
            }
            tour[i] = nearestIndex;
            visited[nearestIndex] = 1;
      }
      
      free(visited);
      return tour;
}

void swap(int *currentTour, int *nextTour)
{
      int i,aux;

      for (i = 0; i < dimension; i++) nextTour[i] = currentTour[i];

      pthread_mutex_lock(&mutex);	
      int first = (rand() % (dimension - 1)) + 1;
      int second = (rand() % (dimension - 1)) + 1;
      pthread_mutex_unlock(&mutex);

      aux = nextTour[first];
      nextTour[first] = nextTour[second];
      nextTour[second] = aux;
}

void* SimulatedAnnealing(void *id)
{
      int *currentTour;
      int *nextTour;

      double distance;
      double delta;
      int i;

      currentTour = nearestNeighbour();

      distance = tourCost(currentTour);

      nextTour = (int *)malloc(sizeof(int)*dimension);

      while (temperature > ABSTEMP)
      {
            swap(currentTour, nextTour);
            delta = tourCost(nextTour) - distance;
            if (((delta < 0) || (distance > 0)) && (exp(-delta/temperature) > (double)rand()/RAND_MAX))
            {
                  for (i = 0; i < dimension; i++)
                        currentTour[i] = nextTour[i];
                  distance = delta + distance;
            }

            pthread_mutex_lock(&mutex);
            temperature *= COOLING;
            pthread_mutex_unlock(&mutex);

      }

      pthread_mutex_lock(&mutex);
      if (tourCost(bestTour) > tourCost(currentTour))
      {
            for (i = 0; i < dimension; i++)
                  bestTour[i] = currentTour[i];
      }
      pthread_mutex_unlock(&mutex);

      return(NULL);
}

int main(int argc, char **argv)
{
      int i;
      void *status;

      pthread_t *threads;
      threads = (pthread_t *)malloc(sizeof(pthread_t)*POOL);

      temperature = TEMPERATURE; //init value for temperature
      matrix = readMatrix(argv[1]);

      bestTour = (int *)malloc(sizeof(int)*dimension);
      
      //first tour
      for (i = 0; i < dimension;i++) bestTour[i] = i;
      
      #ifdef DEBUG
      printTour(bestTour);
      #endif
      
      pthread_mutex_init(&mutex,NULL);

      for (i = 0; i < POOL; i++)
            if (pthread_create(&threads[i], NULL, SimulatedAnnealing,(void*)(intptr_t)i))
                  exit(-1);

      for (i = 0; i < POOL; i++)
            if (pthread_join(threads[i], &status))
                  exit(-1);
      
      pthread_mutex_destroy(&mutex);
      
      #ifdef DEBUG
      printTour(bestTour);
      #endif
      
      printf("Best Distance: %lf\n", tourCost(bestTour));

      return 0;
}