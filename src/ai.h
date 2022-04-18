/**
 * Project:  neural chess
 * Author:   Jakub Urbanek
 * Year:     2022
 */

#ifndef __MODULE_AI_H
#define __MODULE_AI_H


#include "chess_net.h"

/**
 * lets two AIs play game of chess
 * 
 * uses networks in parameters to staticaly evaluate positions reached
 * by minimax algo
 * 
 * @return 0 for draw, 1 for win of white, -1 for win of black
 */
int game(TchNet* white, TchNet* black, float timeBudget);

/*
TODO:
  standard minimax with primitive evaluation from my past project
  minimax with use of neural net
  tournament (objective is not to find the best one...
              ...It is to find the better half)
  evolution where the tournament is the fitness function
  tinker with layerCounts and other stuff
*/

#endif