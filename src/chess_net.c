/**
 * Project:  neural chess
 * Author:   Jakub Urbanek
 * Year:     2022
 */

#include "chess_net.h"
#include "fcnn.h"
#include "neuron.h"

#include <stdlib.h>

#define PREPR_NEURONS_COUNT 64  //64 pieces
#define PREPR_NEURON_INP_COUNT 12

TchNet* initRandChNet(int fcnnLayerCount, const int* fcnnNeuronsInLayersCount)
{
  if(fcnnNeuronsInLayersCount[0] != PREPR_NEURONS_COUNT){
    return NULL;
  }
  
  TchNet* net = malloc(sizeof(TchNet));
  net->preprocessingNeurons = malloc(PREPR_NEURONS_COUNT * sizeof(Tneuron*));
  for(int i = 0; i < PREPR_NEURONS_COUNT; ++i){
    net->preprocessingNeurons[i] = initRandNeuron(PREPR_NEURON_INP_COUNT,
                                                  MIN_RAND_WEIGHT,
                                                  MAX_RAND_WEIGHT);
  }

  net->fcnn = initRandfcnn(fcnnLayerCount, fcnnNeuronsInLayersCount);

  return net;
}

void freeChNet(TchNet* net)
{
  for(int i = 0; i < PREPR_NEURONS_COUNT; ++i){
    freeNeuron(net->preprocessingNeurons[i]);
  }
  free(net->preprocessingNeurons);

  freefcnn(net->fcnn);
  
  free(net);
}

void printChNet(const TchNet* n)
{
  fprintChNet(stdout, n);
}

void fprintChNet(FILE* out, const TchNet* n)
{
  for(int i = 0; i < PREPR_NEURONS_COUNT; ++i){
    fprintNeuron(out, n->preprocessingNeurons[i]);
  }
  fprintfcnn(out, n->fcnn);
}

TchNet* getChNet(void)
{
  return fgetChNet(stdin);
}

TchNet* fgetChNet(FILE* in)
{
  TchNet* net = malloc(sizeof(TchNet));
  net->preprocessingNeurons = malloc(PREPR_NEURONS_COUNT * sizeof(Tneuron*));
  for(int i = 0; i < PREPR_NEURONS_COUNT; ++i){
    
    net->preprocessingNeurons[i] = fgetNeuron(in);
    
    if(net->preprocessingNeurons[i] == NULL){
      for(int j = 0; j < i; ++j){
        freeNeuron(net->preprocessingNeurons[j]);
      }
      free(net->preprocessingNeurons);
      free(net);

      return NULL;
    }
  }

  net->fcnn = fgetfcnn(in);

  if(net->fcnn == NULL){
    for(int i = 0; i < PREPR_NEURONS_COUNT; ++i){
      freeNeuron(net->preprocessingNeurons[i]);
    }
    free(net->preprocessingNeurons);

    free(net);

    return NULL;
  }

  return net;
}

float chNetPredict(const TchNet* net, const char* posString)
{
  #warning "chNetPredict not implemented"
}


TchNet* chNetSex(const TchNet* dad, const TchNet* mum, int mutationRareness)
{
  #warning "chNetSex not implemented"
}