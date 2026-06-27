#pragma once

#include <Arduino.h>

enum GameState {
  WAIT_OPERATION,
  SHOW_OPERATION,
  WAIT_INPUT,
  CHECK_RESULT,
  WIN,
  LOSE
};

enum FaceExpression {
  FACE_IDLE,
  FACE_EVIL,
  FACE_ANGRY,
  FACE_SAD,
  FACE_WIN    // Defeated robot: X eyes, drooping mouth
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
