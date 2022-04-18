/**
 * Project:  neural chess
 * Author:   Jakub Urbanek
 * Year:     2022
 * 
 * note:     copied from my minimax-chess project
 */

#include "chess_logic.h"
#include "chess_structs.h"

#include <math.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>



const int knightMoveVectors[8][2] = {
  {+2, -1}, {+2, +1}, {-2, -1}, {-2, +1},
  {+1, -2}, {+1, +2}, {-1, -2}, {-1, +2}
};

//dont touch this! change of order breaks casteling (in fun generatePieceMoves)
const int rookMoveVectors[4][2] = {
  {-1, 0}, {0, -1}, {1, 0}, {0, 1}
};

const int bishopMoveVectors[4][2] = {
  {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
};


void moveBoard(const char *input, Tboard *b)
{
  int from[2] = {input[0]-'A', 7- (input[1]-'1')};
  int to[2] = {input[2]-'A', 7- (input[3]-'1')};

  //if something moves from or to a corner,
  //disable castling ability for that corner
  if((input[0] == 'A' && input[1] == '8') ||
     (input[2] == 'A' && input[3] == '8'))
    b->canBlackCastle[0] = false;
  
  if((input[0] == 'H' && input[1] == '8') ||
          (input[2] == 'H' && input[3] == '8'))
    b->canBlackCastle[1] = false;
  
  if((input[0] == 'A' && input[1] == '1') ||
          (input[2] == 'A' && input[3] == '1'))
    b->canWhiteCastle[0] = false;
  
  if((input[0] == 'H' && input[1] == '1') ||
          (input[2] == 'H' && input[3] == '1'))
    b->canWhiteCastle[1] = false;


  //extra action if en passanting
  if(getLastMovedPiece(b) == 'p' || getLastMovedPiece(b) == 'P'){
    if(isEnPassant(input, b)){
      b->pieceCount--;
      b->pieces['8' - b->lastMove[3]][b->lastMove[2] - 'A'] = ' ';
    }
  }

  //extra action if castling
  if(b->pieces[from[1]][from[0]] == 'k' &&
     (b->canWhiteCastle[0] || b->canWhiteCastle[1]))
  {
    b->canWhiteCastle[0] = b->canWhiteCastle[1] = false;
    if(input[2] == 'G'){
      b->pieces[7][5] = 'r';
      b->pieces[7][7] = ' ';
    } else if(input[2] == 'C'){
      b->pieces[7][3] = 'r';
      b->pieces[7][0] = ' ';
    }
  } else if(b->pieces[from[1]][from[0]] == 'K' &&
            (b->canBlackCastle[0] || b->canBlackCastle[1]))
  {
    b->canBlackCastle[0] = b->canBlackCastle[1] = false;
    if(input[2] == 'G'){
      b->pieces[0][5] = 'R';
      b->pieces[0][7] = ' ';
    } else if(input[2] == 'C'){
      b->pieces[0][3] = 'R';
      b->pieces[0][0] = ' ';
    }
  }

  //extra action if promoting
  if((input[3] == '1' || input[3] == '8') &&
     (b->pieces[from[1]][from[0]] == 'p' ||
      b->pieces[from[1]][from[0]] == 'P'))
  {
    if(input[4] == '\0') {
      b->pieces[from[1]][from[0]] = b->pieces[from[1]][from[0]] + ('q'-'p');
    } else {
      b->pieces[from[1]][from[0]] = input[4] +
                                    (((b->move+1) % 2) * ('a' - 'A'));
    }
  }

  //boringMovesCount
  if(b->pieces[to[1]][to[0]] != ' ' ||
     b->pieces[from[1]][from[0]] == 'p' ||
     b->pieces[from[1]][from[0]] == 'P')
  {
    freeArrayOfStrings(b->boringPoss, b->boringMoveCount);
    b->boringPoss = malloc(0);

    b->boringMoveCount = 0;

  } else {
    b->boringMoveCount++;

    b->boringPoss = realloc(b->boringPoss, b->boringMoveCount * sizeof(char*));
    b->boringPoss[b->boringMoveCount-1] = malloc(POS_STRING_LEN *
                                                 sizeof(char));

    char *temp = boardToPosString(b);
    strcpy(b->boringPoss[b->boringMoveCount-1], temp);
    free(temp);
  }

  if(b->pieces[to[1]][to[0]] != ' ') b->pieceCount--;

  //moving
  b->pieces[to[1]][to[0]] = b->pieces[from[1]][from[0]];
  b->pieces[from[1]][from[0]] = ' ';

  b->move++;
  strcpy(b->lastMove, input);
}


int getResultFaster(Tboard *b, TmoveList *ml)
{
  char *temp = boardToPosString(b);
  if(b->boringMoveCount >= MAX_BORING_MOVES ||
     isStringTwiceInArrayOfStrings(temp, b->boringPoss, b->boringMoveCount))
  {
    free(temp);
    return 0;
  }
  free(temp);

  if(ml->filled == 0){
    int kingPos[2] = {0, 0};
    if(b->move%2 == 0){
      getPieceLocation(b, 'k', kingPos);
      if(gotChecked(b, kingPos)){
        return -1;
      }
    }else{
      getPieceLocation(b, 'K', kingPos);
      if(gotChecked(b, kingPos)){
        return 1;
      }
    }
    return 0;  //stalemate
  }
  if(b->pieceCount == 2) return 0;  //only two kings on board are draw
  return 2;  // not stopped
}


int getResult(Tboard *b)
{
  char *temp = boardToPosString(b);
  if(b->boringMoveCount >= MAX_BORING_MOVES ||
     isStringTwiceInArrayOfStrings(temp, b->boringPoss, b->boringMoveCount)){

    free(temp);
    return 0;
  }
  free(temp);

  TmoveList *moveBuffer = initMoveList(2);
  generateAllPossibleMoves(b, moveBuffer);
  int moveCount = moveBuffer->filled;
  freeMoveList(moveBuffer);

  if(moveCount == 0){
    int kingPos[2] = {0, 0};
    if(b->move%2 == 0){
      getPieceLocation(b, 'k', kingPos);
      if(gotChecked(b, kingPos)){
        return -1;
      }
    }else{
      getPieceLocation(b, 'K', kingPos);
      if(gotChecked(b, kingPos)){
        return 1;
      }
    }
    return 0;  //stalemate
  }
  if(b->pieceCount == 2) return 0;  //only two kings on board are draw

  return 2; // not stopped
}


void generateAllPossibleMoves(Tboard * b, TmoveList *ml)
{
  char color;
  if(b->move % 2 == 0) color = 'a';
  else color = 'A';

  int *kingPos = malloc(2 * sizeof(int));
  getPieceLocation(b, color + ('k' - 'a'), kingPos);
  if(!(kingPos[0] >= 0 && kingPos[0] <= 7 &&
       kingPos[1] >= 0 && kingPos[1] <= 7)){
    free(kingPos);
    fprintf(stderr, "\nERROR:\n");
    fprintf(stderr, "kingPos is not valid in fun generateAllPossibleMoves\n");
    return;
  }
  if(gotChecked(b, kingPos)){
    TmoveList *tempMl = initMoveList(8);
    bool buffer[2];

    //temporarely disable castling ability
    disableCastling(b, color, buffer);

    for(int i = 0; i < 8; i++){
      for(int j = 0; j < 8; j++){
        if(isColor(color, b->pieces[i][j]) &&
           b->pieces[i][j] != (color + ('k' - 'a'))){
          generatePieceMoves(b, (const int[2]){j, i}, tempMl);
        }
      }
    }
    for(int i = 0; i < tempMl->filled; i++){
      if(doesMoveBlockCheck(b, tempMl->moves[i], kingPos)){
        appendMoveList(ml, tempMl->moves[i]);
      }
    }
    generatePieceMoves(b, kingPos, ml);

    regainCastling(b, color, buffer);  //regain castling ability

    freeMoveList(tempMl);


  } else {
    for(int i = 0; i < 8; i++){
      for(int j = 0; j < 8; j++){
        if(isColor(color, b->pieces[i][j])){
          generatePieceMoves(b, (const int[2]){j, i}, ml);
        }
      }
    }
  }
  free(kingPos);
}


void generateHints(Tboard *b, const char *input, TmoveList* ml)
{
  TmoveList *temp = initMoveList(8);
  generateAllPossibleMoves(b, temp);
  for(int i = 0; i < temp->filled; i++){
    if(temp->moves[i][0] == input[0] && temp->moves[i][1] == input[1]){
      appendMoveList(ml, temp->moves[i]);
    }
  }
  freeMoveList(temp);
}


char getLastMovedPiece(const Tboard* b)
{
  return b->pieces['8' - b->lastMove[3]][b->lastMove[2] - 'A'];
}


void generatePieceMoves(const Tboard* b, const int pos[2], TmoveList* ml)
{
  char row = '8' - pos[1];  // '1'..'8'
  char col = 'A' + pos[0];  // 'A'..'H'

  char color = 'a';
  if(isupper(b->pieces[pos[1]][pos[0]])) color = 'A';

  int kingPos[2];
  getPieceLocation(b, color + 10, kingPos);


  int moveRestriction = 0;
  // useful for not letting pinned piece move in
  // other direction than to or from the king
  // values-  0(no restriction), 1(|), 2(/), 3(-), 4(\)


  if(isBlockingCheck(b, oppositeColor(color), pos, kingPos)){

    if(pos[0] == kingPos[0]){
      moveRestriction = 1;
    }else if(pos[1] == kingPos[1]){
      moveRestriction = 3;
    }else if((pos[0] - kingPos[0]) == (pos[1] - kingPos[1])){
      moveRestriction = 4;
    }else{
      moveRestriction = 2;
    }
  }

  switch(b->pieces[pos[1]][pos[0]]){
  case 'p':
    //straight one
    if(isMoveClean(b, pos, (const int[2]){0, -1}, moveRestriction)){
      appendMoveList(ml, (char []){col, row, col, row+1, '\0'});
      if(row == '7'){
        appendMoveList(ml, (char []){col, row, col, row+1, 'N', '\0'});
        appendMoveList(ml, (char []){col, row, col, row+1, 'B', '\0'});
        appendMoveList(ml, (char []){col, row, col, row+1, 'R', '\0'});
      }


      //start jump of len two
      if(isMoveClean(b, pos, (const int[2]){0, -2}, moveRestriction) &&
         row == '2')
      {
        appendMoveList(ml, (char []){col, row, col, row+2, '\0'});
      }
    }
    //take
    if(isMoveTake(b, 'A', pos, (const int[2]){1, -1}, moveRestriction)) {
      appendMoveList(ml, (char []){col, row, col+1, row+1, '\0'});
      if(row == '7'){
        appendMoveList(ml, (char []){col, row, col+1, row+1, 'N', '\0'});
        appendMoveList(ml, (char []){col, row, col+1, row+1, 'B', '\0'});
        appendMoveList(ml, (char []){col, row, col+1, row+1, 'R', '\0'});
      }
    }
    if(isMoveTake(b, 'A', pos, (const int[2]){-1, -1}, moveRestriction)) {
      appendMoveList(ml, (char []){col, row, col-1, row+1, '\0'});
      if(row == '7'){
        appendMoveList(ml, (char []){col, row, col-1, row+1, 'N', '\0'});
        appendMoveList(ml, (char []){col, row, col-1, row+1, 'B', '\0'});
        appendMoveList(ml, (char []){col, row, col-1, row+1, 'R', '\0'});
      }
    }

    //en passant
    if(isEnPassantLegal(b, pos, (const int[2]){1, -1},
                        moveRestriction, 'A', kingPos))
      appendMoveList(ml, (char []){col, row, col+1, row+1, '\0'});

    if(isEnPassantLegal(b, pos, (const int[2]){-1, -1},
                        moveRestriction, 'A', kingPos))
      appendMoveList(ml, (char []){col, row, col-1, row+1, '\0'});
    break;

  case 'P':
    //straight one
    if(isMoveClean(b, pos, (const int[2]){0, +1}, moveRestriction)){
      appendMoveList(ml, (char []){col, row, col, row-1, '\0'});
      if(row == '2'){
        appendMoveList(ml, (char []){col, row, col, row-1, 'N', '\0'});
        appendMoveList(ml, (char []){col, row, col, row-1, 'B', '\0'});
        appendMoveList(ml, (char []){col, row, col, row-1, 'R', '\0'});
      }

      //start jump of len two
      if(isMoveClean(b, pos, (const int[2]){0, +2}, moveRestriction) &&
         row == '7')
      {
        appendMoveList(ml, (char []){col, row, col, row-2, '\0'});
      }
    }
    //take
    if(isMoveTake(b, 'a', pos, (const int[2]){1, 1}, moveRestriction)) {
      appendMoveList(ml, (char []){col, row, col+1, row-1, '\0'});
      if(row == '2'){
        appendMoveList(ml, (char []){col, row, col+1, row-1, 'N', '\0'});
        appendMoveList(ml, (char []){col, row, col+1, row-1, 'B', '\0'});
        appendMoveList(ml, (char []){col, row, col+1, row-1, 'R', '\0'});
      }
    }
    if(isMoveTake(b, 'a', pos, (const int[2]){-1, 1}, moveRestriction)) {
      appendMoveList(ml, (char []){col, row, col-1, row-1, '\0'});
      if(row == '2'){
        appendMoveList(ml, (char []){col, row, col-1, row-1, 'N', '\0'});
        appendMoveList(ml, (char []){col, row, col-1, row-1, 'B', '\0'});
        appendMoveList(ml, (char []){col, row, col-1, row-1, 'R', '\0'});
      }
    }

    //en passant
    if(isEnPassantLegal(b, pos, (const int[2]){1, 1},
                        moveRestriction, 'a', kingPos))
      appendMoveList(ml, (char []){col, row, col+1, row-1, '\0'});

    if(isEnPassantLegal(b, pos, (const int[2]){-1, 1},
                        moveRestriction, 'a', kingPos))
      appendMoveList(ml, (char []){col, row, col-1, row-1, '\0'});
    break;

  case 'n':
    for(int i = 0; i < 8; i++){
      if(isKnightMovePossible(b, 'a', pos, knightMoveVectors[i]) &&
         moveRestriction == 0)
      {
        appendMoveList(ml, (char[]){col,
                                    row,
                                    col + knightMoveVectors[i][0],
                                    row - knightMoveVectors[i][1],
                                    '\0'});
      }
    }
    break;

  case 'N':
    for(int i = 0; i < 8; i++){
      if(isKnightMovePossible(b, 'A', pos, knightMoveVectors[i]) &&
         moveRestriction == 0)
      {
        appendMoveList(ml, (char[]){col,
                                    row,
                                    col + knightMoveVectors[i][0],
                                    row - knightMoveVectors[i][1],
                                    '\0'});
      }
    }
    break;

  case 'b':
    for(int direction = 0; direction < 4; direction++){
      int moveVector[2] = {bishopMoveVectors[direction][0],
                           bishopMoveVectors[direction][1]};

      if(fitsRestriction(moveVector, moveRestriction)){
        int positionBuffer[2] = {pos[0] + moveVector[0],
                                 pos[1] + moveVector[1]};
        while(isOnBoard(positionBuffer)){
          if(islower(b->pieces[positionBuffer[1]][positionBuffer[0]])) break;

          appendMoveList(ml, (char[]){col,
                                      row,
                                      'A' + positionBuffer[0],
                                      '8' - positionBuffer[1],
                                      '\0'});

          if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

          positionBuffer[0] = positionBuffer[0] + moveVector[0];
          positionBuffer[1] = positionBuffer[1] + moveVector[1];
        }
      }
    }
    break;

  case 'B':
    for(int direction = 0; direction < 4; direction++){
      int moveVector[2] = {bishopMoveVectors[direction][0],
                           bishopMoveVectors[direction][1]};

      if(fitsRestriction(moveVector, moveRestriction)){
        int positionBuffer[2] = {pos[0] + moveVector[0],
                                 pos[1] + moveVector[1]};
        while(isOnBoard(positionBuffer)){
          if(isupper(b->pieces[positionBuffer[1]][positionBuffer[0]])) break;

          appendMoveList(ml, (char[]){col,
                                      row,
                                      'A' + positionBuffer[0],
                                      '8' - positionBuffer[1],
                                      '\0'});

          if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

          positionBuffer[0] = positionBuffer[0] + moveVector[0];
          positionBuffer[1] = positionBuffer[1] + moveVector[1];
        }
      }
    }
    break;

  case 'r':
    for(int direction = 0; direction < 4; direction++){
      int moveVector[2] = {rookMoveVectors[direction][0],
                           rookMoveVectors[direction][1]};

      if(fitsRestriction(moveVector, moveRestriction)){
        int positionBuffer[2] = {pos[0] + moveVector[0],
                                 pos[1] + moveVector[1]};
        while(isOnBoard(positionBuffer)){
          if(islower(b->pieces[positionBuffer[1]][positionBuffer[0]])) break;

          appendMoveList(ml, (char[]){col,
                                      row,
                                      'A' + positionBuffer[0],
                                      '8' - positionBuffer[1],
                                      '\0'});

          if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

          positionBuffer[0] = positionBuffer[0] + moveVector[0];
          positionBuffer[1] = positionBuffer[1] + moveVector[1];
        }
      }
    }
    break;

  case 'R':
    for(int direction = 0; direction < 4; direction++){
      int moveVector[2] = {rookMoveVectors[direction][0],
                           rookMoveVectors[direction][1]};

      if(fitsRestriction(moveVector, moveRestriction)){
        int positionBuffer[2] = {pos[0] + moveVector[0],
                                 pos[1] + moveVector[1]};
        while(isOnBoard(positionBuffer)){
          if(isupper(b->pieces[positionBuffer[1]][positionBuffer[0]])) break;

          appendMoveList(ml, (char[]){col,
                                      row,
                                      'A' + positionBuffer[0],
                                      '8' - positionBuffer[1],
                                      '\0'});

          if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

          positionBuffer[0] = positionBuffer[0] + moveVector[0];
          positionBuffer[1] = positionBuffer[1] + moveVector[1];
        }
      }
    }
    break;

  case 'q':
    //bishop
    for(int direction = 0; direction < 4; direction++){
      int moveVector[2] = {bishopMoveVectors[direction][0],
                           bishopMoveVectors[direction][1]};

      if(fitsRestriction(moveVector, moveRestriction)){
        int positionBuffer[2] = {pos[0] + moveVector[0],
                                 pos[1] + moveVector[1]};
        while(isOnBoard(positionBuffer)){
          if(islower(b->pieces[positionBuffer[1]][positionBuffer[0]])) break;

          appendMoveList(ml, (char[]){col,
                                      row,
                                      'A' + positionBuffer[0],
                                      '8' - positionBuffer[1],
                                      '\0'});

          if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

          positionBuffer[0] = positionBuffer[0] + moveVector[0];
          positionBuffer[1] = positionBuffer[1] + moveVector[1];
        }
      }
    }
    //rook
    for(int direction = 0; direction < 4; direction++){
      int moveVector[2] = {rookMoveVectors[direction][0],
                           rookMoveVectors[direction][1]};

      if(fitsRestriction(moveVector, moveRestriction)){
        int positionBuffer[2] = {pos[0] + moveVector[0],
                                 pos[1] + moveVector[1]};
        while(isOnBoard(positionBuffer)){
          if(islower(b->pieces[positionBuffer[1]][positionBuffer[0]])) break;

          appendMoveList(ml, (char[]){col,
                                      row,
                                      'A' + positionBuffer[0],
                                      '8' - positionBuffer[1],
                                      '\0'});

          if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

          positionBuffer[0] = positionBuffer[0] + moveVector[0];
          positionBuffer[1] = positionBuffer[1] + moveVector[1];
        }
      }
    }
    break;

  case 'Q':
    //bishop
    for(int direction = 0; direction < 4; direction++){
      int moveVector[2] = {bishopMoveVectors[direction][0],
                           bishopMoveVectors[direction][1]};

      if(fitsRestriction(moveVector, moveRestriction)){
        int positionBuffer[2] = {pos[0] + moveVector[0],
                                 pos[1] + moveVector[1]};
        while(isOnBoard(positionBuffer)){
          if(isupper(b->pieces[positionBuffer[1]][positionBuffer[0]])) break;

          appendMoveList(ml, (char[]){col,
                                      row,
                                      'A' + positionBuffer[0],
                                      '8' - positionBuffer[1],
                                      '\0'});

          if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

          positionBuffer[0] = positionBuffer[0] + moveVector[0];
          positionBuffer[1] = positionBuffer[1] + moveVector[1];
        }
      }
    }
    //rook
    for(int direction = 0; direction < 4; direction++){
      int moveVector[2] = {rookMoveVectors[direction][0],
                           rookMoveVectors[direction][1]};

      if(fitsRestriction(moveVector, moveRestriction)){
        int positionBuffer[2] = {pos[0] + moveVector[0],
                                 pos[1] + moveVector[1]};
        while(isOnBoard(positionBuffer)){
          if(isupper(b->pieces[positionBuffer[1]][positionBuffer[0]])) break;

          appendMoveList(ml, (char[]){col,
                                      row,
                                      'A' + positionBuffer[0],
                                      '8' - positionBuffer[1],
                                      '\0'});

          if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

          positionBuffer[0] = positionBuffer[0] + moveVector[0];
          positionBuffer[1] = positionBuffer[1] + moveVector[1];
        }
      }
    }
    break;


  case 'k':
    //bishop one step
    for(int direction = 0; direction < 4; direction++){

      int destination[2] = {pos[0] + bishopMoveVectors[direction][0],
                            pos[1] + bishopMoveVectors[direction][1]};
      if (isKingMovePossible(b, 'a', pos, destination)){
        appendMoveList(ml, (char[]){col,
                                    row,
                                    'A' + destination[0],
                                    '8' - destination[1],
                                    '\0'});
      }
    }

    //rook one step
    for(int direction = 0; direction < 4; direction++){
      int destination[2] = {pos[0] + rookMoveVectors[direction][0],
                            pos[1] + rookMoveVectors[direction][1]};
      if (isKingMovePossible(b, 'a', pos, destination)){
        appendMoveList(ml, (char[]){col,
                                    row,
                                    'A' + destination[0],
                                    '8' - destination[1],
                                    '\0'});

        //castling
        if(direction == 0 &&
           b->canWhiteCastle[0] &&
           b->pieces[7][1] == ' ' &&
           b->pieces[7][2] == ' ' &&
           b->pieces[7][3] == ' ' &&
           isKingMovePossible(b, 'a', pos,
                              (const int[2]) {destination[0]-1,
                                              destination[1]}))
        {
          appendMoveList(ml, (char[]){col,
                                      row,
                                      'A' + destination[0] -1,
                                      '8' - destination[1],
                                      '\0'});
        } else if(direction == 2 &&
                  b->canWhiteCastle[1] &&
                  b->pieces[7][5] == ' ' &&
                  b->pieces[7][6] == ' ' &&
                  isKingMovePossible(b, 'a', pos,
                                     (const int[2]) {destination[0]+1,
                                                     destination[1]}))
        {
          appendMoveList(ml, (char[]){col,
                                      row,
                                      'A' + destination[0] +1,
                                      '8' - destination[1],
                                      '\0'});
        }
      }
    }
    break;

  case 'K':
    //bishop one step
    for(int direction = 0; direction < 4; direction++){

      int destination[2] = {pos[0] + bishopMoveVectors[direction][0],
                            pos[1] + bishopMoveVectors[direction][1]};
      if (isKingMovePossible(b, 'A', pos, destination)){
        appendMoveList(ml, (char[]){col,
                                    row,
                                    'A' + destination[0],
                                    '8' - destination[1],
                                    '\0'});
      }
    }

    //rook one step
    for(int direction = 0; direction < 4; direction++){
      int destination[2] = {pos[0] + rookMoveVectors[direction][0],
                            pos[1] + rookMoveVectors[direction][1]};
      if (isKingMovePossible(b, 'A', pos, destination)){
        appendMoveList(ml, (char[]){col,
                                    row,
                                    'A' + destination[0],
                                    '8' - destination[1],
                                    '\0'});

        //castling
        if(direction == 0 &&
           b->canBlackCastle[0] &&
           b->pieces[0][1] == ' ' &&
           b->pieces[0][2] == ' ' &&
           b->pieces[0][3] == ' ' &&
           isKingMovePossible(b, 'A', pos,
                              (const int[2]) {destination[0]-1,
                                              destination[1]}))
        {
          appendMoveList(ml, (char[]){col,
                                      row,
                                      'A' + destination[0] -1,
                                      '8' - destination[1],
                                      '\0'});
        } else if(direction == 2 &&
                  b->canBlackCastle[1] &&
                  b->pieces[0][5] == ' ' &&
                  b->pieces[0][6] == ' ' &&
                  isKingMovePossible(b, 'A', pos,
                                     (const int[2]) {destination[0]+1,
                                                     destination[1]}))
        {
          appendMoveList(ml, (char[]){col,
                                      row,
                                      'A' + destination[0] +1,
                                      '8' - destination[1],
                                      '\0'});
        }
      }
    }
    break;
  }
}


bool isEnPassant(const char* input, const Tboard* b)
{
  // returns true if move is diagonally by pawn and lands on empty square
  return ((b->pieces['8' - input[1]][input[0]-'A'] == 'p' ||
           b->pieces['8' - input[1]][input[0]-'A'] == 'P') &&
          (b->pieces['8' - input[3]][input[2]-'A'] == ' ' &&
           input[0] != input[2]));
}


bool isEnPassantLegal(const Tboard* b,
                      const int pos[2],
                      const int moveVector[2],
                      int moveRestriction,
                      char oppColor,
                      const int myKingPos[2])
{
  //if not enpassanting possible at first glance
  if(!(abs(b->lastMove[1] - b->lastMove[3]) == 2 &&
       (getLastMovedPiece(b) == (oppColor + 15)) &&
       (b->lastMove[3] == ('8' - pos[1])) &&
        b->lastMove[2] == ('A' + pos[0]  + moveVector[0]) &&
       (fitsRestriction(moveVector, moveRestriction))))
    return false;

  //if taken piece is blocking king and enpassanting piece doesnt
  //cover it afterwards
  if(isBlockingCheck(b, oppColor,
                     (const int[2]){b->lastMove[2] - 'A',
                                    '8' - b->lastMove[3]},
                     myKingPos) &&
     myKingPos[1] == (pos[1] + moveVector[1]))
    return false;


  //takes care of this pos enpassanting: 'R', ' ', 'p', 'P', ' ', 'k'
    
  int leftOffset = 0, rightOffset = 0;
  //if enpassanting to the left
  if(moveVector[0] == -1){
    leftOffset = 1;
  } else {
    rightOffset = 1;
  }
    // if king has same vertical index
  if(myKingPos[1] == pos[1]){
    if(pos[0] < myKingPos[0]){

      for(int i = pos[0] - leftOffset - 1; i >= 0; i--){
        if(b->pieces[pos[1]][i] == oppColor + ('r' - 'a') ||
           b->pieces[pos[1]][i] == oppColor + ('q' - 'a')) {
            for(int i = pos[0] + rightOffset + 1; i < myKingPos[0]; i++){
              if(b->pieces[pos[1]][i] != ' ') return true;
            }
            return false;
          }

        if(b->pieces[pos[1]][i] != ' ') return true;
      }

      return true;

    } else {

      for(int i = pos[0] + rightOffset + 1; i <= 7; i++){
        if(b->pieces[pos[1]][i] == oppColor + ('r' - 'a') ||
           b->pieces[pos[1]][i] == oppColor + ('q' - 'a')){
              for(int j = pos[0] - leftOffset - 1; j > myKingPos[0]; j--){
                if(b->pieces[pos[1]][j] != ' ') return true;
              }
              return false;
           }

        if(b->pieces[pos[1]][i] != ' ') return true;
      }

      return true;
    }
  }

  return true;
}


bool doesMoveBlockCheck(const Tboard* b,
                        const char *move,
                        const int kingPos[2])
{
  char oppColor = oppositeColor(b->pieces[kingPos[1]][kingPos[0]] -
                                ('k' - 'a'));

  Tboard *copy = copyBoard(b);

  //make move
  if(isEnPassant(move, copy)){
    copy->pieces['8' - copy->lastMove[3]][copy->lastMove[2] - 'A'] = ' ';
  }
  copy->pieces['8' - move[3]]
              [move[2] - 'A'] = copy->pieces['8' - move[1]]
                                            [move[0] - 'A'];
  copy->pieces['8' - move[1]][move[0] - 'A'] = ' ';

  if(isAttacked(copy, oppColor, kingPos)){
    freeBoard(copy);
    return false;
  }
  freeBoard(copy);
  return true;
}


bool gotChecked(const Tboard *b, const int myKingPos[2])
{
  //is Checked by moved piece
  int attPos[2] = {b->lastMove[2] - 'A', '8' - b->lastMove[3]};

  switch(b->pieces[attPos[1]][attPos[0]]){
  case 'p':
    if(attPos[0] +1 == myKingPos[0] && attPos[1] -1 == myKingPos[1])
      return true;
    if(attPos[0] -1 == myKingPos[0] && attPos[1] -1 == myKingPos[1])
      return true;
    break;

  case 'P':
    if(attPos[0] +1 == myKingPos[0] && attPos[1] +1 == myKingPos[1])
      return true;
    if(attPos[0] -1 == myKingPos[0] && attPos[1] +1 == myKingPos[1])
      return true;
    break;

  case 'N':
  case 'n':
    for(int i = 0; i < 8; i++){
      if(isOnBoard((const int[2]) {myKingPos[1] + knightMoveVectors[i][1],
                                   myKingPos[0] + knightMoveVectors[i][0]}))
      {
        if(attPos[0] == knightMoveVectors[i][0] + myKingPos[0] &&
           attPos[1]  == knightMoveVectors[i][1] + myKingPos[1])
          return true;
      }
    }
    break;

  case 'b':
  case 'B':
    for(int direction = 0; direction < 4; direction++){
      int moveVector[2] = {bishopMoveVectors[direction][0],
                           bishopMoveVectors[direction][1]};

      int positionBuffer[2] = {myKingPos[0] + moveVector[0],
                               myKingPos[1] + moveVector[1]};
      while(isOnBoard(positionBuffer)){
        if(positionBuffer[1] == attPos[1] &&
           positionBuffer[0] == attPos[0]) return true;

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

        positionBuffer[0] = positionBuffer[0] + moveVector[0];
        positionBuffer[1] = positionBuffer[1] + moveVector[1];
      }
    }
    break;

  case 'r':
  case 'R':
    for(int direction = 0; direction < 4; direction++){
      int moveVector[2] = {rookMoveVectors[direction][0],
                           rookMoveVectors[direction][1]};

      int positionBuffer[2] = {myKingPos[0] + moveVector[0],
                               myKingPos[1] + moveVector[1]};
      while(isOnBoard(positionBuffer)){
        if(positionBuffer[1] == attPos[1] &&
           positionBuffer[0] == attPos[0]) return true;

      if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

      positionBuffer[0] = positionBuffer[0] + moveVector[0];
      positionBuffer[1] = positionBuffer[1] + moveVector[1];
      }
    }
    break;

  case 'q':
  case 'Q':
    //bishop
    for(int direction = 0; direction < 4; direction++){
      int moveVector[2] = {bishopMoveVectors[direction][0],
                           bishopMoveVectors[direction][1]};

      int positionBuffer[2] = {myKingPos[0] + moveVector[0],
                               myKingPos[1] + moveVector[1]};
      while(isOnBoard(positionBuffer)){
        if(positionBuffer[1] == attPos[1] &&
           positionBuffer[0] == attPos[0])
          return true;

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

        positionBuffer[0] = positionBuffer[0] + moveVector[0];
        positionBuffer[1] = positionBuffer[1] + moveVector[1];
      }
    }

    //rook
    for(int direction = 0; direction < 4; direction++){
      int moveVector[2] = {rookMoveVectors[direction][0],
                           rookMoveVectors[direction][1]};

      int positionBuffer[2] = {myKingPos[0] + moveVector[0],
                               myKingPos[1] + moveVector[1]};
      while(isOnBoard(positionBuffer)){
        if(positionBuffer[1] == attPos[1] &&
           positionBuffer[0] == attPos[0])
          return true;

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

        positionBuffer[0] = positionBuffer[0] + moveVector[0];
        positionBuffer[1] = positionBuffer[1] + moveVector[1];
      }
    }
    break;

  case 'k':
  case 'K':
    
    if(b->lastMove[0] == 'E'){
      
      int columnUnderAttack = -1;

      if(b->lastMove[2] == 'C'){
        columnUnderAttack = 3;
      } else if(b->lastMove[2] == 'G'){
        columnUnderAttack = 5;
      } else {
        break;
      }

      char attackedPiece = switchCase(b->pieces[attPos[1]][attPos[0]]);

      for(int i = 1; i < 8; i++){
        if(b->pieces[i][columnUnderAttack] != ' '){
          if(b->pieces[i][columnUnderAttack] == attackedPiece){
            return true;
          } else {
            break;
          }
        }
      }
    }
    break;
  }


  //is checked by piece uncovered by moved piece
  int attPastPos[2] = {b->lastMove[0] - 'A', '8' - b->lastMove[1]};

  char myColor = (b->pieces[myKingPos[1]][myKingPos[0]]) - ('k'-'a');
  // if moved from pos diagonally reachable from king pos
  if(abs(myKingPos[0] - attPastPos[0]) == abs(myKingPos[1] - attPastPos[1])){

    int moveVector[2] = {attPastPos[0]-myKingPos[0],
                         attPastPos[1]-myKingPos[1]};
    normalize(moveVector);
    int positionBuffer[2] = {myKingPos[0] + moveVector[0],
                             myKingPos[1] + moveVector[1]};

    if(myColor == 'a'){
      while(isOnBoard(positionBuffer)){

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] == 'B' ||
           b->pieces[positionBuffer[1]][positionBuffer[0]] == 'Q')
          return true;

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

        positionBuffer[0] = positionBuffer[0] + moveVector[0];
        positionBuffer[1] = positionBuffer[1] + moveVector[1];
      }
    } else {
      while(isOnBoard(positionBuffer)){

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] == 'b' ||
           b->pieces[positionBuffer[1]][positionBuffer[0]] == 'q')
          return true;

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

        positionBuffer[0] = positionBuffer[0] + moveVector[0];
        positionBuffer[1] = positionBuffer[1] + moveVector[1];
      }
    }

            // if moved from pos straightly reachable from king pos
  } else if(myKingPos[0] == attPastPos[0] || myKingPos[1] == attPastPos[1]){

    int moveVector[2] = {attPastPos[0]-myKingPos[0],
                         attPastPos[1]-myKingPos[1]};
    normalize(moveVector);
    int positionBuffer[2] = {myKingPos[0] + moveVector[0],
                             myKingPos[1] + moveVector[1]};

    if(myColor == 'a'){
      while(isOnBoard(positionBuffer)){

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] == 'R' ||
           b->pieces[positionBuffer[1]][positionBuffer[0]] == 'Q')
          return true;

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

        positionBuffer[0] = positionBuffer[0] + moveVector[0];
        positionBuffer[1] = positionBuffer[1] + moveVector[1];
      }
    } else {
      while(isOnBoard(positionBuffer)){

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] == 'r' ||
           b->pieces[positionBuffer[1]][positionBuffer[0]] == 'q')
          return true;

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

        positionBuffer[0] = positionBuffer[0] + moveVector[0];
        positionBuffer[1] = positionBuffer[1] + moveVector[1];
      }
    }


  }

  return false;
}


