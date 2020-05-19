#include "ngssp.hpp"

volatile uint32_t* SSP_DR[3]    = {&LPC_SSP0->DR,   &LPC_SSP1->DR,    &LPC_SSP2->DR};
volatile uint32_t* SSP_SR[3]    = {&LPC_SSP0->SR,   &LPC_SSP1->SR,    &LPC_SSP2->SR};
volatile uint32_t* SSP_CR0[3]   = {&LPC_SSP0->CR0,  &LPC_SSP1->CR0,   &LPC_SSP2->CR0};
volatile uint32_t* SSP_CR1[3]   = {&LPC_SSP0->CR1,  &LPC_SSP1->CR1,   &LPC_SSP2->CR1};
volatile uint32_t* SSP_CPSR[3]  = {&LPC_SSP0->CPSR, &LPC_SSP1->CPSR,  &LPC_SSP2->CPSR};

SSP::SSP(uint8_t data_size_select, FrameModes format, uint8_t divide, uint8_t ssp_num){
  _dss = data_size_select - 1;
  _ssp_num = ssp_num;
  _format = format;
  _div = divide;


  LOG_DEBUG("scr = %b", scr);
  LOG_DEBUG("dss = %b", _dss);
  LOG_DEBUG("form = %b", format);
  LOG_DEBUG("div = %b", _div);
  LOG_DEBUG("ssp_num = %b", _ssp_num);
}

void SSP::Init(){
  uint8_t form = 0;
  uint16_t scr = (0b01 << 8);

  switch (_format)
  {
    case kSPI : form = (0b00 << 4);
                break;

    case kTI :  form = (0b01 << 4);
                break;

    case kMW :  form = (0b10 << 4);
                break;
  }
  switch (_ssp_num){
      case 0 : 	LPC_SC->PCONP |= (1 << 21);
                //SSP0_SCK
                PinconSetFunc(0, 15, 0b010);
                //SSP0_MOSI
                PinconSetFunc(0, 18, 0b010);
                //SSP0 MISO
                PinconSetFunc(0, 17, 0b010);
                break;

      case 1 :	LPC_SC->PCONP |= (1 << 10);
                //SSP1_SCK
                PinconSetFunc(1, 19, 0b101);
                //SSP1_MOSI
                PinconSetFunc(1, 22, 0b101);
                //SSP1 MISO
                PinconSetFunc(1, 18, 0b101);
                break;

      case 2 :	LPC_SC->PCONP |= (1 << 20);
                //SSP2_SCK
                PinconSetFunc(1, 0, 0b100);
                //SSP2_MOSI
                PinconSetFunc(1, 1, 0b100);
                //SSP2 MISO
                PinconSetFunc(1, 4, 0b100);
                break;
  }

  *SSP_CPSR[GetPort()] = _div;
  //*SSP_CPSR[GetPort()] = 0b10;
  *SSP_CR0[GetPort()] |= _dss;
  *SSP_CR0[GetPort()] |= form;
  *SSP_CR0[GetPort()] |= scr;

  *SSP_CR1[GetPort()] = 0b10;

  LOG_DEBUG("SSP%x CPSR = %b", GetPort(), *SSP_CPSR[GetPort()]);
  LOG_DEBUG("SSP%x CR0 = %b", GetPort(), *SSP_CR0[GetPort()]);
  LOG_DEBUG("SSP%x CR1 = %b", GetPort(), *SSP_CR1[GetPort()]);
}


void SSP::Send(uint16_t *buf, uint32_t len){
  //I'm the TRASHMAN. I throw TRASH all over the ring! And then, I start eatin GARBAGE
  uint16_t trashman = 0;
  //Write data from the device to the buffer for the length specified
  for(uint32_t i = 0; i < len; i++){
    //Make sure SSP isn't busy and can send data
    BusyWait();
    //Send data to the device
    *SSP_DR[GetPort()] = buf[i];
    //Busy wait so we don't miss trashing the data
    BusyWait();
    //Trash garbage data from the SSP FIFO
    trashman = *SSP_DR[GetPort()];
  }
}

void SSP::Send(uint16_t data){
  //I'm the TRASHMAN. I throw TRASH all over the ring! And then, I start eatin GARBAGE
  uint16_t trashman = 0;
  //Write data from the device to the buffer for the length specified
  //Make sure SSP isn't busy and can send data
  BusyWait();
  //Send data to the device
  *SSP_DR[GetPort()] = data;
  //Busy wait so we don't miss trashing the data
  BusyWait();
  //Trash garbage data from the SSP FIFO
  trashman = *SSP_DR[GetPort()];
}

void SSP::Send(uint8_t *buf, uint32_t len){
  //I'm the TRASHMAN. I throw TRASH all over the ring! And then, I start eatin GARBAGE
  uint16_t trashman = 0;
  //Write data from the device to the buffer for the length specified
  for(uint32_t i = 0; i < len; i++){
    //Make sure SSP isn't busy and can send data
    BusyWait();
    //Send data to the device
    *SSP_DR[GetPort()] = buf[i];
    //Trash garbage data from the SSP FIFO
    trashman = *SSP_DR[GetPort()];
  }
}

void SSP::Recv(uint16_t *buf, uint32_t len){
  //Read data back from the device, save data to buffer
  for(uint32_t i = 0; i < len; i++){
    //Make sure the device is ready to send data
    BusyWait();
    //Send garbage data to start a transfer
    *SSP_DR[GetPort()] = 0x00;
    //Wait for the transfer to complete
    BusyWait();
    //Record the recieved data into the buffer
    buf[i] = *SSP_DR[GetPort()];
  }
}

void SSP::Recv(uint8_t *buf, uint32_t len){
  //Read data back from the device, save data to buffer
  for(uint32_t i = 0; i < len; i++){
    //Make sure the device is ready to send data
    BusyWait();
    //Send garbage data to start a transfer
    *SSP_DR[GetPort()] = 0x00;
    //Wait for the transfer to complete
    BusyWait();
    //Record the recieved data into the buffer
    buf[i] = *SSP_DR[GetPort()];
  }
}

//Send a single packet, recieve a single packet
uint16_t SSP::Transfer(uint16_t send){
  uint16_t buf;
  Send(&send);
  Recv(&buf);
  return buf;
}

uint8_t SSP::GetStatus(){
  return *SSP_SR[GetPort()];
}

bool SSP::IsBusy(){
  //The 4th bit is the busy bit.
  return GetStatus() & (1 << 4);
}

bool SSP::RFIFOIsEmpty(){
  //The 2nd bit is the RNE bit. It's 1 if recieve FIFO is NOT empty
  return !(GetStatus() & (1 << 2));
}

bool SSP::TFIFOIsEmpty(){
  //The 0th but is the TFE bit
  return GetStatus() & (1 << 0);
}

void SSP::BusyRFIFOWait(){
  while(RFIFOIsEmpty()){
    continue;
  }
}

void SSP::BusyWait(){
  //Spin until SSP is done transferring
  while(IsBusy()){
    continue;
    //printf("Waiting...");
  }
}

uint8_t SSP::GetPort(){
  return _ssp_num;
}
