# include "nguart.hpp"

// Lookup table for all the UART registers
// Don't touch UART0, UART1, UART4!
// UART0 is reserved for programming, UART1 and UART4 are special
volatile LPC_UART_TypeDef* LPC_UART[] = {LPC_UART2, LPC_UART3};
uint8_t UART_INT_MAP[] = {UART2_IRQn, UART3_IRQn};
uint8_t UART_PCON_BIT[]     = {24, 25};
uint8_t UART_TX_PORT_NUM[]  = {4,  4};
uint8_t UART_TX_PIN_NUM[]   = {22, 28};
uint8_t UART_RX_PORT_NUM[]  = {4,  4};
uint8_t UART_RX_PIN_NUM[]   = {23, 29};
IRQn_Type UART_IRQ_MAP [] =   {UART2_IRQn, UART3_IRQn};

UART::UART(uint8_t port, uint16_t dl){
  if(port != 2 && port != 3){
    LOG_ERROR("Invalid UART Port %d!", port);
    return;
  }
  _dl = dl;
  _port = port;
}

UART::~UART(){
  LOG_DEBUG("UART%d is being deleted...", _port);
}

void UART::Init(){
  //Set the PCON register appropriately
  LPC_SC->PCONP |= (1 << UART_PCON_BIT[GetPort() - 2]);
  //Set DLAB = 1 so we can access divisor latches
  LPC_UART[GetPort() - 2]->LCR |= (1 << 7);
  //Set DLL and DLM to set the baud rate
  LPC_UART[GetPort() - 2]->DLL = (0x00FF & GetDL()) >> 0;
  LPC_UART[GetPort() - 2]->DLM = (0xFF00 & GetDL()) >> 8;
  //We're not using FDR, so just set it to its default value
  LPC_UART[GetPort() - 2]->FDR = (1 << 4);
  //Set the FIFO enable bit in FCR
  LPC_UART[GetPort() - 2]->FCR |= (1 << 0);
  //Set packet length to 8 bits
  LPC_UART[GetPort() - 2]->LCR |= (0x03 << 0);
  //Set the IOCON Registers
  //Set the TX pin
  PinconSetFunc(UART_TX_PORT_NUM[GetPort() - 2], UART_TX_PIN_NUM[GetPort() - 2], 0b010);
  //Set the RX Pin
  PinconSetFunc(UART_RX_PORT_NUM[GetPort() - 2], UART_RX_PIN_NUM[GetPort() - 2], 0b010);
  //Set DLAB = 0 so we can turn on interrupts
  LPC_UART[GetPort() - 2]->LCR &= ~(1 << 7);
}

bool UART::CheckRecvData(){
  //Return the RDR bit in the LSR register
  return LPC_UART[GetPort() - 2]->LSR & (1 << 0);
}

void UART::WaitRecvData(){
  //Keep polling the RDR bit in the LSR register until it's set
  while(!CheckRecvData()){
    continue;
  }
}

bool UART::CheckError(){
  //Check the LSR register to see if there's an overrun, parity, or framing error
  return LPC_UART[GetPort() - 2]->LSR & ((1 << 1) | (1 << 2) | (1 << 3));
}

void UART::SendByte(uint8_t byte){
  LPC_UART[GetPort() - 2]->THR = byte;
}

void UART::SendBuffer(uint8_t* buf, uint16_t len){
  for(uint16_t i = 0; i < len; i++){
    SendByte(buf[i]);
  }
}

void UART::RecvBuffer(uint8_t* buf, uint16_t len){
  for(uint16_t i = 0; i < len; i++){
    buf[i] = RecvByte();
  }
}

uint8_t UART::RecvByte(){
  WaitRecvData();
  if(CheckError()){
    LOG_ERROR("WARNING: UART%d is reporting a transmission error", GetPort());
  }
  return LPC_UART[GetPort() - 2]->RBR;
}
void UART::SendString(const char* string){
  for(uint16_t i = 0; string[i] != '\0'; i++){
    SendByte((uint8_t)string[i]);
  }
}

uint8_t UART::GetPort(){
  return _port;
}

uint16_t UART::GetDL(){
  return _dl;
}

void UART::AttachRBRIsr(IsrPointer isr){
  //Register the interrupt handler to the correct UART interrupt
  RegisterIsr(UART_IRQ_MAP[GetPort() - 2], isr);
}

void UART::EnableRBRInterrupt(){
  LPC_UART[GetPort() - 2]->IER |= (1 << 0);
  NVIC_EnableIRQ(UART_IRQ_MAP[GetPort() - 2]);
}

void UART::DisableRBRInterrupt(){
  LPC_UART[GetPort() - 2]->IER |= (1 << 0);
  NVIC_DisableIRQ(UART_IRQ_MAP[GetPort() - 2]);
}
