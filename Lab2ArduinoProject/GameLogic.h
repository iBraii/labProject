#pragma once

#include "Types.h"

extern GameState state;
extern OperationType currentOperation;

extern int operandA;
extern int operandB;
extern int correctAnswer;
extern int playerAnswer;
extern int lastCorrectAnswer;
extern int lastPlayerAnswer;

extern int inputDigits[2];
extern int inputCount;

extern int currentBossIndex;
extern int bossHP;
extern int playerLives;
extern int score;
extern int combo;
extern bool allBossesCleared;

void setupGame();
void startBoss(int index);
void continueAfterBossDefeated();
void resetGame();
void prepareNextRound();

const char* getCurrentBossName();
OperationType getCurrentBossOperation();
const char* getOperationDisplayName(OperationType operation);
char getOperationSymbol(OperationType operation);
byte getBossCount();

void waitOperation();
void generateOperands();
void showOperation();
void handleInput();
void resolveAnswer();

bool confirmPressed();
