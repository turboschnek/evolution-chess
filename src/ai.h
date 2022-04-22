/**
 * Project:  neural chess
 * Author:   Jakub Urbanek
 * Year:     2022
 */

#ifndef __MODULE_AI_H
#define __MODULE_AI_H


#include "chess_net.h"
#include "chess_structs.h"


/**
 * Initializes population of chNets and evolves them by forcing them to fight
 * each other in the most deadly game of chess in their lives.
 * 
 * saves the population
 */
void chNetEvolution();


/**
 * sorts population, second half is sentenced to death
 */
void quickTournament(TchNet** population, int populationCount, int rounds,
                     float timeForMove);


/**
 * lets two AIs play game of chess
 * 
 * uses networks in parameters to staticaly evaluate positions reached
 * by minimax algo. If chNet is NULL, primitiveEval is used instead. 
 * 
 * @return 0 for draw, 1 for win of white, -1 for win of black
 */
int game(const TchNet* white, const TchNet* black, float timeBudget);


/**
 * Uses minimax to choose a move.
 * 
 * @param b current board
 * @param net chess network used for evaluation (if NULL, primitiveEval is used)
 * @param output gets filled by AI (length == MAX_INP_LENGTH)
 * @param seconds max time for move
 * 
 * @return depth of finished search
 */
int minimax(Tboard *b, const TchNet* net, float seconds, char *output);


/**
 * recursive part of minimax alg
 * 
 * @param b board position to evaluate
 * @param net chess network used for evaluation (if NULL, primitiveEval is used)
 * @param depth depth of search (recomended odd value)
 * @param isMax is player maximazing?
 * @param alfa for pruning (initially -INF)
 * @param beta for pruning (initially INF)
 * 
 * @return numerical evaluation of position
 * @note use minimax() instead
 */
float innerMinimax(Tboard *b, const TchNet* net, int depth, bool isMax, float alfa, float beta);


/**
 * sorts ml based on keys` values
 */
void sortPopulation(TchNet** population, float *keys, int populationCount, bool increasing);

/**
 * randomizes population`s order
 * 
 * net+key pairs remain together
 */
void shufflePopulationWithKeys(TchNet** population, float* keys, int populationCount);

/**
 * sorts ml based on keys` values
 */
void sortMoveList(TmoveList* ml, float *keys, bool increasing);


/**
 * returns evaluation of position
 * 
 * if net is null, uses primitiveEval
 */
float evaluateBoard(const Tboard* b, const TchNet* net);

/**
 * sum of piece values from fun getPieceValue
 */
float primitiveEval(const char* posString);

/**
 * 
 * Gets value of piece.  
 * Position is considered for knight, pawn and king.
 * 
 * @param piece ex.: 'k', 'K', 'n'
 * @param col piece's column (starting on left)
 * @param row piece's row (starting on top)
 * @param pieceCount number of remaining pieces on board
 * 
 * @return value between -900(Q) and 900(q)
 */
int getPieceValue(char piece, int row, int col, int pieceCount);

#endif