bool isInputValid(const char *input, Tboard *b)
{
  TmoveList *moveList = initMoveList(16);
  generateAllPossibleMoves(b, moveList);

  for(int i = 0; i < moveList->filled; i++){
    if(strcmp(moveList->moves[i], input) == 0){
      freeMoveList(moveList);
      return true;
    }
  }

  freeMoveList(moveList);
  return false;
}


char* getPieceGraphics(char piece)
{
  switch(piece)
  {
  case 'k':
    return "♚";
  case 'K':
    return BLACK_FOREGROUND("♚");
  case 'q':
    return "♛";
  case 'Q':
    return BLACK_FOREGROUND("♛");
  case 'b':
    return "♝";
  case 'B':
    return BLACK_FOREGROUND("♝");
  case 'n':
    return "♞";
  case 'N':
    return BLACK_FOREGROUND("♞");
  case 'r':
    return "♜";
  case 'R':
    return BLACK_FOREGROUND("♜");
  case 'p':
    return "♟";
  case 'P':
    return BLACK_FOREGROUND("♟");
  default:
    return " ";
  }
}


bool isOnBoard(const int vector[2]){
  return (vector[0] >= 0 && vector[0] < 8 && vector[1] >= 0 && vector[1] < 8);
}


bool isKingMovePossible(const Tboard* b,
                        const char color,
                        const int origin[2],
                        const int dest[2])
{
  if(isOnBoard(dest)){
    return ((!willBeAttacked(b, oppositeColor(color), origin, dest)) &&
            !(b->pieces[dest[1]][dest[0]] >= color &&
              b->pieces[dest[1]][dest[0]] <= (color+25)));
  }
  return false;
}


