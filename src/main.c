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
  const int netStruct[3] = {64, 12, 1};
  const int netStructLayerCount = sizeof(netStruct) / sizeof(*netStruct);

  TchNet* net = initRandChNet(netStructLayerCount, netStruct);
  if(net == NULL){
    fprintf(stderr, "unable to init chNet");
    return;
  }

  FILE* file;
  file = fopen("temp.txt", "w");
  fprintChNet(file, net);
  fclose(file);

  file = fopen("temp.txt", "r");
  TchNet* net2 = fgetChNet(file);
  fclose(file);
  if(net2 == NULL){
    fprintf(stderr, "unable to get chNet from temp.txt");
    return;
  }

  file = fopen("temp2.txt", "w");
  fprintChNet(file, net2);
  fclose(file);

  freeChNet(net);
  freeChNet(net2);
}


int main(){
  srand(time(NULL));

  chessNetworkTest();
}