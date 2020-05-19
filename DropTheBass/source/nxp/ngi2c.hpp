#pragma once

#include "L0_LowLevel/LPC40xx.h"
#include "ngpincon.hpp"
#include "third_party/FreeRTOS/Source/include/FreeRTOS.h"
#include "L0_LowLevel/interrupt.hpp"
#include "utility/log.hpp"

#include <iterator>

class I2C{
public:

  //Control settings for the I2CON registers
  enum Ctrl : uint32_t{
    kAsrtAck  = (1 << 2),  // AA
    kInt      = (1 << 3),  // SI
    kStop     = (1 << 4),  // STO
    kStart    = (1 << 5),  // STA
    kEn       = (1 << 6)   // I2EN
  };

  //Slave reciever states
  enum SRStates : uint8_t{
    kGotSLAW          = 0x60,  //Got SLA+W, returned ACK
    kSLAWArbLost      = 0x68,  //Lost arbitration in SLA+W, returned ACK
    kGotGenC          = 0x70,  //Got General Call Address, returned ACK
    kGenCArbLost      = 0x78,  //Lost arbitration in Gen Call, returned ACK
    kSLAWGotData      = 0x80,  //Addressed as SLA+W previously, got data, returned ACK
    kSLAWNoData       = 0x88,  //Addressed as SLA+W previously, didn't get any data, returned NACK
    kGenCGotData      = 0x90,  //Addressed as Gen Call previously, got data, returned ACK
    kGenCNoData       = 0x98,  //Addressed as Gen Call previously, didn't get any data, returned NACK
    kGotSSr           = 0xA0   //Got a STOP or repeated START
  };

  //Slave Transmitter States
  enum STStates : uint8_t{
    kGotSLAR          = 0xA8, //Got SLA+R, returned ACK
    kSLARArbLost      = 0xB0, //Lost arbitration in SLA+R/W. returned ACK
    kSentDataACK      = 0xB8, //Sent data in I2CDAT, got back an ACK
    kSentDataNACK     = 0xC0, //Sent data in I2CDAT, got back a NACK
    kSentLastByte     = 0xC8, //Sent the last byte in I2DAT, got back an ACK
  };

  I2C(uint8_t address);
  void InitI2CSlave();

private:
  static void I2CStateHandler();
  static uint8_t i2c_memory[256];
  uint8_t _addr;
};