bool isKnightMovePossible(const Tboard* b,
                         const char color,
                         const int pos[2],
                         const int moveVector[2])
{
  if(isOnBoard((const int[2]) {pos[1] + moveVector[1],
                               pos[0] + moveVector[0]}))
    return ((b->pieces[pos[1] + moveVector[1]]
                      [pos[0] + moveVector[0]] < color) ||
            (b->pieces[pos[1] + moveVector[1]]
                      [pos[0] + moveVector[0]] > (color + 25)));
  return false;
}


bool isMoveClean(const Tboard* b,
                 const int pos[2],
                 const int moveVector[2],
                 const int restriction)
{
  if(isOnBoard((const int[2]) {pos[1] + moveVector[1],
                               pos[0] + moveVector[0]}))
    return (b->pieces[pos[1] + moveVector[1]][pos[0] + moveVector[0]] == ' ' &&
            fitsRestriction(moveVector, restriction));
  return false;
}


bool isMoveTake(const Tboard* b,
                const char opponentColor,
                const int pos[2],
                const int moveVector[2],
                const int restriction)
{
  if(isOnBoard((const int[2]) {pos[1] + moveVector[1],
                               pos[0] + moveVector[0]}))
    return (b->pieces[pos[1] + moveVector[1]]
                     [pos[0] + moveVector[0]] >= opponentColor &&
            b->pieces[pos[1] + moveVector[1]]
                     [pos[0] + moveVector[0]] <= (opponentColor + 25) &&
            fitsRestriction(moveVector, restriction));
  return false;
}


