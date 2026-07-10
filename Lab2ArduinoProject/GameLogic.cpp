#include "GameLogic.h"
#include "Config.h"
#include "RFIDUtils.h"
#include "DisplayUI.h"

GameState state = WAIT_OPERATION;
OperationType currentOperation = INVALID;

int operandA = 0;
int operandB = 0;
int correctAnswer = 0;
int playerAnswer  = 0;
int lastCorrectAnswer = 0;
int lastPlayerAnswer = 0;

int inputDigits[2] = {0, 0};
int inputCount = 0;

int currentBossIndex = 0;
int bossHP = 0;
int playerLives = 0;
int score = 0;
int combo = 0;
bool allBossesCleared = false;

// Boss concept: each boss IS one operator.
// Sumi = addition, Minus = subtraction, Multiplor = multiplication.
BossLevel bosses[] = {
  {"Sumi",      ADDITION,       4, 3},
  {"Minus",     SUBTRACTION,    4, 3},
  {"Multiplor", MULTIPLICATION, 5, 3}
};

const byte BOSS_COUNT = sizeof(bosses) / sizeof(BossLevel);

const char* getCurrentBossName() {
  return bosses[currentBossIndex].name;
}

OperationType getCurrentBossOperation() {
  return bosses[currentBossIndex].operation;
}

byte getBossCount() {
  return BOSS_COUNT;
}

const char* getOperationDisplayName(OperationType operation) {
  switch (operation) {
    case ADDITION:       return "Sumi";
    case SUBTRACTION:    return "Minus";
    case MULTIPLICATION: return "Multiplor";
    default:             return "???";
  }
}

char getOperationSymbol(OperationType operation) {
  switch (operation) {
    case ADDITION:       return '+';
    case SUBTRACTION:    return '-';
    case MULTIPLICATION: return 'x';
    default:             return '?';
  }
}

void setEyesForOperation(OperationType operation) {
  switch (operation) {
    case ADDITION:       currentEyes = EYES_PLUS; break;
    case SUBTRACTION:    currentEyes = EYES_MINUS; break;
    case MULTIPLICATION: currentEyes = EYES_MULTIPLY; break;
    default:             currentEyes = EYES_NORMAL; break;
  }
}

int getBossProgress() {
  // Number of correct hits already landed on this boss.
  return bosses[currentBossIndex].maxHP - bossHP;
}

void setupGame() {
  score = 0;
  combo = 0;
  allBossesCleared = false;
  startBoss(0);
}

void startBoss(int index) {
  if (index < 0) index = 0;
  if (index >= BOSS_COUNT) index = BOSS_COUNT - 1;

  currentBossIndex = index;
  bossHP = bosses[currentBossIndex].maxHP;
  playerLives = bosses[currentBossIndex].maxLives;
  inputCount = 0;
  currentOperation = bosses[currentBossIndex].operation;

  setEyesForOperation(currentOperation);
  currentExpression = FACE_EVIL;
  state = WAIT_OPERATION;
  screenDirty = true;
}

void resetGame() {
  score = 0;
  combo = 0;
  allBossesCleared = false;
  startBoss(0);
}

void continueAfterBossDefeated() {
  if (allBossesCleared) {
    resetGame();
  } else {
    startBoss(currentBossIndex + 1);
  }
}

void prepareNextRound() {
  // Same boss, same operator, harder numbers as HP goes down.
  inputCount = 0;
  playerAnswer = 0;
  currentOperation = bosses[currentBossIndex].operation;
  generateOperands();
  state = SHOW_OPERATION;
  screenDirty = true;
}

// ── Debounced confirm button ─────────────────────────────────────
// Active LOW with INPUT_PULLUP. This returns true once per real press.
bool confirmPressed() {
  const unsigned long debounceMs = 45;

  static bool lastReading = HIGH;
  static bool stableState = HIGH;
  static unsigned long lastChangeTime = 0;

  bool reading = digitalRead(CONFIRM_BTN);
  unsigned long now = millis();

  if (reading != lastReading) {
    lastChangeTime = now;
    lastReading = reading;
  }

  if ((now - lastChangeTime) > debounceMs && reading != stableState) {
    stableState = reading;

    if (stableState == LOW) {
      return true;
    }
  }

  return false;
}

