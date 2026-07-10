#include "DisplayUI.h"
#include "Config.h"
#include "GameLogic.h"

bool screenDirty = true;

FaceExpression currentExpression = FACE_IDLE;
EyeMode currentEyes = EYES_NORMAL;

// ── Small drawing helpers ─────────────────────────────────────────
void drawCenteredText(int baselineY, const char* text) {
  int width = u8g2.getStrWidth(text);
  int x = (SCREEN_WIDTH - width) / 2;
  if (x < 0) x = 0;
  u8g2.drawStr(x, baselineY, text);
}

void drawThickLine(int x1, int y1, int x2, int y2, byte thickness = 2) {
  for (byte i = 0; i < thickness; i++) {
    u8g2.drawLine(x1, y1 + i, x2, y2 + i);
  }
}

// ── Large boss eyes ───────────────────────────────────────────────
// The screen only draws the boss face itself: eyes and mouth.
// No robot head and no external RoboEyes library are required.

void drawLargeBaseEye(int x, int y, int width = 36, int height = 38) {
  u8g2.drawRBox(x, y, width, height, 8);
}

void drawIdleEyes() {
  drawLargeBaseEye(14, 26);
  drawLargeBaseEye(78, 26);
}

void drawBlinkEyes() {
  u8g2.drawRBox(14, 44, 36, 4, 2);
  u8g2.drawRBox(78, 44, 36, 4, 2);
}

// Angry combat eyes: the upper edges lean down toward the center.
void drawEvilEyes() {
  drawLargeBaseEye(14, 25, 40, 40);
  drawLargeBaseEye(74, 25, 40, 40);

  u8g2.setDrawColor(0);
  u8g2.drawTriangle(14, 25, 54, 25, 54, 45);
  u8g2.drawTriangle(74, 25, 114, 25, 74, 45);
  u8g2.setDrawColor(1);
}

// Hurt eyes: same boss silhouette, but with visible damage cracks.
void drawHurtEyes() {
  drawEvilEyes();

  u8g2.setDrawColor(0);

  // Left eye crack
  drawThickLine(42, 43, 34, 49, 2);
  drawThickLine(34, 49, 39, 53, 2);
  drawThickLine(34, 49, 29, 56, 2);

  // Right eye crack
  drawThickLine(86, 43, 94, 49, 2);
  drawThickLine(94, 49, 89, 54, 2);
  drawThickLine(94, 49, 99, 57, 2);

  u8g2.setDrawColor(1);
}

// Taunting face shown when the player answers incorrectly.
void drawTauntEyes() {
  u8g2.drawRBox(14, 31, 40, 30, 8);
  u8g2.drawRBox(74, 31, 40, 30, 8);

  u8g2.setDrawColor(0);
  u8g2.drawTriangle(14, 31, 54, 31, 54, 43);
  u8g2.drawTriangle(74, 31, 114, 31, 74, 43);

  // Narrow the lower edge slightly so the eyes feel smug/dominant.
  u8g2.drawTriangle(14, 61, 54, 61, 14, 54);
  u8g2.drawTriangle(74, 61, 114, 61, 114, 54);
  u8g2.setDrawColor(1);
}

void drawDefeatedEyes() {
  for (int8_t offset = -1; offset <= 1; offset++) {
    u8g2.drawLine(16, 29 + offset, 52, 62 + offset);
    u8g2.drawLine(16, 62 + offset, 52, 29 + offset);
    u8g2.drawLine(76, 29 + offset, 112, 62 + offset);
    u8g2.drawLine(76, 62 + offset, 112, 29 + offset);
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

    case FACE_ANGRY:
      drawTauntEyes();
      break;

    case FACE_SAD:
      drawHurtEyes();
      break;

    case FACE_WIN:
      drawDefeatedEyes();
      break;

    case FACE_IDLE:
    default:
      drawIdleEyes();
      break;
  }
}

// ── Boss mouths ───────────────────────────────────────────────────
void drawNeutralMouth() {
  u8g2.drawRBox(53, 75, 22, 4, 2);
}

void drawCombatMouth() {
  const int xs[] = {38, 46, 54, 62, 70, 78, 86, 94};
  const int ys[] = {76, 70, 78, 70, 78, 70, 78, 76};

  for (byte i = 0; i < 7; i++) {
    drawThickLine(xs[i], ys[i], xs[i + 1], ys[i + 1], 2);
  }
}

void drawHurtMouth() {
  const int xs[] = {39, 47, 55, 63, 71, 79, 87, 95};
  const int ys[] = {75, 80, 73, 81, 73, 81, 74, 79};

  for (byte i = 0; i < 7; i++) {
    drawThickLine(xs[i], ys[i], xs[i + 1], ys[i + 1], 2);
  }
}

void drawTauntMouth() {
  // Wide toothy grin.
  u8g2.drawRBox(38, 70, 52, 17, 5);

  u8g2.setDrawColor(0);
  u8g2.drawRBox(41, 69, 46, 7, 3);

  for (int x = 49; x <= 81; x += 11) {
    u8g2.drawBox(x, 76, 3, 10);
  }
  u8g2.setDrawColor(1);
}

void drawDefeatedMouth() {
  drawThickLine(49, 82, 64, 73, 2);
  drawThickLine(64, 73, 79, 82, 2);
}

void drawMouth() {
  switch (currentExpression) {
    case FACE_EVIL:
      drawCombatMouth();
      break;

    case FACE_ANGRY:
      drawTauntMouth();
      break;

    case FACE_SAD:
      drawHurtMouth();
      break;

    case FACE_WIN:
      drawDefeatedMouth();
      break;

    case FACE_IDLE:
    default:
      drawNeutralMouth();
      break;
  }
}