bool isBlockingCheck(const Tboard* b,
                     const char oppColor,
                     const int pos[2],
                     const int kingPos[2])
{
  char attacker;

  int moveVector[2] = {
    pos[0] - kingPos[0],
    pos[1] - kingPos[1]
  };

  if(moveVector[0] == 0 || moveVector[1] == 0){
    attacker = 'r';
  } else if(abs(moveVector[0]) == abs(moveVector[1])){
    attacker = 'b';
  } else {
    return false;
  }

  normalize(moveVector);


  //bishop
  int positionBuffer[2] = {kingPos[0] + moveVector[0],
                           kingPos[1] + moveVector[1]};
  bool inLineWithKing = false;
  while(isOnBoard(positionBuffer)){
    if(inLineWithKing &&
       ((b->pieces[positionBuffer[1]]
                  [positionBuffer[0]] == (attacker - 'a') + oppColor) ||
        (b->pieces[positionBuffer[1]]
                  [positionBuffer[0]] == ('q' - 'a') + oppColor)))
      return true;

    if(inLineWithKing &&
       b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ')
        break;

    if(positionBuffer[0] == pos[0] &&
        positionBuffer[1] == pos[1]) inLineWithKing = true;

    if(!inLineWithKing &&
       b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ')
        break;

    positionBuffer[0] = positionBuffer[0] + moveVector[0];
    positionBuffer[1] = positionBuffer[1] + moveVector[1];
  }
  return false;
}


void getPieceLocation(const Tboard* b, const char piece, int returnedPos[2])
{
  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 8; j++){
      if(b->pieces[i][j] == piece){
        returnedPos[0] = j;
        returnedPos[1] = i;
        return;
      }
    }
  }
  returnedPos[0] = -1;
  returnedPos[1] = -1;
  return;
}


