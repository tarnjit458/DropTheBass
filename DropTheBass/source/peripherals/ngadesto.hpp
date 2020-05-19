#pragma once

#include "../nxp/nggpio.hpp"
#include "../nxp/ngssp.hpp"

#include <cstdint>
#include <iterator>

class Adesto{
public:
  //Constructor
  Adesto();
  //Destructor
  ~Adesto();
  //Enable writing to the Adesto
  void WriteEnable();
  //Disable writing to the Adesto
  void WriteDisable();
  //Set the write status of the Adesto
  void SetWrite(bool state);
  //Write data to the Adesto chip
  void Write(uint8_t *buf, uint32_t addr, uint16_t len=1);
  //Read data from the adesto chip
  void Read(uint8_t *buf, uint32_t addr, uint16_t len=1);
  //Get the status register of the Adesto chip
  uint8_t GetStatReg();
  //Get the Manufacturer/Device Signature of the Adesto
  uint32_t GetSignature();
  //Check to see if the Adesto is busy
  bool CheckBusy();
  //Busy wait for the Adesto to become available
  void BusyWait();
private:
  SSP *_ssp2;
  GPIO *_cs;
  //Enable the Adesto
  void _enable();
  //Disable the Adesto
  void _disable();
  //Enable or disable the adesto
  void _set_state(bool state);
};
