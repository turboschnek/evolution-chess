/**
 * Project:  neural chess
 * Author:   Jakub Urbanek
 * Year:     2022
 */

#include "chess_net.h"

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


int main(){
  srand(time(NULL));

  chessNetworkTest();
}
