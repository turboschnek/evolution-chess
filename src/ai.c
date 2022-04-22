/**
 * Project:  neural chess
 * Author:   Jakub Urbanek
 * Year:     2022
 */

#include "ai.h"
#include "chess_net.h"
#include "chess_logic.h"
#include "chess_structs.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>


#define MAX_MINIMAX_DEPTH 20

#define MINIMAX_WIN_EVAL_COEF 100000

void chNetEvolution()
{
  const int maxGeneration = 100;  // max number of generations in simulation
  const int populationCount = 100;  // number of networks in population
  const int mutationRareness = 100;  // 1 in mutationRareness neurons gets randomized

  const int netStruct[4] = {64, 200, 100, 1};
  const int netStructLayerCount = sizeof(netStruct) / sizeof(*netStruct);

  const int tournamentRounds = 2;
  const float tournamentMoveTime = 0.1; 

  TchNet** population = malloc(populationCount * sizeof(TchNet*));
  for(int i = 0; i < populationCount; ++i){
    population[i] = initRandChNet(netStructLayerCount, netStruct);
  }


  for(int i = 0;
      (i < maxGeneration);
      ++i){
    quickTournament(population, populationCount,
                    tournamentRounds, tournamentMoveTime);

    int elderyCount = populationCount/2;
    for(int j = elderyCount; j < populationCount; ++j){
      freeChNet(population[j]);

      population[j] = chNetSex(population[(j - elderyCount) % elderyCount],
                                     population[(j+1) % elderyCount],
                                     mutationRareness);
    }
  }


  for(int i = 0; i < populationCount; ++i){
    const char filename[11] = {'s', 'a', 'v', 'e', '_', i, '.', 't', 'x', 't', '\0'};
    FILE* file = fopen(filename, "w");
    if(file != NULL){
      fprintChNet(file, population[i]);
    }
    fclose(file);

    freeChNet(population[i]);
  }
}


void quickTournament(TchNet** population, int populationCount, int rounds,
                     float timeForMove)
{
  float* keys = calloc(populationCount, sizeof(float));

  for(int round = 0; round < rounds; ++round){
    shufflePopulationWithKeys(population, keys, populationCount);
    for(int i = 0; i < populationCount; i += 2){
      switch (game(population[i], population[i+1], timeForMove)){
        case 0:  //draw
          keys[i] += 0.3;
          keys[i+1] += 0.3;
          break;
        
        case 1: //win of white
          keys[i] += 1;
          break;

        case -1:  //win of black
          keys[i+1] += 1;
          break;

        default:
          fprintf(stderr, "unpredictable error in function quickTournament");
          return;
      }
    }
  }
  

  sortPopulation(population, keys, populationCount, true);

  free(keys);
}

void sortPopulation(TchNet** population, float *keys, int populationCount, bool increasing)
{
  if(increasing){
    for(int i = 1; i < populationCount; ++i){
      int j;
      float tempKey = keys[i];
      TchNet* tempNet = population[i];

      for(j = i-1; (j >= 0 && keys[j] > tempKey); --j){
        population[j+1] = population[j];
        keys[j+1] = keys[j];
      }
      population[j+1] = tempNet;
      keys[j+1] = tempKey;
    }
  } else {
    for(int i = 1; i < populationCount; --i){
      int j;
      float tempKey = keys[i];
      TchNet* tempNet = population[i];

      for(j = i-1; (j >= 0 && keys[j] < tempKey); --j){
        population[j+1] = population[j];
        keys[j+1] = keys[j];
      }
      population[j+1] = tempNet;
      keys[j+1] = tempKey;
    }
  }
}

void shufflePopulationWithKeys(TchNet** population, float* keys,
                               int populationCount)
{
  for(int i = 0; i < populationCount; ++i){
    int index = rand() % populationCount;

    TchNet* tempNet = population[i];
    float tempkey = keys[i];

    population[i] = population[index];
    keys[i] = keys[index];

    population[index] = tempNet;
    keys[index] = tempkey;
  }
}


int game(const TchNet* white, const TchNet* black, float timeBudget)
{
  Tboard *b = initBoard();


  char *moveBuffer = malloc(MAX_INP_LEN * sizeof(char));
  int result = 2;
  while(result == 2)
  {
    if(b->move%2 == 0){
      //white`s move

      minimax(b, white, timeBudget, moveBuffer);

    } else {
      //black`s move

      minimax(b, black, timeBudget, moveBuffer);
    
    }
  
    moveBoard(moveBuffer, b);

    result = getResult(b);
  }

  freeBoard(b);
  free(moveBuffer);
  return result;
}

