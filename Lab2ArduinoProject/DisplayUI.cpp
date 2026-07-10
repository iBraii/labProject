#include "DisplayUI.h"
#include "Config.h"
#include "GameLogic.h"

bool screenDirty = true;

FaceExpression currentExpression = FACE_EVIL;
EyeMode currentEyes = EYES_NORMAL;

// ── Eye shapes ────────────────────────────────────────────────────
void drawBaseEye(int x, int y) {
  u8g2.drawRBox(x, y, 28, 20, 6);
}

void drawIdleEyes() {
  drawBaseEye(22, 30);
  drawBaseEye(78, 30);
}

void drawBlinkEyes() {
  u8g2.drawBox(22, 40, 28, 2);
  u8g2.drawBox(78, 40, 28, 2);
}

void drawEvilEyes() {
  drawBaseEye(22, 30);
  drawBaseEye(78, 30);

  u8g2.setDrawColor(0);
  u8g2.drawTriangle(22, 30, 50, 30, 50, 50);
  u8g2.drawTriangle(78, 30, 106, 30, 78, 50);
  u8g2.setDrawColor(1);
}

void drawSadEyes() {
  drawBaseEye(22, 30);
  drawBaseEye(78, 30);

  u8g2.setDrawColor(0);
  u8g2.drawTriangle(22, 30, 50, 30, 22, 50);
  u8g2.drawTriangle(78, 30, 106, 30, 106, 50);
  u8g2.setDrawColor(1);
}

void drawAngryEyes() {
  drawBaseEye(22, 30);
  drawBaseEye(78, 30);

  u8g2.setDrawColor(0);
  u8g2.drawBox(22, 30, 28, 10);
  u8g2.drawBox(78, 30, 28, 10);
  u8g2.setDrawColor(1);
}

void drawXEyes() {
  for (int8_t t = -1; t <= 1; t++) {
    u8g2.drawLine(22, 30 + t, 50, 50 + t);
    u8g2.drawLine(22, 50 + t, 50, 30 + t);
    u8g2.drawLine(78, 30 + t, 106, 50 + t);
    u8g2.drawLine(78, 50 + t, 106, 30 + t);
  }
}

void drawOperatorEyes() {
  u8g2.setFont(u8g2_font_logisoso24_tf);

  switch (currentEyes) {
    case EYES_PLUS:
      u8g2.drawStr(28, 50, "+");
      u8g2.drawStr(84, 50, "+");
      break;
    case EYES_MINUS:
      u8g2.drawStr(30, 50, "-");
      u8g2.drawStr(86, 50, "-");
      break;
    case EYES_MULTIPLY:
      u8g2.drawStr(28, 50, "x");
      u8g2.drawStr(84, 50, "x");
      break;
    default:
      drawIdleEyes();
      break;
  }
}

void drawEyes() {
  if (currentEyes == EYES_BLINK) {
    drawBlinkEyes();
    return;
  }

  switch (currentExpression) {
    case FACE_EVIL:
      drawEvilEyes();
      break;
    case FACE_WIN:
      drawXEyes();
      break;
    case FACE_SAD:
      drawSadEyes();
      break;
    case FACE_ANGRY:
      drawAngryEyes();
      break;
    case FACE_IDLE:
    default:
      if (currentEyes == EYES_PLUS || currentEyes == EYES_MINUS || currentEyes == EYES_MULTIPLY) {
        drawOperatorEyes();
      } else {
        drawIdleEyes();
      }
      break;
  }
}

// ── Mouth ─────────────────────────────────────────────────────────
void drawMouth() {
  switch (currentExpression) {
    case FACE_IDLE:
      u8g2.drawBox(56, 76, 16, 2);
      break;
    case FACE_EVIL:
      u8g2.drawLine(54, 80, 74, 74);
      break;
    case FACE_ANGRY:
      u8g2.drawBox(54, 76, 20, 3);
      break;
    case FACE_SAD:
    case FACE_WIN:
      u8g2.drawLine(54, 74, 74, 80);
      break;
  }
}

void drawHud() {
  char hud[32];

  u8g2.setFont(u8g2_font_5x8_tf);
  snprintf(hud, sizeof(hud), "B%d/%d %s", currentBossIndex + 1, getBossCount(), getCurrentBossName());
  u8g2.drawStr(2, 8, hud);

  snprintf(hud, sizeof(hud), "HP:%d  V:%d  S:%d", bossHP, playerLives, score);
  u8g2.drawStr(2, 18, hud);
}

void drawOperationName() {
  char line[32];
  snprintf(line, sizeof(line), "%s!", getCurrentBossName());

  u8g2.setFont(u8g2_font_8x13_tf);
  u8g2.drawStr(36, 105, line);

  u8g2.setFont(u8g2_font_6x12_tf);
  snprintf(line, sizeof(line), "Operacion: %c", getOperationSymbol(currentOperation));
  u8g2.drawStr(26, 121, line);
}

