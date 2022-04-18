/**
 * Project:  neural chess
 * Author:   Jakub Urbanek
 * Year:     2022
 */

#include "chess_net.h"
#include "fcnn.h"
#include "neuron.h"

#include <stdlib.h>
#include <math.h>

#define PREPR_NEURONS_COUNT 64  // 64 pieces
#define PREPR_NEURON_INP_COUNT 12  // 12 possible pieces

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
  int i;
  float* fcnnInputs = malloc(PREPR_NEURONS_COUNT * sizeof(float));
  
  for(i = 0; i < PREPR_NEURONS_COUNT; ++i){
    float* preprNeuronIputs = calloc(PREPR_NEURON_INP_COUNT, sizeof(float));
    
    switch(posString[i]) {
      case 'p': preprNeuronIputs[0] = 1.0; break;
      case 'P': preprNeuronIputs[1] = 1.0; break;
      case 'k': preprNeuronIputs[2] = 1.0; break;
      case 'K': preprNeuronIputs[3] = 1.0; break;
      case 'n': preprNeuronIputs[4] = 1.0; break;
      case 'N': preprNeuronIputs[5] = 1.0; break;
      case 'b': preprNeuronIputs[6] = 1.0; break;
      case 'B': preprNeuronIputs[7] = 1.0; break;
      case 'r': preprNeuronIputs[8] = 1.0; break;
      case 'R': preprNeuronIputs[9] = 1.0; break;
      case 'q': preprNeuronIputs[10]= 1.0; break;
      case 'Q': preprNeuronIputs[11]= 1.0; break;
      case ' ': break;

      case '\0':
      default:
        free(fcnnInputs);
        free(preprNeuronIputs);
        return NAN;
    }


    fcnnInputs[i] = calcNeuronOutput(net->preprocessingNeurons[i],
                                     preprNeuronIputs);
    free(preprNeuronIputs);
  }


  float* temptemp = fcnnPredict(net->fcnn, fcnnInputs);
  float temp = temptemp[0];
  free(fcnnInputs);
  free(temptemp);
  return temp;
}


TchNet* chNetSex(const TchNet* dad, const TchNet* mum, int mutationRareness)
{
  TchNet* baby = malloc(sizeof(TchNet));
  baby->preprocessingNeurons = malloc(PREPR_NEURONS_COUNT * sizeof(Tneuron*));
  
  // preprocessing neurons
  for(int i = 0; i < PREPR_NEURONS_COUNT; ++i){
    if(mutationRareness > 0 && rand() > (RAND_MAX / mutationRareness)){
      baby->preprocessingNeurons[i] = initRandNeuron(PREPR_NEURON_INP_COUNT,
                                              MIN_RAND_WEIGHT,
                                              MAX_RAND_WEIGHT);
    } else {
      if(rand() > RAND_MAX/2){
        baby->preprocessingNeurons[i] = cpyNeuron(dad->preprocessingNeurons[i]);
      } else {
        baby->preprocessingNeurons[i] = cpyNeuron(mum->preprocessingNeurons[i]);
      }
    }
  }

  baby->fcnn = fcnnSex(dad->fcnn, mum->fcnn, mutationRareness);

  return baby;
}