// ── Wait for the correct boss/operator tag ───────────────────────
void waitOperation() {
  if (readCard()) {
    OperationType scannedOperation = getOperation(&mfrc522.uid);
    OperationType expectedOperation = bosses[currentBossIndex].operation;

    if (scannedOperation == expectedOperation) {
      currentOperation = expectedOperation;
      generateOperands();
      state = SHOW_OPERATION;
    } else if (scannedOperation != INVALID) {
      // Valid card, but wrong boss/order. Give feedback without changing boss.
      currentExpression = FACE_ANGRY;
      currentEyes = EYES_NORMAL;
    }

    screenDirty = true;
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
}

// ── Generate operands for the current operator boss ───────────────
// Answers stay 0–99 because the project currently accepts 2 digit scans.
void generateOperands() {
  OperationType bossOperation = bosses[currentBossIndex].operation;
  int progress = getBossProgress();

  currentOperation = bossOperation;

  switch (bossOperation) {
    case ADDITION: {
      // Sumi: easy addition -> larger addition.
      int minValue = 1;
      int maxValue = 8 + progress * 10;   // 8, 18, 28, 38...
      if (maxValue > 49) maxValue = 49;

      do {
        operandA = random(minValue, maxValue + 1);
        operandB = random(minValue, maxValue + 1);
      } while (operandA + operandB > 99);

      correctAnswer = operandA + operandB;
      break;
    }

    case SUBTRACTION: {
      // Minus: always non-negative, grows from small numbers to 2-digit numbers.
      int maxA = 15 + progress * 20;      // 15, 35, 55, 75...
      if (maxA > 99) maxA = 99;
      int minA = 5 + progress * 5;
      if (minA >= maxA) minA = 5;

      operandA = random(minA, maxA + 1);
      operandB = random(0, operandA + 1);
      correctAnswer = operandA - operandB;
      break;
    }

    case MULTIPLICATION: {
      // Multiplor: starts with tiny tables, ends with full 1–9 tables.
      int minValue = 1;
      int maxValue = 3 + progress * 2;    // 3, 5, 7, 9, 9...
      if (maxValue > 9) maxValue = 9;

      operandA = random(minValue, maxValue + 1);
      operandB = random(minValue, maxValue + 1);
      correctAnswer = operandA * operandB;
      break;
    }

    default:
      break;
  }

  setEyesForOperation(currentOperation);
  currentExpression = FACE_IDLE;
  screenDirty = true;
}

// ── Show chosen boss/operator for a short time, then enter input ──
void showOperation() {
  static unsigned long startTime = 0;

  if (startTime == 0) {
    startTime = millis();
    screenDirty = true;
  }

  if (millis() - startTime >= 1200) {
    startTime = 0;
    state = WAIT_INPUT;
    screenDirty = true;
  }
}

// ── Handle digit card scans and confirm button ────────────────────
void handleInput() {
  bool pressed = confirmPressed();

  if (readCard()) {
    int value = getCardValue(&mfrc522.uid);

    if (value >= 0 && value <= 9 && inputCount < 2) {
      inputDigits[inputCount++] = value;
      currentExpression = FACE_IDLE;
      setEyesForOperation(currentOperation);
      screenDirty = true;
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }

  if (!pressed) return;

  if (inputCount == 0) {
    currentExpression = FACE_ANGRY;
    currentEyes = EYES_NORMAL;
    screenDirty = true;
    return;
  }

  playerAnswer = (inputCount == 1)
    ? inputDigits[0]
    : inputDigits[0] * 10 + inputDigits[1];

  lastPlayerAnswer = playerAnswer;
  lastCorrectAnswer = correctAnswer;

  inputCount = 0;
  state = CHECK_RESULT;
  screenDirty = true;
}

// ── Apply combat result ───────────────────────────────────────────
void resolveAnswer() {
  if (playerAnswer == correctAnswer) {
    bossHP--;
    score += 10 + (currentBossIndex * 10) + (combo * 3);
    combo++;

    currentEyes = EYES_NORMAL;

    if (bossHP <= 0) {
      bossHP = 0;
      allBossesCleared = (currentBossIndex == BOSS_COUNT - 1);
      currentExpression = FACE_WIN;
      state = BOSS_DEFEATED;
    } else {
      currentExpression = FACE_SAD;
      state = WIN;
    }
  } else {
    playerLives--;
    if (playerLives < 0) playerLives = 0;
    combo = 0;

    currentExpression = FACE_EVIL;
    currentEyes = EYES_NORMAL;

    if (playerLives <= 0) {
      state = GAME_OVER;
    } else {
      state = LOSE;
    }
  }

  screenDirty = true;
}
