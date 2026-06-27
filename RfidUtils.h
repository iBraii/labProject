#pragma once

#include "Config.h"
#include "Types.h"

bool readCard();
bool matchUID(MFRC522::Uid *uid, RFIDEntity &entity);

OperationType getOperation(MFRC522::Uid *uid);
int getCardValue(MFRC522::Uid *uid);

extern Operation operations[];
extern Card cards[];