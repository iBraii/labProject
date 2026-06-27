#include "DisplayUI.h"
#include "Config.h"
#include "GameLogic.h"

bool screenDirty = true;

// ── Default expression ────────────────────────────────────────────
// Change FACE_EVIL to any FaceExpression to set a different default.
// Available: FACE_IDLE, FACE_EVIL, FACE_ANGRY, FACE_SAD
FaceExpression currentExpression = FACE_EVIL;
EyeMode        currentEyes       = EYES_NORMAL;

// ── Eye shapes ────────────────────────────────────────────────────

void drawBaseEye(int x, int y) {
  u8g2.drawRBox(x, y, 28, 20, 6);
}

void drawIdleEyes() {
  drawBaseEye(22, 28);
  drawBaseEye(78, 28);
}

void drawBlinkEyes() {
  u8g2.drawBox(22, 38, 28, 2);
  u8g2.drawBox(78, 38, 28, 2);
}

void drawEvilEyes() {
  drawBaseEye(22, 28);
  drawBaseEye(78, 28);

  u8g2.setDrawColor(0);

  // Left: cut top-right (inner) corner → \ slash
  u8g2.drawTriangle(22, 28,  50, 28,  50, 48);

  // Right: cut top-left (inner) corner → / slash
  u8g2.drawTriangle(78, 28,  106, 28,  78, 48);

  u8g2.setDrawColor(1);
}

void drawSadEyes() {
  drawBaseEye(22, 28);
  drawBaseEye(78, 28);

  u8g2.setDrawColor(0);

  // Left: cut top-left corner → drooping outer edge
  u8g2.drawTriangle(22, 28,  50, 28,  22, 48);

  // Right: cut top-right corner
  u8g2.drawTriangle(78, 28,  106, 28,  106, 48);

  u8g2.setDrawColor(1);
}

void drawHappyEyes() {
  drawBaseEye(22, 28);
  drawBaseEye(78, 28);

  // Eyelids — squinting, content look
  u8g2.setDrawColor(0);
  u8g2.drawBox(22, 28, 28, 10);
  u8g2.drawBox(78, 28, 28, 10);
  u8g2.setDrawColor(1);
}

// X eyes: robot is defeated (player won)
void drawXEyes() {
  // Left eye X — region (22,28) to (50,48)
  for (int8_t t = -1; t <= 1; t++) {
    u8g2.drawLine(22, 28 + t,  50, 48 + t);   // top-left → bottom-right
    u8g2.drawLine(22, 48 + t,  50, 28 + t);   // bottom-left → top-right
  }
  // Right eye X — region (78,28) to (106,48)
  for (int8_t t = -1; t <= 1; t++) {
    u8g2.drawLine(78, 28 + t,  106, 48 + t);
    u8g2.drawLine(78, 48 + t,  106, 28 + t);
  }
}

// Operator symbol eyes (only active when expression is FACE_IDLE)
void drawOperatorEyes() {
  u8g2.setFont(u8g2_font_logisoso24_tf);
  switch (currentEyes) {
    case EYES_PLUS:
      u8g2.drawStr(28, 48, "+");
      u8g2.drawStr(84, 48, "+");
      break;
    case EYES_MINUS:
      u8g2.drawStr(30, 48, "-");
      u8g2.drawStr(86, 48, "-");
      break;
    case EYES_MULTIPLY:
      u8g2.drawStr(28, 48, "x");
      u8g2.drawStr(84, 48, "x");
      break;
    default:
      drawIdleEyes();
      break;
  }
}

// ── Master eye dispatcher ─────────────────────────────────────────
// Blink is checked first so it overrides any expression.
// To disable blinking: comment out the updateAnimations() call in loop().

