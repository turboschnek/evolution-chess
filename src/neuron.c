/**
 * Project:  neural chess
 * Author:   Jakub Urbanek
 * Year:     2022
 */

#include "neuron.h"


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>


Tneuron* initNeuron(int inputCount, const float* weights, float bias)
{
  Tneuron *n = malloc(sizeof(Tneuron));
  n->inputCount = inputCount;
  n->weights = malloc(n->inputCount * sizeof(float));
  for(int i = 0; i < n->inputCount; ++i){
    n->weights[i] = weights[i];
  }
  n->bias = bias;
  return n;
}


Tneuron* initRandNeuron(int inputCount, float min, float max)
{
  Tneuron *n = malloc(sizeof(Tneuron));
  n->inputCount = inputCount;
  n->weights = malloc(n->inputCount * sizeof(float));
  for(int i = 0; i < n->inputCount; ++i){
    n->weights[i] = (((float)rand()/(float)(RAND_MAX)) * (max-min)) + min;
  }
  n->bias = (((float)rand()/(float)(RAND_MAX)) * (max-min)) + min;
  return n;
}


void freeNeuron(Tneuron* n)
{
  free(n->weights);
  free(n);
}


void printNeuron(const Tneuron* n)
{
  fprintNeuron(stdout, n);
}


void fprintNeuron(FILE* out, const Tneuron* n)
{
  fprintf(out, "%d\n", n->inputCount);
  for(int i = 0; i < n->inputCount; ++i){
    fprintf(out, "%f ", n->weights[i]);
  }
  fprintf(out, "\n%f\n", n->bias);
}


Tneuron* getNeuron(void)
{
  return fgetNeuron(stdin);
}


Tneuron* fgetNeuron(FILE* in)
{
  Tneuron *n = malloc(sizeof(Tneuron));
  if(fscanf(in, "%d", &n->inputCount) != 1){
    free(n);
    return NULL;
  }
  n->weights = malloc(n->inputCount * sizeof(float));
  for(int i = 0; i < n->inputCount; ++i){
    if(fscanf(in, "%f", &n->weights[i]) != 1){
      freeNeuron(n);
      return NULL;
    }
  }
  if(fscanf(in, "%f", &n->bias) != 1){
    freeNeuron(n);
    return NULL;
  }

  return n;
}

Tneuron* cpyNeuron(const Tneuron* origin)
{
  Tneuron* n = malloc(sizeof(Tneuron));

  n->inputCount = origin->inputCount;
  n->weights = malloc(n->inputCount * sizeof(float));
  for(int i = 0; i < n->inputCount; ++i){
    n->weights[i] = origin->weights[i];
  }
  n->bias = origin->bias;

  return n;
}


float sigmoid(float x)
{
  return 1 / (1 + exp(-x));
}


float calcNeuronOutput(const Tneuron* n, const float* inputs)
{
  float sum = 0;
  for(int i = 0; i < n->inputCount; i++){
    sum += inputs[i] * n->weights[i];
  }
  return sigmoid(sum + n->bias);
}