// ── Persistent HUD ────────────────────────────────────────────────
void drawHud() {
  char bossText[18];
  char stageText[8];
  char hpText[8];
  char livesText[12];
  char scoreText[12];

  u8g2.setDrawColor(1);
  u8g2.setFontMode(1);

  // Exactamente la misma fuente usada en "Jefe de +".
  u8g2.setFont(u8g2_font_5x8_tf);

  // Primera fila: jefe y progreso.
  snprintf(
    bossText,
    sizeof(bossText),
    "%s %c",
    getCurrentBossName(),
    getOperationSymbol(getCurrentBossOperation())
  );

  u8g2.drawStr(2, 10, bossText);

  snprintf(
    stageText,
    sizeof(stageText),
    "%d/%d",
    currentBossIndex + 1,
    getBossCount()
  );

  int stageX =
    SCREEN_WIDTH - u8g2.getStrWidth(stageText) - 2;

  u8g2.drawStr(stageX, 10, stageText);

  // Segunda fila: cada estadística se dibuja por separado.
  snprintf(hpText, sizeof(hpText), "HP:%d", bossHP);
  u8g2.drawStr(2, 21, hpText);

  snprintf(
    livesText,
    sizeof(livesText),
    "VIDAS:%d",
    playerLives
  );

  int livesX =
    (SCREEN_WIDTH - u8g2.getStrWidth(livesText)) / 2;

  u8g2.drawStr(livesX, 21, livesText);

  snprintf(scoreText, sizeof(scoreText), "PTS:%d", score);

  int scoreX =
    SCREEN_WIDTH - u8g2.getStrWidth(scoreText) - 2;

  u8g2.drawStr(scoreX, 21, scoreText);
}

void drawOperationName() {
  char line[32];

  u8g2.setFont(u8g2_font_8x13_tf);
  snprintf(line, sizeof(line), "%s!", getCurrentBossName());
  drawCenteredText(104, line);

  u8g2.setFont(u8g2_font_6x12_tf);
  snprintf(line, sizeof(line), "Operacion: %c", getOperationSymbol(currentOperation));
  drawCenteredText(121, line);
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

  drawCenteredText(108, equation);

  u8g2.setFont(u8g2_font_5x8_tf);
  if (combo > 1) {
    char comboText[20];
    snprintf(comboText, sizeof(comboText), "Combo x%d", combo);
    drawCenteredText(124, comboText);
  } else {
    drawCenteredText(124, "Escanea y confirma");
  }
}

// ── State-specific text and feedback ──────────────────────────────
void drawGameText() {
  char text[32];

  switch (state) {
    case WAIT_OPERATION:
      u8g2.setFont(u8g2_font_6x12_tf);
      snprintf(text, sizeof(text), "Escanea a %s", getCurrentBossName());
      drawCenteredText(104, text);

      u8g2.setFont(u8g2_font_5x8_tf);
      snprintf(text, sizeof(text), "Jefe de %c", getOperationSymbol(getCurrentBossOperation()));
      drawCenteredText(121, text);
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
      drawCenteredText(102, "CORRECTO!");

      u8g2.setFont(u8g2_font_5x8_tf);
      drawCenteredText(115, "-1 vida al jefe");

      snprintf(text, sizeof(text), "HP:%d  Combo:%d", bossHP, combo);
      drawCenteredText(125, text);
      break;

    case LOSE:
      u8g2.setFont(u8g2_font_8x13_tf);
      drawCenteredText(101, "INCORRECTO!");

      u8g2.setFont(u8g2_font_5x8_tf);
      snprintf(text, sizeof(text), "Era: %d", lastCorrectAnswer);
      drawCenteredText(114, text);
      drawCenteredText(125, "Pierdes 1 vida");
      break;

    case BOSS_DEFEATED:
      u8g2.setFont(u8g2_font_8x13_tf);
      if (allBossesCleared) {
        drawCenteredText(103, "GANASTE TODO!");
      } else {
        drawCenteredText(103, "JEFE VENCIDO!");
      }

      u8g2.setFont(u8g2_font_5x8_tf);
      snprintf(text, sizeof(text), "Score: %d", score);
      drawCenteredText(121, text);
      break;

    case GAME_OVER:
      u8g2.setFont(u8g2_font_8x13_tf);
      drawCenteredText(101, "GAME OVER");

      u8g2.setFont(u8g2_font_5x8_tf);
      snprintf(text, sizeof(text), "Era: %d", lastCorrectAnswer);
      drawCenteredText(114, text);
      snprintf(text, sizeof(text), "Score: %d", score);
      drawCenteredText(125, text);
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

// ── Idle-only blink animation ─────────────────────────────────────
void updateAnimations() {
  static unsigned long lastBlinkEnd = 0;
  static unsigned long blinkStartedAt = 0;
  static bool blinking = false;

  const unsigned long blinkInterval = 3000;
  const unsigned long blinkDuration = 120;

  unsigned long now = millis();

  // Blink only while waiting for the current boss/operator card.
  // Angry, damaged, taunting and defeated faces must stay visible.
  bool canBlink =
    state == WAIT_OPERATION &&
    currentExpression == FACE_IDLE;

  if (!canBlink) {
    if (blinking || currentEyes == EYES_BLINK) {
      blinking = false;
      currentEyes = EYES_NORMAL;
      screenDirty = true;
    }

    lastBlinkEnd = now;
    return;
  }

  if (!blinking && now - lastBlinkEnd >= blinkInterval) {
    blinking = true;
    blinkStartedAt = now;
    currentEyes = EYES_BLINK;
    screenDirty = true;
    return;
  }

  if (blinking && now - blinkStartedAt >= blinkDuration) {
    blinking = false;
    currentEyes = EYES_NORMAL;
    lastBlinkEnd = now;
    screenDirty = true;
  }
}
