/**
 * Project:  neural chess
 * Author:   Jakub Urbanek
 * Year:     2022
 * 
 * note:     copied from my minimax-chess project
 */

//NOTE:
// I am ignoring the chess terminology definition of "move"
// My "move" is just one turn (so this: [e4, e5, d3, d6] is four moves)

#ifndef __MODULE_CHESS_STRUCTS_H
#define __MODULE_CHESS_STRUCTS_H

#include <stdbool.h>

//length of posString (64 squares + 1 '\0')
#define POS_STRING_LEN 65

//max length of string representing move + 1 '\0'
#define MAX_INP_LEN 6


typedef struct{

  // 2d arrray of pieces
  // white - k = king, q = queen, r = rook, b = bishop, n = knight, p = pawn
  // black - K = king, Q = queen, R = rook, B = bishop, N = knight, P = pawn
  char pieces[8][8];

  // two values for long and short castling (0 - long, 1- short)
  bool canWhiteCastle[2];

  // two values for long and short castling (0 - long, 1 - short)
  bool canBlackCastle[2];
  
  // useful for "en passant"
  char* lastMove;

  // total number of moves made
  int move;

  // number of moves without take or pawn move
  //(for https://en.wikipedia.org/wiki/Fifty-move_rule)
  int boringMoveCount;

  // array of boring positions since last unboring move
  // (used for draws by repetition)
  char **boringPoss;
  
  //number of pieces left (for evaluating king's position)
  int pieceCount;

} Tboard;


/**
 * initializes board datastructure
 * 
 * @param void
 * @return pointer to allocated board
 * @note board is set to starting position
 */
Tboard* initBoard(void);

/**
 * copies board datastructure
 * 
 * @param b pointer to board to be copied
 * @return pointer to copy of b
 */
Tboard* copyBoard(const Tboard *b);


/**
 * initializes board that matches the fenString
 * @return pointer to board if OK else NULL
 * @note https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation
 */
Tboard* fenToBoard(char *fenString);

/**
 * returns posString of current possition
 * 
 * @param b pointer to board
 * @returns string with len 65 representing current position
 * @note posString of srarting position:  
 * "RNBQKBNRPPPPPPPP                                pppppppprnbqkbnr"
 */
char* boardToPosString(const Tboard *b);

/**
 * frees board
 * 
 * @param b pointer to board to be freed
 * @return void
 */
void freeBoard(Tboard* b);






/**
 * dynamically allocated expandable list of moves  
 * move notation:  
 * ex. E2E4, E7E5, F1E2, F7F6, G1F3, D7D6
 * ex. castlinng: E1G1, E1C1
 * ex. promotions: E7E8, E7E8r, E7E8n, E7E8b
 */
typedef struct{

  //array of moves
  char **moves;

  //allocated number of moves
  int size;

  //filled number of moves
  int filled;

} TmoveList;


/**
 * initializes TmoveList
 * 
 * @param size initial size of list (meaning count of moves to fill it)
 * @return pointer to initialized moveList
 */
TmoveList* initMoveList(int size);

/**
 * appends move to the end of moveList
 * 
 * @param ml pointer to moveList
 * @param move string (max 5+1 chars) to be appended
 * @return void
 */
void appendMoveList(TmoveList* ml, char* move);

/**
 * frees moveList
 * 
 * @param ml pointer to moveList to be freed
 * @return void
 */
void freeMoveList(TmoveList* ml);

/**
 * returns caseswitched c 
 */
char switchCase(char c);

#endif