/**
 * Project:  neural chess
 * Author:   Jakub Urbanek
 * Year:     2022
 */

#ifndef __MODULE_NEURON_H
#define __MODULE_NEURON_H

#include <stdio.h>

/**
 * neuron to be used in neural network
 * 
 * 
 */
typedef struct {

  //number of inputs of neuron
  int inputCount;

  //array of weights (obviously)
  float* weights;

  //bias (obviously)
  float bias;

} Tneuron;

/**
 * returns pointer to Tneuron initialized with
 * weights and bias in parameters
 */
Tneuron* initNeuron(int inputCount, const float* weights, float bias);

/**
 * returns pointer to Tneuron initialized with
 * random weights and bias between min and max values
 */
Tneuron* initRandNeuron(int inputCount, float min, float max);

/**
 * frees neuron
 */
void freeNeuron(Tneuron* n);

/**
 * prints neuron in format:  
 * inputCount  
 * weights  
 * bias
 */
void printNeuron(const Tneuron* n);

/**
 * prints neuron to file in format:  
 * inputCount  
 * weights  
 * bias
 */
void fprintNeuron(FILE* out, const Tneuron* n);

/**
 * gets neuron in format:  
 * inputCount  
 * weight1 weight2 weight3 ... weightinputCount  
 * bias
 */
Tneuron* getNeuron(void);

/**
 * gets neuron from file in format:  
 * inputCount  
 * weight1 weight2 weight3 ... weightinputCount  
 * bias
 */
Tneuron* fgetNeuron(FILE* in);

/**
 * returns copy of neuron in parameter
 */
Tneuron* cpyNeuron(const Tneuron* origin);

/**
 * y = 1/(1+e^(-x))
 * https://en.wikipedia.org/wiki/Logistic_function
 */
float sigmoid(float x);

/**
 * takes array of inputs and returns output of neuron
 */
float calcNeuronOutput(const Tneuron* n, const float* inputs);

#endif