void drawEquation() {
  char equation[32];
  char op = getOperationSymbol(currentOperation);

  u8g2.setFont(u8g2_font_6x12_tf);

  if (inputCount == 0) {
    snprintf(equation, sizeof(equation), "%d %c %d = _", operandA, op, operandB);
  } else if (inputCount == 1) {
    snprintf(equation, sizeof(equation), "%d %c %d = %d_", operandA, op, operandB, inputDigits[0]);
  } else {
    snprintf(equation, sizeof(equation), "%d %c %d = %d%d", operandA, op, operandB, inputDigits[0], inputDigits[1]);
  }

  u8g2.drawStr(14, 110, equation);

  if (combo > 1) {
    char comboText[16];
    snprintf(comboText, sizeof(comboText), "Combo x%d", combo);
    u8g2.drawStr(38, 124, comboText);
  }
}

// ── Game text ─────────────────────────────────────────────────────
void drawGameText() {
  char text[32];

  switch (state) {
    case WAIT_OPERATION:
      u8g2.setFont(u8g2_font_6x12_tf);
      snprintf(text, sizeof(text), "Escanea a %s", getCurrentBossName());
      u8g2.drawStr(14, 104, text);
      snprintf(text, sizeof(text), "Boss de %c", getOperationSymbol(getCurrentBossOperation()));
      u8g2.drawStr(35, 119, text);
      break;

    case SHOW_OPERATION:
      drawOperationName();
      break;

    case WAIT_INPUT:
    case CHECK_RESULT:
      drawEquation();
      break;

    case WIN:
      u8g2.setFont(u8g2_font_8x13_tf);
      u8g2.drawStr(24, 104, "Buen golpe!");
      u8g2.setFont(u8g2_font_6x12_tf);
      snprintf(text, sizeof(text), "Combo:%d", combo);
      u8g2.drawStr(42, 121, text);
      break;

    case LOSE:
      u8g2.setFont(u8g2_font_8x13_tf);
      u8g2.drawStr(28, 104, "Fallaste!");
      u8g2.setFont(u8g2_font_6x12_tf);
      snprintf(text, sizeof(text), "Era: %d", lastCorrectAnswer);
      u8g2.drawStr(42, 121, text);
      break;

    case BOSS_DEFEATED:
      u8g2.setFont(u8g2_font_8x13_tf);
      if (allBossesCleared) {
        u8g2.drawStr(18, 104, "Ganaste todo!");
      } else {
        u8g2.drawStr(14, 104, "Boss vencido!");
      }
      u8g2.setFont(u8g2_font_6x12_tf);
      snprintf(text, sizeof(text), "Score: %d", score);
      u8g2.drawStr(35, 121, text);
      break;

    case GAME_OVER:
      u8g2.setFont(u8g2_font_8x13_tf);
      u8g2.drawStr(28, 104, "Game Over");
      u8g2.setFont(u8g2_font_6x12_tf);
      snprintf(text, sizeof(text), "Era: %d", lastCorrectAnswer);
      u8g2.drawStr(42, 121, text);
      break;
  }
}

// ── Render ────────────────────────────────────────────────────────
void renderCurrentScreen() {
  u8g2.firstPage();
  do {
    drawHud();
    drawEyes();
    drawMouth();
    drawGameText();
  } while (u8g2.nextPage());
}

// ── Blink animation ───────────────────────────────────────────────
void updateAnimations() {
  static unsigned long lastBlink = 0;
  static bool blinking = false;

  unsigned long now = millis();

  if (!blinking && now - lastBlink > 3000) {
    currentEyes = EYES_BLINK;
    blinking = true;
    lastBlink = now;
    screenDirty = true;
  } else if (blinking && now - lastBlink > 120) {
    if (state == WAIT_INPUT || state == SHOW_OPERATION) {
      switch (currentOperation) {
        case ADDITION:       currentEyes = EYES_PLUS; break;
        case SUBTRACTION:    currentEyes = EYES_MINUS; break;
        case MULTIPLICATION: currentEyes = EYES_MULTIPLY; break;
        default:             currentEyes = EYES_NORMAL; break;
      }
    } else if (state == WAIT_OPERATION) {
      switch (getCurrentBossOperation()) {
        case ADDITION:       currentEyes = EYES_PLUS; break;
        case SUBTRACTION:    currentEyes = EYES_MINUS; break;
        case MULTIPLICATION: currentEyes = EYES_MULTIPLY; break;
        default:             currentEyes = EYES_NORMAL; break;
      }
    } else {
      currentEyes = EYES_NORMAL;
    }

    blinking = false;
    lastBlink = now;
    screenDirty = true;
  }
}