bool fitsRestriction(const int moveVector[2], int restriction)
{
  switch (restriction){
  case 0:
    return true;

  case 1:  //(|)
    return (moveVector[0] == 0 && moveVector[1] != 0);

  case 2:  //(/)
    return (moveVector[0] == -moveVector[1]);

  case 3:  //(-)
    return (moveVector[0] != 0 && moveVector[1] == 0);

  case 4:  //(\)
    return (moveVector[0] == moveVector[1]);

  default:
    return false;
  }
  return false;
}


bool isAttacked(const Tboard *b, const char oppColor, const int dest[2])
{
  //by pawn
  if(oppColor == 'a'){
    int attPos[2] = {dest[0] + 1, dest[1] + 1}; //one move left and up to pos
    if(isOnBoard(attPos)){
      if(b->pieces[attPos[1]][attPos[0]] == 'p') return true;
    }

    attPos[0] = dest[0] - 1;
    if(isOnBoard(attPos)){
      if(b->pieces[attPos[1]][attPos[0]] == 'p') return true;
    }
  } else {
    int attPos[2] = {dest[0] + 1, dest[1] - 1}; //one move left and down to pos
    if(isOnBoard(attPos)){
      if(b->pieces[attPos[1]][attPos[0]] == 'P') return true;
    }

    attPos[0] = dest[0] - 1;
    if(isOnBoard(attPos)){
      if(b->pieces[attPos[1]][attPos[0]] == 'P') return true;
    }
  }

  //by knight
  char piece = oppColor + ('n' - 'a');
  for(int i = 0; i < 8; i++){
    if(isOnBoard((const int[2]){knightMoveVectors[i][1]+dest[1],
                                knightMoveVectors[i][0]+dest[0]}))
      if(b->pieces[dest[1] + knightMoveVectors[i][1]]
                  [dest[0] + knightMoveVectors[i][0]] == piece)
        return true;
  }

  //by bishop or queen as bishop or king as bishop
  if(oppColor == 'a'){
    for(int i = 0; i < 4; i++){
      int moveVector[2] = {bishopMoveVectors[i][0], bishopMoveVectors[i][1]};
      int positionBuffer[2] = {dest[0] + moveVector[0],
                               dest[1] + moveVector[1]};

      if(isOnBoard(positionBuffer))
        if(b->pieces[positionBuffer[1]][positionBuffer[0]] == 'k')
          return true;

      while(isOnBoard(positionBuffer)){
        if(isupper(b->pieces[positionBuffer[1]][positionBuffer[0]])) break;

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] == 'b' ||
           b->pieces[positionBuffer[1]][positionBuffer[0]] == 'q')
          return true;

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

        positionBuffer[0] = positionBuffer[0] + moveVector[0];
        positionBuffer[1] = positionBuffer[1] + moveVector[1];
      }
    }
  } else {
    for(int i = 0; i < 4; i++){
      int moveVector[2] = {bishopMoveVectors[i][0], bishopMoveVectors[i][1]};
      int positionBuffer[2] = {dest[0] + moveVector[0],
                               dest[1] + moveVector[1]};

      if(isOnBoard(positionBuffer))
        if(b->pieces[positionBuffer[1]][positionBuffer[0]] == 'K')
          return true;

      while(isOnBoard(positionBuffer)){
        if(islower(b->pieces[positionBuffer[1]][positionBuffer[0]])) break;

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] == 'B' ||
           b->pieces[positionBuffer[1]][positionBuffer[0]] == 'Q')
          return true;

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

        positionBuffer[0] = positionBuffer[0] + moveVector[0];
        positionBuffer[1] = positionBuffer[1] + moveVector[1];
      }
    }
  }

  //by rook or queen as rook or king as rook
  if(oppColor == 'a'){
    for(int i = 0; i < 4; i++){
      int moveVector[2] = {rookMoveVectors[i][0], rookMoveVectors[i][1]};
      int positionBuffer[2] = {dest[0] + moveVector[0],
                               dest[1] + moveVector[1]};

      if(isOnBoard(positionBuffer))
        if(b->pieces[positionBuffer[1]][positionBuffer[0]] == 'k')
          return true;

      while(isOnBoard(positionBuffer)){
        if(isupper(b->pieces[positionBuffer[1]][positionBuffer[0]])) break;

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] == 'r' ||
           b->pieces[positionBuffer[1]][positionBuffer[0]] == 'q')
          return true;

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

        positionBuffer[0] = positionBuffer[0] + moveVector[0];
        positionBuffer[1] = positionBuffer[1] + moveVector[1];
      }
    }
  } else {
    for(int i = 0; i < 4; i++){
      int moveVector[2] = {rookMoveVectors[i][0], rookMoveVectors[i][1]};
      int positionBuffer[2] = {dest[0] + moveVector[0],
                               dest[1] + moveVector[1]};

      if(isOnBoard(positionBuffer))
        if(b->pieces[positionBuffer[1]][positionBuffer[0]] == 'K')
          return true;

      while(isOnBoard(positionBuffer)){
        if(islower(b->pieces[positionBuffer[1]][positionBuffer[0]])) break;

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] == 'R' ||
           b->pieces[positionBuffer[1]][positionBuffer[0]] == 'Q')
          return true;

        if(b->pieces[positionBuffer[1]][positionBuffer[0]] != ' ') break;

        positionBuffer[0] = positionBuffer[0] + moveVector[0];
        positionBuffer[1] = positionBuffer[1] + moveVector[1];
      }
    }
  }

  return false;
}


