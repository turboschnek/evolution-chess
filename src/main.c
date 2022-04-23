/**
 * Project:  neural chess
 * Author:   Jakub Urbanek
 * Year:     2022
 */

#include "ai.h"
#include "chess_net.h"
#include "chess_logic.h"
#include "chess_structs.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>



void chessNetworkTest()
{
  const int netStruct[4] = {64, 200, 100, 1};
  const int netStructLayerCount = sizeof(netStruct) / sizeof(*netStruct);

  TchNet* mum = initRandChNet(netStructLayerCount, netStruct);
  if(mum == NULL){
    fprintf(stderr, "unable to init chNet");
    return;
  }

  TchNet* dad = initRandChNet(netStructLayerCount, netStruct);
  if(dad == NULL){
    fprintf(stderr, "unable to init chNet");
    return;
  }

  TchNet* kid = chNetSex(dad, mum, 0);

  printf("%f", chNetPredict(kid, "pPkKnNbBrRqQ     pPkKnNbBrRqQpPkKnNbBrRqQ  pPkKnNbBrRqQpPkKnNbBp"));

  freeChNet(kid);
  freeChNet(mum);
  freeChNet(dad);
}

void chessLogicBasicTest()
{
  Tboard* b = initBoard();
  TmoveList* ml = initMoveList(10);

  generateAllPossibleMoves(b, ml);

  for(int i = 0; i < ml->filled; ++i){
    printf("%s\n", ml->moves[i]);
  }
  
  freeMoveList(ml);
  freeBoard(b);
}


int main(){
  srand(time(NULL));

  //chessNetworkTest();

  //chessLogicBasicTest();

  chNetEvolution();
}
