#pragma once
#include <SparkFun_ST25DV64KC_Arduino_Library.h> // Click here to get the library:  http://librarymanager/All#SparkFun_ST25DV64KC


void EnableFTM(SFE_ST25DV64KC_NDEF tag);
bool isMailboxConsumed(SFE_ST25DV64KC_NDEF tag);
bool writeToMailbox(SFE_ST25DV64KC_NDEF tag, uint8_t *mail, int mailLength);
uint8_t readMailbox(SFE_ST25DV64KC_NDEF tag, uint8_t *mail);
bool receivedMail(SFE_ST25DV64KC_NDEF tag);