bool willBeAttacked(const Tboard *b,
                    const char color,
                    const int origin[2],
                    const int dest[2])
{
  Tboard *copy = copyBoard(b);

  copy->pieces[origin[1]][origin[0]] = ' ';

  if(isAttacked(copy, color, dest)){
    freeBoard(copy);
    return true;
  } else {
    freeBoard(copy);
    return false;
  }
}


void disableCastling(Tboard *b, char color, bool buffer[2])
{
  if(color == 'a'){
    buffer[0] = b->canWhiteCastle[0];
    buffer[1] = b->canWhiteCastle[1];
    b->canWhiteCastle[0] = b->canWhiteCastle[1] = false;
  }else{
    buffer[0] = b->canBlackCastle[0];
    buffer[1] = b->canBlackCastle[1];
    b->canBlackCastle[0] = b->canBlackCastle[1] = false;
  }
}


void regainCastling(Tboard *b, char color, bool buffer[2])
{
  if(color == 'a'){
    b->canWhiteCastle[0] = buffer[0];
    b->canWhiteCastle[1] = buffer[1];
  }else{
    b->canBlackCastle[0] = buffer[0];
    b->canBlackCastle[1] = buffer[1];
  }
}


char oppositeColor(const char color)
{
  return (color + 2*((('a'+'A')/2)-color));
}


