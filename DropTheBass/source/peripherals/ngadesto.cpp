#include "ngadesto.hpp"

#include "L0_LowLevel/LPC40xx.h"
#include "../nxp/nggpio.hpp"
#include "utility/log.hpp"

#define ADESTO_SIG_CMMD       0x9F
#define ADESTO_WRITE_CMMD     0x02
#define ADESTO_WRITE_EN_CMMD  0x06
#define ADESTO_READ_CMMD      0x03
#define ADESTO_STAT_CMMD      0x05

#define ADESTO_PORT     1
#define ADESTO_SCK_PIN  0
#define ADESTO_MOSI_PIN 1
#define ADESTO_MISO_PIN 4
#define ADESTO_CS_PIN   10

Adesto::Adesto(){
  _cs = new GPIO(ADESTO_PORT, ADESTO_CS_PIN);
  _ssp2 = new SSP(8, SSP::kSPI, 0b10, 2);
  _cs->SetAsOutput();
  _disable();
}

void Adesto::_enable(){
  _cs->SetLow();
}

void Adesto::_disable(){
  _cs->SetHigh();
}

void Adesto::_set_state(bool state){
  if(state){
    _enable();
  }
  else{
    _disable();
  }
}

uint32_t Adesto::GetSignature(){
  uint8_t data[4];
  uint8_t cmmd = ADESTO_SIG_CMMD;
  _enable();
  _ssp2->Send(&cmmd);
  _ssp2->Recv(data, 4);
  _disable();
  return data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
}

uint8_t Adesto::GetStatReg(){
  uint8_t data = ADESTO_STAT_CMMD;
  _enable();
  _ssp2->Send(&data);
  _ssp2->Recv(&data);
  _disable();
  return (uint8_t)data;
}

void Adesto::WriteEnable(){
  uint8_t data = ADESTO_WRITE_EN_CMMD;
  _enable();
  _ssp2->Send(&data);
  _disable();
}

bool Adesto::CheckBusy(){
  return GetStatReg() & (1 << 0);
}

void Adesto::BusyWait(){
  while(CheckBusy()){
    continue;
  }
}

void Adesto::Write(uint8_t *buf, uint32_t addr, uint16_t len){
  uint8_t cmmd[4];
  cmmd[0] = ADESTO_WRITE_CMMD;
  cmmd[1] = addr & 0x00FF0000;
  cmmd[2] = addr & 0x0000FF00;
  cmmd[3] = addr & 0x000000FF;
  BusyWait();
  _enable();
  _ssp2->Send(cmmd, 4);
  _ssp2->Send(buf, len);
  _disable();
}

void Adesto::Read(uint8_t *buf, uint32_t addr, uint16_t len){
  uint8_t cmmd[4];
  cmmd[0] = ADESTO_READ_CMMD;
  cmmd[1] = addr & 0x00FF0000;
  cmmd[2] = addr & 0x0000FF00;
  cmmd[3] = addr & 0x000000FF;
  BusyWait();
  _enable();
  _ssp2->Send(cmmd, 4);
  _ssp2->Recv(buf, len);
  _disable();
}

Adesto::~Adesto(){
  delete _ssp2;
  delete _cs;
}