int minimax(Tboard *b, const TchNet* net, float seconds, char *output)
{
  TmoveList *ml = initMoveList(16);
  generateAllPossibleMoves(b, ml);

  // no move possible
  if(ml->filled < 1){
    strcpy(output, (char[5]){'n', 'o', 'm', 'o', '\0'});
    freeMoveList(ml);
    return -1;
  }


  int maxDepth = MAX_MINIMAX_DEPTH, startDepth = 1, depthStep = 1,
      timeBudget = seconds * 1000000;
  float depthTimeCoeff = 0.5;
  bool isBlack = (((b->move+1) % 2) == 0),
       isInTime = true,
       interrupted = false;
  
  clock_t startTime = clock();


  int depth = startDepth;
  for(; depth <= maxDepth && isInTime; depth += depthStep){
    
    float *keys = malloc(ml->filled * sizeof(int));
    
    for(int i = 0; i < ml->filled && isInTime; i++){
      Tboard *bCopy = copyBoard(b);
      moveBoard(ml->moves[i], bCopy);

      keys[i] = innerMinimax(bCopy, net, depth, isBlack, -INF, INF);

      freeBoard(bCopy);

      isInTime = !(startTime + timeBudget < clock());
      interrupted = !isInTime;
    }

    if(!interrupted) sortMoveList(ml, keys, isBlack);

    free(keys);

    isInTime = !((startTime + timeBudget/
                  (depthTimeCoeff * pow(10, depthStep))) < clock());
  }
  strcpy(output, ml->moves[0]);
  freeMoveList(ml);

  if(interrupted) depth -= depthStep;
  
  if(!isInTime) depth -= depthStep;

  return depth+1;
}


float innerMinimax(Tboard *b, const TchNet* net, int depth, bool isMax, float alfa, float beta)
{
  if(depth == 0){
    return evaluateBoard(b, net);
  }

  TmoveList *ml = initMoveList(8);
  generateAllPossibleMoves(b, ml);

  int result = getResultFaster(b, ml);
  if(result < 2){
    freeMoveList(ml);

    // *(depth+1) for faster checkmates
    return result * (MINIMAX_WIN_EVAL_COEF * (depth+1));
  }

  if(isMax){
    float max = -INF;

    for (int i = 0; i < ml->filled; i++){
      Tboard *copy = copyBoard(b);
      moveBoard(ml->moves[i], copy);

      
      max = fmax(max, innerMinimax(copy, net, depth-1, false, alfa, beta));
      freeBoard(copy);

      alfa = fmax(alfa, max);
      if(beta <= alfa){
        freeMoveList(ml);
        return max;
      }
    }
    freeMoveList(ml);
    return max;
  }else{
    float min = INF;

    for (int i = 0; i < ml->filled; i++){
      Tboard *copy = copyBoard(b);
      moveBoard(ml->moves[i], copy);

      
      min = fmin(min, innerMinimax(copy, net, depth-1, true, alfa, beta));
      freeBoard(copy);

      beta = fmin(beta, min);
      if(beta <= alfa){
        freeMoveList(ml);
        return min;
      }
    }
    freeMoveList(ml);
    return min;
  }
}

void sortMoveList(TmoveList* ml, float *keys, bool increasing)
{
  if(increasing){
    for(int i = 1; i < ml->filled; i++){
      int j;
      float tempKey = keys[i];
      char *tempMove = ml->moves[i];

      for(j = i-1; (j >= 0 && keys[j] > tempKey); j--){
        ml->moves[j+1] = ml->moves[j];
        keys[j+1] = keys[j];
      }
      ml->moves[j+1] = tempMove;
      keys[j+1] = tempKey;
    }
  } else {
    for(int i = 1; i < ml->filled; i++){
      int j;
      float tempKey = keys[i];
      char *tempMove = ml->moves[i];

      //keys[j] <= tempKey   would be stable, but slower
      for(j = i-1; (j >= 0 && keys[j] < tempKey); j--){
        ml->moves[j+1] = ml->moves[j];
        keys[j+1] = keys[j];
      }
      ml->moves[j+1] = tempMove;
      keys[j+1] = tempKey;
    }
  }
}

float evaluateBoard(const Tboard* b, const TchNet* net)
{
  #warning function evaluateBoard not implemented
}