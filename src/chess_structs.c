/**
 * Project:  neural chess
 * Author:   Jakub Urbanek
 * Year:     2022
 * 
 * note:     copied from my minimax-chess project
 */

#include "chess_structs.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>


Tboard* initBoard()
{
  Tboard *b = malloc(sizeof(Tboard));
  b->canBlackCastle[0] = true;
  b->canBlackCastle[1] = true;
  b->canWhiteCastle[0] = true;
  b->canWhiteCastle[1] = true;
  b->lastMove = malloc(MAX_INP_LEN * sizeof(char));
  strcpy(b->lastMove, "0000");
  b->move = 0;
  b->boringMoveCount = 0;
  b->boringPoss = malloc(0);
  b->pieceCount = 32;

  char temp[8][8] = {
    {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'},
    {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
    {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'}
  };
  for (int i = 0; i < 8; i++){
    for (int j = 0; j < 8; j++){
      b->pieces[i][j] = temp[i][j];
    }
  }
  return b;
}

Tboard* copyBoard(const Tboard *input)
{
  Tboard *b = malloc(sizeof(Tboard));
  b->canBlackCastle[0] = input->canBlackCastle[0];
  b->canBlackCastle[1] = input->canBlackCastle[1];
  b->canWhiteCastle[0] = input->canWhiteCastle[0];
  b->canWhiteCastle[1] = input->canWhiteCastle[1];
  b->lastMove = malloc(MAX_INP_LEN * sizeof(char));
  strcpy(b->lastMove, input->lastMove);
  b->move = input->move;
  b->boringMoveCount = input->boringMoveCount;

  b->boringPoss = malloc(b->boringMoveCount * sizeof(char*));
  for(int i = 0; i < b->boringMoveCount; i++){
    b->boringPoss[i] = malloc(POS_STRING_LEN * sizeof(char));
    strcpy(b->boringPoss[i], input->boringPoss[i]);
  }

  b->pieceCount = input->pieceCount;

  for (int i = 0; i < 8; i++){
    for (int j = 0; j < 8; j++){
      b->pieces[i][j] = input->pieces[i][j];
    }
  }

  return b;
}

Tboard* fenToBoard(char *fen)
{
  int fenLen = strlen(fen);
  
  Tboard *b = malloc(sizeof(Tboard));
  int index = 0;

  b->pieceCount = 0;
  b->boringMoveCount = 0;
  b->boringPoss = malloc(0);
  
  b->canBlackCastle[0] = b->canBlackCastle[1] =\
  b->canWhiteCastle[0] = b->canWhiteCastle[1] = false;


  int i = 0, j = 0;
  //pieces
  while(fen[index] != ' '){
    if(fen[index] == '/')
      index++;
    while(fen[index] != '/' && fen[index] != ' '){
      if(isalpha(fen[index])){
        b->pieces[i][j] = switchCase(fen[index]);
        b->pieceCount++;
        j++;
      } else if(isdigit(fen[index])){
        for(int k = 0; k < (fen[index] - '0'); k++){
          b->pieces[i][j + k] = ' ';
        }
        j += (fen[index] - '0');
      } else {
        free(b);
        return NULL;
      }
      index++;
    }
    i++;

    if(j == 8){
      j = 0;
    } else{
      free(b);
      return NULL;
    }
  }
  if(i != 8){
    free(b);
    return NULL;
  }

  //testing if both kings are present
  bool isKOnBoard = false, iskOnBoard = false;
  for(int r = 0; r < 8; r++){
    for(int c = 0; c < 8; c++){
      iskOnBoard = (b->pieces[r][c] == 'k') || iskOnBoard;
      isKOnBoard = (b->pieces[r][c] == 'K') || isKOnBoard;
    }
  }
  if(!(iskOnBoard && isKOnBoard)){
    free(b);
    return NULL;
  }


  index++;
  if(fen[index] == 'w'){
    b->move = 0;
  } else if(fen[index] == 'b'){
    b->move = 1;
  }
  index++;
  if(fenLen-1 <= index){
    return b;
  }
  if(fen[index] != ' '){
    free(b);
    return NULL;
  }
  index++;

  if(fenLen-1 <= index){
    return b;
  }

  
  //castling
  if(fen[index] == '-'){
    index++;
  } else {
    while(fen[index] != ' '){
      switch (fen[index])
      {
      case 'K':
        b->canWhiteCastle[1] = true;
        break;

      case 'Q':
        b->canWhiteCastle[0] = true;
        break;
      
      case 'k':
        b->canBlackCastle[1] = true;
        break;

      case 'q':
        b->canBlackCastle[0] = true;
        break;

      default:
        free(b);
        return NULL;
      }
      index++;
    }
  }

  index++;
  if(fenLen-1 <= index){
    return b;
  }


  //lastMove
  char temp[2];
  if(fen[index] == '-'){
    b->lastMove = malloc(MAX_INP_LEN * sizeof(char));
    strcpy(b->lastMove, "0000");

  } else {
    if(isalpha(fen[index])){
      temp[0] = toupper(fen[index]);
      index++;
    } else {
      free(b);
      return NULL;
    }

    if(isdigit(fen[index])){
      temp[1] = fen[index];
      index++;
    } else {
      free(b);
      return NULL;
    }
  
    b->lastMove = malloc(MAX_INP_LEN * sizeof(char));
    if(temp[1] == '3'){
      strcpy(b->lastMove, (char[]){temp[0], '2', temp[0], '4', '\0'});
    
    } else if(temp[1] == '6'){
      strcpy(b->lastMove, (char[]){temp[0], '7', temp[0], '5', '\0'});
    } else {
      free(b->lastMove);
      free(b);
      return NULL;
    }
  }

  index++;
  if(fenLen-2 <= index){
    return b;
  }
  if(fen[index] != ' '){
    free(b->lastMove);
    free(b);
    return NULL;
  }
  index++;

  if(isdigit(fen[index])){
    b->boringMoveCount = fen[index] - '0';
    b->boringPoss = realloc(b->boringPoss, b->boringMoveCount * sizeof(char*));
    for(int a = 0; a < b->boringMoveCount; a++){
      b->boringPoss[a] = malloc(POS_STRING_LEN*sizeof(char));
      for(int l = 0; l < POS_STRING_LEN-1; l++){
        b->boringPoss[a][l] = ' ';
      }
      b->boringPoss[a][POS_STRING_LEN-1] = '\0';
    }
  } else{
    b->boringMoveCount = 0;
  }

  return b;
}

void freeBoard(Tboard* b)
{
  for(int i = 0; i < b->boringMoveCount; i++){
    free(b->boringPoss[i]);
  }
  free(b->boringPoss);

  free(b->lastMove);
  free(b);
}


char* boardToPosString(const Tboard *b)
{
  char *posString = malloc(POS_STRING_LEN * sizeof(char));
  for(int i = 0; i < 8; i++) {
    for(int j = 0; j < 8; j++) {
      posString[i*8 + j] = b->pieces[i][j];
    }
  }
  posString[64] = '\0';
  return posString;
}



TmoveList* initMoveList(int n)
{
  TmoveList *ml = malloc(sizeof(TmoveList));
  ml->moves = malloc(n * sizeof(char*));
  for(int i = 0; i < n; i++){
    ml->moves[i] = malloc(MAX_INP_LEN * sizeof(char));
  }
  ml->filled = 0;
  ml->size = n;

  return ml;
}


void appendMoveList(TmoveList *ml, char* move)
{
  ml->filled++;

  // if can fit in allocated memory... else realloc
  if(ml->filled <=ml->size){
    strcpy(ml->moves[ml->filled-1], move);
    return;
  }

  //realloc
  int n = ml->size * 2;
  ml->moves = realloc(ml->moves, n * sizeof(*ml->moves));
  for(int i = ml->size; i < n; i++){
    ml->moves[i] = malloc(MAX_INP_LEN * sizeof(char));
  }
  ml->size = n;


  strcpy(ml->moves[ml->filled-1], move);
}


void freeMoveList(TmoveList* ml)
{
  for(int i = 0; i < ml->size; i++){
    free(ml->moves[i]);
  }
  free(ml->moves);
  free(ml);
}


char switchCase(char c)
{
  if(islower(c)){
    return toupper(c);
  } else if (isupper(c)){
    return tolower(c);
  }

  return c;
}
