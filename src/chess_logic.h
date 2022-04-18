/**
 * Project:  neural chess
 * Author:   Jakub Urbanek
 * Year:     2022
 * 
 * note:     copied from my minimax-chess project
 */

#ifndef __MODULE_CHESS_LOGIC_H
#define __MODULE_CHESS_LOGIC_H

#include "chess_structs.h"

#include <stdio.h>
#include <stdbool.h>


/**
 * constants
 */

//almost infinite number
#define INF 1000000000

// this is for https://en.wikipedia.org/wiki/Fifty-move_rule
#define MAX_BORING_MOVES 100  
 


/**
 * functions for colored terminal output
 */

// yellow background
#define LIGHT(string) "\x1b[43m" string "\x1b[0m"
// purple background
#define DARK(string) "\x1b[45m" string "\x1b[0m"
// cyan background
#define LIGHT_HINT(string) "\x1b[46m" string "\x1b[0m"
// blue background
#define DARK_HINT(string) "\x1b[44m" string "\x1b[0m"
// black foreground
#define BLACK_FOREGROUND(string) "\x1b[30m" string

/**
 * generates all possible moves for given position
 * 
 * @param ml pointer to moveList - gets filled with all possible moves
 * @param b pointer to board - moves are generated based on this
 * 
 * @note b doesn't get modified
 */
void generateAllPossibleMoves(Tboard *b, TmoveList* ml);

/**
 * generates all moves starting at *input
 * 
 * @param b pointer to board - moves are generated based on this
 * @param input string of at least two chars (ex. A8C3, C2)
 * @param ml pointer to moveList - gets filled with hints
 * 
 * @note b doesn't get modified
 */
void generateHints(Tboard *b, const char *input, TmoveList* ml);


/**
 * returns unicode string representation of piece
 * 
 * @param piece (ex. p, p, Q, k, N)
 * @return unicode string (ex. ♚, ♝, "\x1b[30m♛")
 * 
 * @note black is made using ANSI codes
 */
char* getPieceGraphics(char piece);

/**
 * returns last piece that moved on board
 * 
 * @note only if something has been moved, else crashes..?
 */
char getLastMovedPiece(const Tboard* b);

/**
 * returns true if last move is check, else returns false
 * 
 * @param b pointer to board
 * @param myKingPos position of king (checks if this king got checked)
 * 
 * @return true if last move is check, else false
 */
bool gotChecked(const Tboard *b, const int myKingPos[2]);

/**
 * returns 2 for game is pending, 0 for draw,  
 * 1 for win of white and -1 for win of black
 * 
 * @param b pointer to board (isn't modified)
 * @return 2 for game is pending, 0 for draw,  
 * 1 for win of white and -1 for win of black
 */
int getResult(Tboard *b);

/**
 * same as getResult(), but faster
 * 
 * @param b pointer to board (isn't modified)
 * @param ml ponter to moveList containing all possible moves in this pos
 */
int getResultFaster(Tboard *b, TmoveList *ml);

/**
 * returns true if input is in all possible moves in this position
 * 
 * @param b pointer to board (is treated as const)
 * @param input string representing move (ex. E2E4, E7E8r)
 * 
 * @return true if valdid, else false
 */
bool isInputValid(const char* input, Tboard* b);

/**
 * makes move  
 * 
 * @param b pointer to board - is moved
 * @param move string representing move (ex. E2E4, E7E8r)
 * 
 * @note avoid sending invalid move
 */
void moveBoard(const char* move, Tboard* b);

/**
 * returns true if move is diagonnaly by pawn and lands on empty square
 * 
 * @param b pointer to board
 * @param move string representing move (ex. E2E4, E7E8r)
 * 
 */
bool isEnPassant(const char* move, const Tboard* b);

/**
 * returns true if enpassant(pos+moveVector) is legal move
 * 
 * @param b pointer to board
 * @param pos array of two integers representing  
 *        position of enpassanting pawn (ex. white king initial pos == {7, 4})
 * @param moveVector array of two integers (pos + moveVector == finalPosition)
 * @param moveRestriction restricts direction of movement to
 *        one of the following values:  
 *        0(no restriction), 1(|), 2(/), 3(-), 4(\)
 * @param oppColor color of opponent ['A' | 'a']
 * @param mykingPos array of two integers representing pos of king of
 * enpassanting pawn
 */
bool isEnPassantLegal(const Tboard* b, const int pos[2],
                      const int moveVector[2], int moveRestriction,
                      char oppColor, const int mykingPos[2]);

/**
 * returns true if both integers are >=0 and <8, else false
 */
bool isOnBoard(const int pos[2]);

/**
 * fills ml with all posible moves of piece at pos
 */
void generatePieceMoves(const Tboard* b, const int pos[2], TmoveList* ml);

/**
 * returns false if knight's move lands on square
 * occupied by piece of same color or lands outside the board, else true
 * 
 * @param b pointer to board
 * @param color knight color ['A' | 'a']
 * @param pos array of two integers representing  
 *        position of knight
 * @param moveVector array of two integers (pos + moveVector == finalPosition)
 */
bool isKnightMovePossible(const Tboard* b, const char color,
                          const int pos[2], const int moveVector[2]);

/**
 * returns false if king lands on attacked square or square
 * occupied by piece of same color
 * 
 * @param b pointer to board
 * @param color knight color ['A' | 'a']
 * @param origin position of king
 * @param dest destination of king
 */
bool isKingMovePossible(const Tboard* b, const char color,
                        const int origin[2], const int dest[2]);

