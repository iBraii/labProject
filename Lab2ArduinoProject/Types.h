#pragma once

#include <Arduino.h>

enum GameState {
  WAIT_OPERATION,   // Waiting for the current boss/operator card
  SHOW_OPERATION,   // Brief boss introduction before the equation
  WAIT_INPUT,
  CHECK_RESULT,
  WIN,              // Player hit the boss: boss looks damaged
  LOSE,             // Player missed: boss taunts the player
  BOSS_DEFEATED,    // Current operator boss HP reached 0
  GAME_OVER         // Player lives reached 0
};

enum FaceExpression {
  FACE_IDLE,        // Neutral/stern boss waiting to be scanned
  FACE_EVIL,        // Active combat face
  FACE_ANGRY,       // Dominant/taunting boss face
  FACE_SAD,         // Boss hurt after losing one HP
  FACE_WIN          // Boss defeated
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