bool isColor(const char color, const char piece)
{
  return (piece >= color && piece <= color + 32);
}


void normalize(int moveVector[2])
{
  int absMax;
  if(abs(moveVector[0]) > abs(moveVector[1])){
    absMax = abs(moveVector[0]);
  } else {
    absMax = abs(moveVector[1]);
  }

  if(absMax > 1){
    for(int i = 0; i < 2; i++){
      moveVector[i] = moveVector[i]/absMax;
    }
  }
}


bool isArrayInArrayOfArrays(int *array,
                            int **arrayOfArrays,
                            int ArrLen,
                            int ArrOfArrsLen)
{
  for(int i = 0; i < ArrOfArrsLen; i++) {
    bool isSame = true;
    for(int j = 0; j < ArrLen; j++) {
      isSame = isSame && (array[j] == arrayOfArrays[i][j]);
    }
    if(isSame) return true;
  }

  return false;
}


bool isStringTwiceInArrayOfStrings(char *array,
                                   char **arrayOfArrays,
                                   int ArrOfArrsLen)
{
  bool wasOneSame = false;

  for(int i = 0; i < ArrOfArrsLen; i++) {
    if(strcmp(array, arrayOfArrays[i]) == 0){
      if(wasOneSame){
        return true;
      }
      wasOneSame = true;
    }
  }

  return false;
}


void freeArrayOfStrings(char **arr, int len)
{
  for(int i = 0; i < len; i++){
    free(arr[i]);
  }
  free(arr);
}
