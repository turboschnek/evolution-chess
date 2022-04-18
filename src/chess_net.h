/**
 * Project:  neural chess
 * Author:   Jakub Urbanek
 * Year:     2022
 */

#ifndef __MODULE_CHESS_NET_H
#define __MODULE_CHESS_NET_H

#include "neuron.h"
#include "fcnn.h"

#include <stdbool.h>

typedef struct {

  // first layer of neurons acting as more complex inputs of fcnn
  Tneuron** preprocessingNeurons;

  // fully connected neural net
  Tfcnn* fcnn;

} TchNet;


/**
 * returns initialized chNet with random weights and biases
 * 
 * returns NULL if error
 * 
 * @note (fcnnNeuronsInLayersCount[0] != 64) -> return NULL
 */
TchNet* initRandChNet(int fcnnLayerCount, const int* fcnnNeuronsInLayersCount);


void freeChNet(TchNet* net);


/**
 * prints chess network to stdout
 */
void printChNet(const TchNet* n);

/**
 * prints chess network to file
 */
void fprintChNet(FILE* out, const TchNet* n);

/**
 * gets chess network from stdin
 * 
 * @return initialized chNet or NULL for error
 */
TchNet* getChNet(void);

/**
 * gets chess network from file
 * 
 * @return initialized chNet or NULL for error
 */
TchNet* fgetChNet(FILE* in);


/**
 * 
 * 
 * @param net pointer to chessNet
 * 
 * @param posString format:
 * length 64
 * possible characters (white pieces are capital):
 * ['p', 'P', 'k', 'K', 'n', 'N', 'b', 'B', 'r', 'R', 'q', 'Q', ' ']
 * 
 */
float chNetPredict(const TchNet* net, const char* posString);


/**
 * returns baby of mum and dad in parameters
 * 
 * @param mutationRareness 1 in $(mutationRareness) neurons gets randomized
 */
TchNet* chNetSex(const TchNet* dad, const TchNet* mum, int mutationRareness);

#endif