/**
 * returns true if move fits restriction
 * and lands on square occupied by opponent's piece and
 * lands on board
 * 
 * @param b pointer to board
 * @param color moving piece color ['A' | 'a']
 * @param pos array of two integers representing position of moving piece
 * @param moveVector array of two integers (pos + moveVector == finalPosition)
 * @param restriction restricts direction of movement to
 *        one of the following values:  
 *        0(no restriction), 1(|), 2(/), 3(-), 4(\)
 */
bool isMoveTake(const Tboard* b, const char color, const int pos[2],
                const int moveVector[2], const int restriction);

/**
 * returns true if move fits restriction
 * and lands on empty square and lands on board
 * 
 * @param b pointer to board
 * @param color moving piece color ['A' | 'a']
 * @param pos array of two integers representing position of moving piece
 * @param moveVector array of two integers (pos + moveVector == finalPosition)
 * @param restriction restricts direction of movement to
 *        one of the following values:  
 *        0(no restriction), 1(|), 2(/), 3(-), 4(\)
 */
bool isMoveClean(const Tboard* b, const int pos[2],
                 const int moveVector[2], const int restriction);

/**
 * locates piece on board and returns it's location in "location" argument
 * 
 * @param b pointer to board
 * @param piece character representing piece (ex. q, K, p)
 * @param location this is where location of piece is returned
 * 
 * @note if piece is not on board, returns {-1, -1}
 */
void getPieceLocation(const Tboard* b, const char piece, int location[2]);

/**
 * returns true if deleting piece causes check to king of deleted pieces color
 * 
 * @param b pointer to board 
 * @param oppColor color of opponent ['A' | 'a']
 * @param pos array of two integers representing
 *        position of potentialy blocking
 * @param kingPos array of two integers representing
 *        position of king
 * 
 * @return true if piece blocks check, else false
 */
bool isBlockingCheck(const Tboard* b, const char oppColor,
                     const int pos[2], const int kingPos[2]);

/**
 * returns true if there is nobody attacking king after move
 * 
 * @param b pointer to board
 * @param move string representing move (ex. E2E4, E7E8r)
 * @param kingPos array of two integers representing
 *        position of king under attack
 * 
 */
bool doesMoveBlockCheck(const Tboard* b, const char *move,
                        const int kingPos[2]);

/**
 * disables casteling and returns past csteling ability to buffer
 * 
 * @param b pointer to board
 * @param color color of player. His casteling gets disabled.
 * @param buffer gets filled by ability before disabling (left, right)
 */
void disableCastling(Tboard *b, char color, bool buffer[2]);

/**
 * regains casteling to be same as buffer
 * 
 * @param b pointer to board
 * @param color color of player ['a' | 'A']. He regains castling ability.
 * @param buffer castling ability is set to values in buffer (left, right)
 */
void regainCastling(Tboard *b, char color, bool buffer[2]);

/**
 * returns opposite color  
 * 'a' -> 'A'  
 * 'A' -> 'a'  
 * other inputs have undefined outputs
 */
char oppositeColor(const char color);

/**
 * returns true if color<=piece<=color+32, else false
 */
bool isColor(const char color, const char piece);

/**
 * for(int i = 0; i < len; i++){  
 * . . free(arr[i]);  
 * }  
 * free(arr);  
 * 
 */
void freeArrayOfStrings(char **arr, int len);

/**
 * returns true if array is in arrayOfArrays
 * 
 * @param array array to be found
 * @param arrayOfArrays array is searched in there
 * @param arrLen length of array parameter and arrays in arrayOfArrays
 * @param arrOfArrsLen length of arrayOfArrays parameter
 */
bool isArrayInArrayOfArrays(int *array, int **arrayOfArrays,
                            int arrLen, int arrOfArrsLen);

/**
 * returns true if string is twice in arrayOfStrings
 * 
 * @param string string to be found
 * @param arrayOfStrings string is searched in there
 * @param ArrOfStringsLen length of arrayOfStrings parameter
 */
bool isStringTwiceInArrayOfStrings(char *string, char **arrayOfStrings,
                                   int ArrOfStringsLen);

/**
 * makes vector one step long  
 * (diagonal is longer {1, -1})
 * 
 */
void normalize(int moveVector[2]);

/**
 * returns true if moveVector fits restriction
 * 
 * @param moveVector array of two integers (pos + moveVector == finalPosition)
 * @param restriction restricts direction of movement to
 *        one of the following values:  
 *        0(no restriction), 1(|), 2(/), 3(-), 4(\)
 */
bool fitsRestriction(const int moveVector[2], int restriction);

/**
 * returns true if piece at pos is attacked
 * 
 * @param b pointer to board
 * @param oppColor color of attacking pieces ['a' | 'A']
 * @param pos array of two integers representing
 *        position of potentially attacked piece
 *
 * @return true if piece at position is attacked, else false
 */
bool isAttacked(const Tboard* b, const char oppColor, const int pos[2]);

/**
 * returns true if piece moving from origin to dest will be attacked
 * 
 * @param b pointer to board
 * @param oppColor color of attacking pieces ['a' | 'A']
 * @param origin array of two integers representing starting
 *        position of potentially attacked piece
 * @param dest array of two integers representing destination
 *        of potentially attacked piece
 *
 * @return true if piece will be under attack at dest, else false
 */
bool willBeAttacked(const Tboard *b, const char color, const int origin[2],
                    const int dest[2]);

#endif
