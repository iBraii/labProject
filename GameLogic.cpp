#include "GameLogic.h"
#include "Config.h"
#include "RFIDUtils.h"
#include "DisplayUI.h"

GameState state = WAIT_OPERATION;

OperationType currentOperation;

int operandA = 0;
int operandB = 0;
int correctAnswer = 0;
int playerAnswer  = 0;

int inputDigits[2];
int inputCount = 0;

// ── Wait for an operation tag ─────────────────────────────────────

void waitOperation() {

  if (readCard()) {

    OperationType op = getOperation(&mfrc522.uid);

    if (op != INVALID) {

      currentOperation = op;

      generateOperands();

      state = SHOW_OPERATION;

      screenDirty = true;
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
}

// ── Generate operands ─────────────────────────────────────────────
// Answers are always 0–99 (fits 2 RFID card scans).
// Subtraction always produces a non-negative result.

void generateOperands() {

  switch (currentOperation) {

    case ADDITION:
      // Both operands 1–49, sum never exceeds 98
      operandA = random(1, 50);
      operandB = random(1, 50);
      correctAnswer = operandA + operandB;
      currentEyes = EYES_PLUS;
      break;

    case SUBTRACTION:
      // operandB <= operandA guarantees result >= 0
      operandA = random(10, 99);
      operandB = random(0, operandA + 1);
      correctAnswer = operandA - operandB;
      currentEyes = EYES_MINUS;
      break;

    case MULTIPLICATION:
      // 1–9 × 1–9, max answer is 81
      operandA = random(1, 10);
      operandB = random(1, 10);
      correctAnswer = operandA * operandB;
      currentEyes = EYES_MULTIPLY;
      break;

    default:
      break;
  }

  screenDirty = true;
}

// ── Show chosen operation for a short time, then enter input ──────

void showOperation() {

  static unsigned long startTime = 0;

  if (startTime == 0) {
    startTime   = millis();
    screenDirty = true;
  }

  if (millis() - startTime >= 1500) {
    startTime   = 0;        // reset for next call
    state       = WAIT_INPUT;
    screenDirty = true;
  }
}

// ── Handle digit card scans and confirm button ────────────────────

void handleInput() {

  static bool lastBtnState = HIGH;

  bool currentBtnState = digitalRead(CONFIRM_BTN);

  // Digit card scan
  if (readCard()) {

    int value = getCardValue(&mfrc522.uid);

    if (value >= 0 && value <= 9 && inputCount < 2) {
      inputDigits[inputCount++] = value;
      screenDirty = true;
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }

  // Confirm button (active LOW with INPUT_PULLUP)
  if (lastBtnState == HIGH && currentBtnState == LOW) {

    if (inputCount == 0) {
      lastBtnState = currentBtnState;
      return;
    }

    playerAnswer = (inputCount == 1)
      ? inputDigits[0]
      : inputDigits[0] * 10 + inputDigits[1];

    inputCount  = 0;
    state       = CHECK_RESULT;
    screenDirty = true;
  }

  lastBtnState = currentBtnState;
}