void drawEyes() {

  if (currentEyes == EYES_BLINK) {
    drawBlinkEyes();
    return;
  }

  // ── To use a different face for each state, swap case values below ──
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
      drawHappyEyes();   // squinting angry look
      break;

    case FACE_IDLE:
    default:
      // Operator eyes only show in FACE_IDLE mode
      if (currentEyes == EYES_PLUS  ||
          currentEyes == EYES_MINUS ||
          currentEyes == EYES_MULTIPLY) {
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
      u8g2.drawBox(56, 74, 16, 2);         // flat line
      break;

    case FACE_EVIL:
      u8g2.drawLine(54, 78, 74, 72);       // tilted smirk
      break;

    case FACE_ANGRY:
      u8g2.drawBox(54, 74, 20, 3);         // thick flat line
      break;

    case FACE_SAD:
    case FACE_WIN:
      u8g2.drawLine(54, 72, 74, 78);       // drooping line (robot defeated)
      break;
  }
}

// ── Game text ─────────────────────────────────────────────────────

void drawGameText() {

  char equation[32];
  char op = '?';

  switch (currentOperation) {
    case ADDITION:       op = '+'; break;
    case SUBTRACTION:    op = '-'; break;
    case MULTIPLICATION: op = 'x'; break;
    default: break;
  }

  switch (state) {

    case WAIT_OPERATION:
      u8g2.setFont(u8g2_font_6x12_tf);
      u8g2.drawStr(8, 105, "Escanea tu matemon");
      break;

    case SHOW_OPERATION: {
      const char* opName = "???";
      switch (currentOperation) {
        case ADDITION:       opName = "Sumi!!!";    break;
        case SUBTRACTION:    opName = "Minus!!!"; break;
        case MULTIPLICATION: opName = "Multiplor!!!";    break;
        default: break;
      }
      u8g2.setFont(u8g2_font_8x13_tf);
      u8g2.drawStr(10, 105, opName);
      break;
    }

    case WAIT_INPUT:
    case CHECK_RESULT:
      u8g2.setFont(u8g2_font_6x12_tf);
      if (inputCount == 0)
        sprintf(equation, "%d %c %d = _",
                operandA, op, operandB);
      else if (inputCount == 1)
        sprintf(equation, "%d %c %d = %d_",
                operandA, op, operandB, inputDigits[0]);
      else
        sprintf(equation, "%d %c %d = %d%d",
                operandA, op, operandB, inputDigits[0], inputDigits[1]);
      u8g2.drawStr(14, 110, equation);
      break;

    case WIN:
      u8g2.setFont(u8g2_font_8x13_tf);
      u8g2.drawStr(32, 105, "Ganaste!!");
      break;

    case LOSE:
      u8g2.setFont(u8g2_font_8x13_tf);
      u8g2.drawStr(18, 105, "Denuevo!!");
      break;
  }
}

// ── Render ────────────────────────────────────────────────────────

void renderCurrentScreen() {
  u8g2.firstPage();
  do {
    drawEyes();
    drawMouth();
    drawGameText();
  } while (u8g2.nextPage());
}

// ── Blink animation ───────────────────────────────────────────────
// To disable blinking (e.g. if it conflicts with RFID timing),
// comment out the updateAnimations() call in Lab2ArduinoProject.ino.

void updateAnimations() {

  static unsigned long lastBlink = 0;
  static bool blinking = false;

  unsigned long now = millis();

  if (!blinking && now - lastBlink > 3000) {
    currentEyes = EYES_BLINK;
    blinking    = true;
    lastBlink   = now;
    screenDirty = true;
  }

  else if (blinking && now - lastBlink > 120) {

    // Restore operator eyes when in an active round
    if (state == WAIT_INPUT || state == SHOW_OPERATION) {
      switch (currentOperation) {
        case ADDITION:       currentEyes = EYES_PLUS;     break;
        case SUBTRACTION:    currentEyes = EYES_MINUS;    break;
        case MULTIPLICATION: currentEyes = EYES_MULTIPLY; break;
        default:             currentEyes = EYES_NORMAL;   break;
      }
    } else {
      currentEyes = EYES_NORMAL;
    }

    blinking    = false;
    lastBlink   = now;
    screenDirty = true;
  }
}
