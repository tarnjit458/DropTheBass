#pragma once

#include <cstdint>
#include "L0_LowLevel/LPC40xx.h"
#include "utility/log.hpp"
//This is a collection of utilities to set the functions of the MCU Pins

//Turn off the pullup/pulldown resistors
void PinconPulloff(uint8_t port, uint8_t pin);

//Turn on the pullup resistor
void PinconPullup(uint8_t port, uint8_t pin);

//Turn on the pulldown resistor
void PinconPulldown(uint8_t port, uint8_t pin);

//Set the function of the pin
void PinconSetFunc(uint8_t port, uint8_t pin, uint8_t func);

//TODO
void PinconOpendrain(uint8_t port, uint8_t pin, bool drain);
