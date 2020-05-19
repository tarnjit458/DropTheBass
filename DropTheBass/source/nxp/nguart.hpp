#pragma once

#include "L0_LowLevel/LPC40xx.h"
#include "ngpincon.hpp"
#include "L0_LowLevel/interrupt.hpp"
#include "utility/log.hpp"

#include <iterator>

class UART{
public:
  //Constructor for the UART object
  //@param uint8_t port: The port number for the interrupt (2 or 3
  //@param uint16_t dl: The value to push into the divisor latches to set the bitrate
  UART(uint8_t port, uint16_t dl);

  //Destructor for the UART object
  ~UART();

  //Initialize the UART object with the private class variables
  void Init();

  //Send a single byte over UART
  //@param uint8_t byte: The byte to send
  void SendByte(uint8_t byte);

  //Send a string over UART
  //@param const char*: The string to send
  void SendString(const char* string);

  //Recieve a byte over UART. Busy waits until there's a byte to get and
  //reports if there are any transmission errors with a LOG_ERROR
  //@return uint8_t: The byte recieved over UART
  uint8_t RecvByte();

  //Send a buffer over UART
  //@param uint8_t* buf: A pointer to a buffer of data to send
  //@param uint16_t len: The length of the buffer to send
  void SendBuffer(uint8_t* buf, uint16_t len);

  //Receive len bytes over UART and place them into a buffer. This will busy
  //wait until it gets len bytes. Also reports transmission errors with LOG_ERROR
  //@param uint8_t buf: A pointer to a buffer to place the data into
  //@param uint16_t len: The number of bytes to get from UART
  void RecvBuffer(uint8_t* buf, uint16_t len);

  //Check to see if there's data in the recieve FIFO
  //@return bool: True if there's data in the FIFO, false if not
  bool CheckRecvData();

  //Busy wait until there's data in the receive FIFO
  void WaitRecvData();

  //Check the LSR register to see if any of the error bits are set
  //@return bool: True if there are errors, false if not
  bool CheckError();

  //Get the port number of the UART object
  //@return uint8_t: The port number
  uint8_t GetPort();

  //Get the value being place into the two divisor latches
  //@return uint16_t: The two bytes in the divisol latches DLM and DLL
  uint16_t GetDL();

  //Attach an ISR to RBR inerrupt of the UART object. Note: This function
  //does not automatically clear the interrupt after servicing it! The RBR
  //register must be read to clear the interrupt
  //@param IsrPointer isr: A pointer to the function
  void AttachRBRIsr(IsrPointer isr);

  //Enable interrupts on the UART object
  void EnableRBRInterrupt();

  //Disable Interrupts on the UART object.
  void DisableRBRInterrupt();

private:
  uint16_t _dl;
  uint8_t _port;
};
