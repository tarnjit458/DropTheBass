#pragma once
#include "../nxp/nggpio.hpp"

#include "third_party/FreeRTOS/Source/include/FreeRTOS.h"
#include "third_party/FreeRTOS/Source/include/task.h"

#include "utility/log.hpp"

//This is a FreeRTOS enhanced class for driving motors with an H bridge
//It uses vTaskDelay to create a low CPU usage motor driver
class Motor{
public:
  //Initialize the motor
  void Init(uint8_t sig_a_port,
            uint8_t sig_a_pin,
            uint8_t sig_b_port,
            uint8_t sig_b_pin);
  //Move the motor forward
  void Forward(uint16_t time);
  //Move the motor backward
  void Backward(uint16_t time);
  //Move the motor forward and then backward
  void Toggle(uint16_t time);
  //Destructor
  ~Motor();

private:
  GPIO *_sig_a;
  GPIO *_sig_b;
};
