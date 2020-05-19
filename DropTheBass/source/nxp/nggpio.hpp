#pragma once

#include "L0_LowLevel/LPC40xx.h"
#include "ngpincon.hpp"
#include "third_party/FreeRTOS/Source/include/FreeRTOS.h"
#include "L0_LowLevel/interrupt.hpp"
#include "utility/log.hpp"

#include <iterator>


class GPIO{
public:
  //Enumerate the direction of the GPIO pin
  enum class Direction : uint8_t
  {
    kInput  = 0,
    kOutput = 1
  };
  //Enumerate the state of the GPIO pin
  enum class State : uint8_t
  {
    kLow  = 0,
    kHigh = 1
  };
  //Enumerate the interrupt triggers of the GPIO pin
  enum class Edge : uint8_t{
    kNone = 0,
    kRising,
    kFalling,
    kBoth
  };

  static constexpr size_t kIntPorts = 3;
  static constexpr size_t kPins = 32;

  //Attach an ISR to the GPIO on a given trigger
  //@param isr function: The function to run when the ISR trigger occurs
  //@param edge Edge: The edge to trigger the ISR on
  //@return uint8_t: The result. 0 for Succes, nonzero on failure
  uint8_t AttachIsrHandle(IsrPointer isr, Edge edge);

  //Enable interrupts on the GPIO
  static void EnableInterrupts();

  //Constructor for the GPIO pin.
  //@param Pin: The pin number on the chip
  //@param Port: The port number that the pin belongs to
  GPIO(uint8_t port, uint8_t pin);

  //Set the pin as an input
  void SetAsInput();

  //Set the pin as an output
  void SetAsOutput();

  //Set the direction of the pin based on a bool
  //@param dir: False for input, True for output
  void SetDirection(bool dir);

  //Set the direction of a pin based on a Direction object
  void SetDirection(Direction dir);

  //Set the pin output voltage to 3v3
  void SetHigh();

  //Set the pin output voltage to GND
  void SetLow();

  //Set the pin output based on a bool
  //state: False for GND, True for 3v3
  void Set(bool state);

  //Set the pin output based on a State object
  void Set(State state);

  //Toggle the state of the GPIO
  void Toggle();

  //Turn on the GPIO pullup resistor
  void SetPullup();

  //Turn on the GPIO pulldown resistor
  void SetPulldown();

  //Turn off the GPIO pullup/down resistors
  void SetFloating();

  //Get the state of the GPIO as a boolean
  //Return: The state as a State object
  State Read();

  //Get the state of the GPIO as a bool
  bool ReadBool();

  //Get the port that the GPIO is set to
  uint8_t GetPort();

  //Get the pin that the GPIO is set to
  uint8_t GetPin();

  ~GPIO();
private:
  //Don't touch!
  uint8_t port_;
  uint8_t pin_;
  //Static ISR lookup table (same for all GPIO objects)
  static IsrPointer pin_isr_map[kIntPorts][kPins];
  //ISR that will be called when the system detects an Interrupt on any GPIO
  //Reads which pin caused the interrupt, looks up the ISR registered
  //to it, and calls the interrupt.
  static void gpio_int_handler();
};
