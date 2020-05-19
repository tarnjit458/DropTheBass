#pragma once

#include "L0_LowLevel/LPC40xx.h"
#include "ngpincon.hpp"
#include "utility/log.hpp"

class SSP{
public:
  enum FrameModes
  {
    kSPI = 0b00,
    kTI = 0b01,
    kMW = 0b10
  };

  //Constructor
  //@param data_size_select: The amount of bits to transfer at a time
  //@param format: The format for the SSP frams (SPI, TI, MW)
  //@param divide: The amount to divide CPCSR by
  //@param ssp_num: The SSP device number to initialize (0, 1, 2)
  SSP(uint8_t data_size_select, FrameModes format, uint8_t divide, uint8_t ssp_num);

  //Initialize the SSP object
  void Init();

  //Transfer
  uint16_t Transfer(uint16_t send);

  //Send data to the SSP device, 16 bit aligned
  void Send(uint16_t *buf, uint32_t len=1);

  //Send data to the SSP device, 16 bit aligned
  void Send(uint16_t data);

  //Read data from the SPI device, 16 bit aligned
  void Recv(uint16_t *buf, uint32_t len=1);

  //Send data to the SPI device, 8 bit aligned
  void Send(uint8_t *buf, uint32_t len=1);

  //Read data from the SPI device, 8 bit aligned
  void Recv(uint8_t *buf, uint32_t len=1);

  //Get the status register for the associated SSP device
  uint8_t GetStatus();

  //Check if busy bit is set
  bool IsBusy();

  //Wait for SSP to become available
  void BusyWait();

  //Get the port for the SSP device
  uint8_t GetPort();

  //Check to see if the Recieve FIFO is empty
  bool RFIFOIsEmpty();

  //Check to see if the transmit FIFO is empty
  bool TFIFOIsEmpty();

  //Wait for data in the Recieve FIFO
  void BusyRFIFOWait();

  //Wait for the transmit FIFO to be empty
  void BusyTFIFOWait();

private:
  uint8_t _dss;
  FrameModes _format;
  uint8_t _div;
  uint8_t _ssp_num;
};
