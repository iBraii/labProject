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

// Each boss represents exactly one operator.
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

int getBossProgress() {
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

  // Neutral/stern face while waiting for the correct boss card.
  currentExpression = FACE_IDLE;
  currentEyes = EYES_NORMAL;
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
  inputCount = 0;
  playerAnswer = 0;
  currentOperation = bosses[currentBossIndex].operation;
  generateOperands();
  state = SHOW_OPERATION;
  screenDirty = true;
}

// ── Debounced confirm button ─────────────────────────────────────
// Active LOW with INPUT_PULLUP. Returns true once per physical press.
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
  if (!readCard()) return;

  OperationType scannedOperation = getOperation(&mfrc522.uid);
  OperationType expectedOperation = bosses[currentBossIndex].operation;

  if (scannedOperation == expectedOperation) {
    currentOperation = expectedOperation;
    generateOperands();
    state = SHOW_OPERATION;
  } else if (scannedOperation != INVALID) {
    // A valid but incorrect boss card makes the current boss taunt the player.
    currentExpression = FACE_ANGRY;
    currentEyes = EYES_NORMAL;
  }

  screenDirty = true;
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// ── Generate operands for the current operator boss ──────────────
// Answers remain from 0 to 99 because input currently supports 2 digits.
void generateOperands() {
  OperationType bossOperation = bosses[currentBossIndex].operation;
  int progress = getBossProgress();

  currentOperation = bossOperation;

  switch (bossOperation) {
    case ADDITION: {
      int minValue = 1;
      int maxValue = 8 + progress * 10;
      if (maxValue > 49) maxValue = 49;

      do {
        operandA = random(minValue, maxValue + 1);
        operandB = random(minValue, maxValue + 1);
      } while (operandA + operandB > 99);

      correctAnswer = operandA + operandB;
      break;
    }

    case SUBTRACTION: {
      int maxA = 15 + progress * 20;
      if (maxA > 99) maxA = 99;

      int minA = 5 + progress * 5;
      if (minA >= maxA) minA = 5;

      operandA = random(minA, maxA + 1);
      operandB = random(0, operandA + 1);
      correctAnswer = operandA - operandB;
      break;
    }

    case MULTIPLICATION: {
      int minValue = 1;
      int maxValue = 3 + progress * 2;
      if (maxValue > 9) maxValue = 9;

      operandA = random(minValue, maxValue + 1);
      operandB = random(minValue, maxValue + 1);
      correctAnswer = operandA * operandB;
      break;
    }

    default:
      break;
  }

  // During every equation the visible face is the angry operator boss.
  currentExpression = FACE_EVIL;
  currentEyes = EYES_NORMAL;
  screenDirty = true;
}

// ── Show the current boss/operator briefly ────────────────────────
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

// ── Handle number cards and the confirm button ───────────────────
void handleInput() {
  bool pressed = confirmPressed();

  if (readCard()) {
    int value = getCardValue(&mfrc522.uid);

    if (value >= 0 && value <= 9 && inputCount < 2) {
      inputDigits[inputCount++] = value;
      currentExpression = FACE_EVIL;
      currentEyes = EYES_NORMAL;
      screenDirty = true;
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }

  if (!pressed) return;

  if (inputCount == 0) {
    // Confirming without digits makes the boss taunt the player.
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
  currentEyes = EYES_NORMAL;

  if (playerAnswer == correctAnswer) {
    bossHP--;
    score += 10 + (currentBossIndex * 10) + (combo * 3);
    combo++;

    if (bossHP <= 0) {
      bossHP = 0;
      allBossesCleared = (currentBossIndex == BOSS_COUNT - 1);
      currentExpression = FACE_WIN;
      state = BOSS_DEFEATED;
    } else {
      // The face is the boss, so a correct answer makes it look damaged.
      currentExpression = FACE_SAD;
      state = WIN;
    }
  } else {
    playerLives--;
    if (playerLives < 0) playerLives = 0;
    combo = 0;

    // The boss dominates/taunts the player after an incorrect answer.
    currentExpression = FACE_ANGRY;

    if (playerLives <= 0) {
      state = GAME_OVER;
    } else {
      state = LOSE;
    }
  }

  screenDirty = true;
}
