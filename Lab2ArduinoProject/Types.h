#pragma once

#include <Arduino.h>

enum GameState {
  WAIT_OPERATION,   // Waiting for the correct boss/operator card
  SHOW_OPERATION,   // Brief boss intro before the equation
  WAIT_INPUT,
  CHECK_RESULT,
  WIN,              // Correct hit feedback
  LOSE,             // Wrong answer feedback
  BOSS_DEFEATED,    // Current operator boss HP reached 0
  GAME_OVER         // Player lives reached 0
};

enum FaceExpression {
  FACE_IDLE,
  FACE_EVIL,
  FACE_ANGRY,
  FACE_SAD,
  FACE_WIN          // Defeated robot: X eyes, drooping mouth
};

enum EyeMode {
  EYES_NORMAL,
  EYES_BLINK,
  EYES_PLUS,
  EYES_MINUS,
  EYES_MULTIPLY
};

enum OperationType {
  ADDITION,
  SUBTRACTION,
  MULTIPLICATION,
  INVALID
};

struct RFIDEntity {
  byte uid[10];
  byte size;
};

struct Operation {
  RFIDEntity tag;
  OperationType operation;
};

struct Card {
  RFIDEntity tag;
  int value;
};

struct BossLevel {
  const char* name;
  OperationType operation;
  byte maxHP;
  byte maxLives;
};
