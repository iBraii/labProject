#pragma once

#include "Types.h"

extern FaceExpression currentExpression;
extern EyeMode currentEyes;

extern bool screenDirty;

void renderCurrentScreen();

void updateAnimations();