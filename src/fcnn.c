/**
 * Project:  neural chess
 * Author:   Jakub Urbanek
 * Year:     2022
 */

#include "fcnn.h"
#include "neuron.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>




Tfcnn* initRandfcnn(int layerCount, const int* neuronsInLayersCount)
{
  //there are no neurons in first layer

  Tfcnn* n = malloc(sizeof(Tfcnn));
  n->layerCount = layerCount;
  n->neuronsInLayersCount = malloc(n->layerCount * sizeof(int));
  n->neurons = malloc((n->layerCount-1) * sizeof(Tneuron**));

  n->neuronsInLayersCount[0] = neuronsInLayersCount[0];
  for(int i = 1; i < n->layerCount; ++i){
    n->neuronsInLayersCount[i] = neuronsInLayersCount[i];

    n->neurons[i-1] = malloc(n->neuronsInLayersCount[i] * sizeof(Tneuron*));
    for(int j = 0; j < n->neuronsInLayersCount[i]; ++j){
      n->neurons[i-1][j] = initRandNeuron(n->neuronsInLayersCount[i-1],
                                          MIN_RAND_WEIGHT,
                                          MAX_RAND_WEIGHT);
    }
  }

  return n;
}


void freefcnn(Tfcnn* n)
{
  for(int i = 1; i < n->layerCount; ++i){
    for(int j = 0; j < n->neuronsInLayersCount[i]; ++j){
      freeNeuron(n->neurons[i-1][j]);
    }
    free(n->neurons[i-1]);
  }
  free(n->neuronsInLayersCount);
  free(n->neurons);
  free(n);
}

void printfcnn(const Tfcnn* n)
{
  fprintfcnn(stdout, n);
}


void fprintfcnn(FILE* out, const Tfcnn* n)
{
  fprintf(out, "%d\n", n->layerCount);

  for(int i = 0; i < n->layerCount; ++i){
    fprintf(out, "%d ", n->neuronsInLayersCount[i]);
  }
  fprintf(out, "\n");

  for(int i = 1; i < n->layerCount; ++i){
    for(int j = 0; j < n->neuronsInLayersCount[i]; ++j){
      fprintNeuron(out, n->neurons[i-1][j]);
    }
  }
}

Tfcnn* getfcnn(void)
{
  return fgetfcnn(stdin);
}

Tfcnn* fgetfcnn(FILE* in)
{
  Tfcnn* n = malloc(sizeof(Tfcnn));
  if(fscanf(in, "%d", &n->layerCount) != 1){
    return NULL;
  }

  n->neuronsInLayersCount = malloc(n->layerCount * sizeof(int));
  n->neurons = malloc((n->layerCount-1) * sizeof(Tneuron**));

  if(fscanf(in, "%d", &n->neuronsInLayersCount[0]) != 1){
      return NULL;
    }
  for(int i = 1; i < n->layerCount; ++i){
     if(fscanf(in, "%d", &n->neuronsInLayersCount[i]) != 1){
      return NULL;
    }
    n->neurons[i-1] = malloc(n->neuronsInLayersCount[i] * sizeof(Tneuron*));
  }


  for(int i = 1; i < n->layerCount; ++i){
    for(int j = 0; j < n->neuronsInLayersCount[i]; ++j){
      n->neurons[i-1][j] = fgetNeuron(in);
      if(n->neurons[i-1][j] == NULL){
        return NULL;
      }
    }
  }

  return n;
}

float* propagateLayer(const Tfcnn* net, const float* inputs, int layerIndex)
{
  float* output = malloc(net->neuronsInLayersCount[layerIndex] *
                         sizeof(float));

  for(int i = 0; i < net->neuronsInLayersCount[layerIndex]; ++i){
    output[i] = calcNeuronOutput(net->neurons[layerIndex-1][i], inputs);
  }

  return output;
}

float* fcnnPredict(const Tfcnn* net, const float* inputs)
{
  float* b;

  float* a = malloc(net->neuronsInLayersCount[0] * sizeof(float));
  memcpy(a, inputs, net->neuronsInLayersCount[0] * sizeof(float));

  for(int i = 1; i < net->layerCount; ++i){
    b = propagateLayer(net, a, i);

    a = realloc(a, net->neuronsInLayersCount[i] * sizeof(float));

    memcpy(a, b, net->neuronsInLayersCount[i] * sizeof(float));

    free(b);
  }

  return a;
}

Tfcnn* fcnnSex(const Tfcnn* dad, const Tfcnn* mum, int mutationRareness)
{
  Tfcnn* baby = malloc(sizeof(Tfcnn));
  baby->layerCount = dad->layerCount;
  baby->neuronsInLayersCount = malloc(baby->layerCount * sizeof(int));
  baby->neurons = malloc((baby->layerCount-1) * sizeof(Tneuron**));

  baby->neuronsInLayersCount[0] = dad->neuronsInLayersCount[0];

  for(int i = 1; i < baby->layerCount; ++i){
    baby->neuronsInLayersCount[i] = dad->neuronsInLayersCount[i];

    baby->neurons[i-1] = malloc(baby->neuronsInLayersCount[i] *
                                sizeof(Tneuron*));
    for(int j = 0; j < baby->neuronsInLayersCount[i]; ++j){
      
      if(mutationRareness > 0 && rand() > (RAND_MAX / mutationRareness)){
        baby->neurons[i-1][j] = initRandNeuron(baby->neuronsInLayersCount[i-1],
                                               MIN_RAND_WEIGHT,
                                               MAX_RAND_WEIGHT);
      } else {
        if(rand() > RAND_MAX/2){
          baby->neurons[i-1][j] = cpyNeuron(dad->neurons[i-1][j]);
        } else {
          baby->neurons[i-1][j] = cpyNeuron(mum->neurons[i-1][j]);
        }
      }
    }
  }
  

  return baby;
}
