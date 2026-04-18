#include "mailboxHandling.h"

//using configs for KC version
#define ST25KC

void EnableFTM(SFE_ST25DV64KC_NDEF tag)
{
  uint8_t password[8] = {0x0};
  tag.openI2CSession(password);
  
  #ifdef ST25KC
  //enable GPO interrupt
  tag.st25_io.writeSingleByte(SF_ST25DV64KC_ADDRESS::SYSTEM, REG_GPO1, 0xB0);
  //configure IT pulse
  tag.st25_io.writeSingleByte(SF_ST25DV64KC_ADDRESS::SYSTEM, REG_GPO2, 0x0C);
  //enable FTM
  tag.st25_io.writeSingleByte(SF_ST25DV64KC_ADDRESS::SYSTEM, REG_FTM, 0x0F);
  #else // config for st24dvxxK
  //enable GPO interrupt
  tag.st25_io.writeSingleByte(SF_ST25DV64KC_ADDRESS::SYSTEM, REG_GPO1, 0xB0);
  //staget IT_TIME
  tag.st25_io.writeSingleByte(SF_ST25DV64KC_ADDRESS::SYSTEM, REG_GPO2, 0x07);
  //set watchdog
  tag.st25_io.writeSingleByte(SF_ST25DV64KC_ADDRESS::SYSTEM, 0x0E, 0x07);
  //enable MB_MODE
  tag.st25_io.writeSingleByte(SF_ST25DV64KC_ADDRESS::SYSTEM, REG_FTM, 0x01);
  #endif

  tag.st25_io.setRegisterBit(SF_ST25DV64KC_ADDRESS::DATA, REG_MB_CTRL_DYN, BIT_MB_CTRL_DYN_MB_EN);
}


//check if GPO trigger is due to new mail
bool receivedMail(SFE_ST25DV64KC_NDEF tag)
{ return tag.st25_io.isBitSet(SF_ST25DV64KC_ADDRESS::DATA, REG_IT_STS_DYN, BIT_IT_STS_DYN_RF_PUT_MSG); }



uint8_t readMailbox(SFE_ST25DV64KC_NDEF tag, uint8_t *mail)
{    
  if (tag.st25_io.isBitSet(SF_ST25DV64KC_ADDRESS::DATA, REG_MB_CTRL_DYN, BIT_MB_CTRL_DYN_RF_PUT_MSG) &&
      tag.st25_io.isBitSet(SF_ST25DV64KC_ADDRESS::DATA, REG_MB_CTRL_DYN, BIT_MB_CTRL_DYN_RF_CURRENT_MSG))
  {    
    uint8_t msgLength;
    tag.st25_io.readSingleByte(SF_ST25DV64KC_ADDRESS::DATA, REG_MB_LEN_DYN, &msgLength);
    tag.st25_io.readMultipleBytes(SF_ST25DV64KC_ADDRESS::DATA, MAILBOX_BASE, mail, msgLength+1);
    return msgLength+1;
  }
  return 0;
}

bool writeToMailbox(SFE_ST25DV64KC_NDEF tag, uint8_t *mail, int mailLength)
{
  if (mailLength > 256)
  { return false; }

  tag.st25_io.writeMultipleBytes(SF_ST25DV64KC_ADDRESS::DATA, MAILBOX_BASE, mail, mailLength);
  return true;
}

bool isMailboxConsumed(SFE_ST25DV64KC_NDEF tag)
{ return tag.st25_io.isBitSet(SF_ST25DV64KC_ADDRESS::DATA, REG_IT_STS_DYN, BIT_IT_STS_DYN_RF_GET_MSG); }