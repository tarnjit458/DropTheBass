#include "ngi2c.hpp"

//Initialize the I2C Memory
uint8_t I2C::i2c_memory[256] = {0};

I2C::I2C(uint8_t address){
  if(address > 0b01111111){
    LOG_INFO("WARNING: I2C slave address is longer than 7 bits");
  }
  _addr = address;
}

void I2C::InitI2CSlave(){
	//Note: On reset, all i2c interfaces are enabled. Just to be safe, re-enable them
	//We're using I2C2 so set PCI2C2, bit 26
	LPC_SC->PCONP |= (1 << 26);

	//Set the IOCON registers on P0_10 and P0_11 to use I2C2
	PinconSetFunc(0, 10, 0b010);
	PinconSetFunc(0, 11, 0b010);

  //Turn off the pullup/pulldowns
  PinconPulloff(0, 10);
  PinconPulloff(0, 11);

  //Turn on open drain
  PinconOpendrain(0, 10, true);
  PinconOpendrain(0, 11, true);

	//Setting up slave mode
	//Bits must be set as follows
	//STA 	= 0
	LPC_I2C2->CONCLR = (Ctrl::kStart);

	//SI 		= 0
	LPC_I2C2->CONCLR = (Ctrl::kInt);

	//AA	  = 1
	LPC_I2C2->CONSET = (Ctrl::kAsrtAck);

  //I2EN 	= 1
	LPC_I2C2->CONSET = (Ctrl::kEn);

	//Set our slave address and general call address
  //Turn Gen Call off
	LPC_I2C2->ADR0 = (_addr << 1) | 0b0;
  LPC_I2C2->ADR1 = (_addr << 1) | 0b0;
  LPC_I2C2->ADR2 = (_addr << 1) | 0b0;
  LPC_I2C2->ADR3 = (_addr << 1) | 0b0;

  LPC_I2C2->MASK0 = 0x00;
  LPC_I2C2->MASK1 = 0x00;
  LPC_I2C2->MASK2 = 0x00;
  LPC_I2C2->MASK3 = 0x00;

  RegisterIsr(I2C2_IRQn, I2CStateHandler);
  NVIC_EnableIRQ(I2C2_IRQn);
  LOG_INFO("STAT:0x%x", LPC_I2C2->STAT & 0x11111111);
  LOG_INFO("ADR0: 0x%x",LPC_I2C2->ADR0 >> 1);
  if(LPC_I2C2->ADR0 & 0x01){
    LOG_INFO("General Call Enabled");
  }
  else{
    LOG_INFO("General Call Disabled");
  }
  LOG_INFO("MASK0: 0x%x",LPC_I2C2->MASK0 >> 1);
  LOG_INFO("CON: 0b%b",LPC_I2C2->CONSET);
}

void I2C::I2CStateHandler(){
  //The number of bytes we have recieved so far
  static uint8_t data_index = 0;
  //The working register
  static uint8_t reg = 0;
  //The status register
  uint8_t i2cstat = LPC_I2C2->STAT;
  LPC_I2C2->CONCLR = (Ctrl::kInt);
  switch(i2cstat){
    //Addressed on SLA+W
    case(SRStates::kGotSLAW) :{
      //Reset the data index back to 0
      data_index = 0;
      //Send back an ACK
      LPC_I2C2->CONSET = (Ctrl::kAsrtAck);
      break;
    }
    //Got data over SLA+W
    case(SRStates::kSLAWGotData) :{
      //Send back an ACK
      LPC_I2C2->CONSET = (Ctrl::kAsrtAck);
      if(data_index == 0){
        //I2C always sends the register first, so save first recieved byte as
        //the working register
        reg = LPC_I2C2->DAT;
      }
      else{
        //If this isn't the first data byte, we're doing a write
        //Use the first byte sent as the register, write this current byte to it
        //Use data index as an offest for multi byte writes
        I2C::i2c_memory[(reg + (data_index - 1))] = LPC_I2C2->DAT;
      }
      //Increment the data index
      data_index++;
      break;
    }
    //Got a Stop or Repeated Start
    case(SRStates::kGotSSr) :{
      break;
    }
    //Master is asking for a byte
    case(STStates::kGotSLAR) :{
      //Send the byte from the previously requested register
      LPC_I2C2->DAT = I2C::i2c_memory[reg];
      //Increment the data index
      data_index++;
      break;
    }
    //The master got the byte we transmitted, wants more
    case(STStates::kSentDataACK) :{
      //Send the next byte offset with the data index
      LPC_I2C2->DAT = I2C::i2c_memory[reg + (data_index - 1)];
      //Increment the data index
      data_index++;
      break;
    }
    //Master got our data, told us not to send any more
    case(STStates::kSentDataNACK) :{
      //Make sure ACKs stay on so the bus keeps working
      LPC_I2C2->CONSET = (Ctrl::kAsrtAck);
      break;
    }
    //We've send out last byte and the master got it
    case(STStates::kSentLastByte) :{
      //Make sure ACKs stay on so the bus keeps working
      LPC_I2C2->CONSET = (Ctrl::kAsrtAck);
      break;
    }
  }
}
