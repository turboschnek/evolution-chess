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

#define PRIMITIVE_PIECE_VALUE_ENDGAME_THRESHOLD 15

void chNetEvolution()
{
  const int maxGeneration = 20;  // max number of generations in simulation
  const int populationCount = 100;  // number of networks in population
  const int mutationRareness = 100;  // 1 in mutationRareness neurons gets randomized

  const int netStruct[4] = {64, 200, 100, 1};
  const int netStructLayerCount = sizeof(netStruct) / sizeof(*netStruct);

  const int tournamentRounds = 1;
  const float tournamentMoveTime = 0.01; 

  TchNet** population = malloc(populationCount * sizeof(TchNet*));
  for(int i = 0; i < populationCount; ++i){
    population[i] = initRandChNet(netStructLayerCount, netStruct);
  }


  for(int i = 0;
      (i < maxGeneration);
      ++i){
    
    printf("generation: %d\n", i);

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


  if(canAnyoneBeatPrimitiveEval(population, populationCount)){
    printf("someone is better than primitive eval");
  }
  
  for(int i = 0; i < populationCount; ++i){
    char filename[15] = {'s', 'a', 'v', 'e', '_', '_', '_', '_', '.', 't', 'x', 't', '\0'};
    int a = 0;
    for(int j = i+1; j != 0; j = j/10){
      filename[7 - a] = '0' + j%10;
      ++a;
    }
    FILE* file = fopen(filename, "w");
    if(file != NULL){
      fprintChNet(file, population[i]);
    }
    fclose(file);

    freeChNet(population[i]);
  }

  free(population);
}


void quickTournament(TchNet** population, int populationCount, int rounds,
                     float timeForMove)
{
  float* keys = calloc(populationCount, sizeof(float));

  for(int round = 0; round < rounds; ++round){
    shufflePopulationWithKeys(population, keys, populationCount);
    for(int i = 0; i < populationCount; i += 2){
      printf("game: %d ", i/2);
      switch (game(population[i], population[i+1], timeForMove)){
        case 0:  //draw
          printf("draw\n");
          keys[i] += 0.3;
          keys[i+1] += 0.4;
          break;
        
        case 1: //win of white
          printf("white\n");
          keys[i] += 0.7;
          break;

        case -1:  //win of black
          printf("black\n");
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

bool canAnyoneBeatPrimitiveEval(TchNet** population, int populationCount)
{
  for(int i = 0; i < populationCount; ++i){
    if(game(population[i], NULL, 0.01) == 1){
      printf("net %d won as white\n", i);
      if(game(NULL, population[i], 0.01) == -1){
        printf("net %d won as black\n", i);
        return true;
      }
    } else {
      printf("net %d lost\n", i);
    }
  }
  return false;
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

      isInTime = !(startTime + timeBudget < clock()) || depth == startDepth;
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
  if(net == NULL){
    return primitiveEval(b);
  }

  char* posString = boardToPosString(b);
  float evaluation = chNetPredict(net, posString);
  free(posString);
  return evaluation;
}

float primitiveEval(const Tboard *b)
{
  float sum = 0;
  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 8; j++){
      sum += (float) getPieceValue(b->pieces[i][j], i, j, b->pieceCount) * 0.1;
    }
  }
  return sum;
}

int getPieceValue(char piece, int row, int col, int pieceCount)
{
  switch(piece)
  {
  
  // value of king:
  // in the beginning is highest in corner
  // in endgame is highest in the centre
  case 'k':
    if(pieceCount > PRIMITIVE_PIECE_VALUE_ENDGAME_THRESHOLD){
      if(row == 7)
        return (abs(35 - (col*10))) / 2;
      return 0;
    }
    return -((abs(35 - (col*10)) + abs(35 - (row*10))) / 2);
  case 'K':
    if(pieceCount > PRIMITIVE_PIECE_VALUE_ENDGAME_THRESHOLD){
      if(row == 0)
        return -(abs(35 - (col*10))) / 2;
      return 0;
    }
    return ((abs(35 - (col*10)) + abs(35 - (row*10))) / 2);

  case 'q':
    return 900;
  case 'Q':
    return -900;
  case 'b':
    return 300;
  case 'B':
    return -300;
  case 'n':
    // forces knights to be in the middle, where it is ideal for them
    return 300 - ((abs(35 - (col*10)) + abs(35 - (row*10))) / 10);
  case 'N':
    return -(300 - ((abs(35 - (col*10)) + abs(35 - (row*10))) / 10));
  case 'r':
    return 500;
  case 'R':
    return -500;
  case 'p':
    // pawns are most valuable in middle cols and near promotion
    return 100 + (7-row) - (abs(35 - (col*10)) / 10);
  case 'P':
    return -(100 + row - (abs(35 - (col*10)) / 10));
  default:
    return 0;
  }
}
