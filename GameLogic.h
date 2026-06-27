#pragma once

#include "Types.h"

extern GameState state;

extern OperationType currentOperation;

extern int operandA;
extern int operandB;
extern int correctAnswer;
extern int playerAnswer;

extern int inputDigits[2];
extern int inputCount;

void waitOperation();

void generateOperands();

void showOperation();

void handleInput();