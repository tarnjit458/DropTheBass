#include "ngpincon.hpp"

volatile uint32_t* PIN_ARR[6][32] {
  {&LPC_IOCON->P0_0,
  &LPC_IOCON->P0_1,
  &LPC_IOCON->P0_2,
  &LPC_IOCON->P0_3,
  &LPC_IOCON->P0_4,
  &LPC_IOCON->P0_5,
  &LPC_IOCON->P0_6,
  &LPC_IOCON->P0_7,

  &LPC_IOCON->P0_8,
  &LPC_IOCON->P0_9,
  &LPC_IOCON->P0_10,
  &LPC_IOCON->P0_11,
  &LPC_IOCON->P0_12,
  &LPC_IOCON->P0_13,
  &LPC_IOCON->P0_14,
  &LPC_IOCON->P0_15,

  &LPC_IOCON->P0_16,
  &LPC_IOCON->P0_17,
  &LPC_IOCON->P0_18,
  &LPC_IOCON->P0_19,
  &LPC_IOCON->P0_20,
  &LPC_IOCON->P0_21,
  &LPC_IOCON->P0_22,
  &LPC_IOCON->P0_23,

  &LPC_IOCON->P0_24,
  &LPC_IOCON->P0_25,
  &LPC_IOCON->P0_26,
  &LPC_IOCON->P0_27,
  &LPC_IOCON->P0_28,
  &LPC_IOCON->P0_29,
  &LPC_IOCON->P0_30,
  &LPC_IOCON->P0_31},

  {&LPC_IOCON->P1_0,
  &LPC_IOCON->P1_1,
  &LPC_IOCON->P1_2,
  &LPC_IOCON->P1_3,
  &LPC_IOCON->P1_4,
  &LPC_IOCON->P1_6,
  &LPC_IOCON->P1_5,
  &LPC_IOCON->P1_7,

  &LPC_IOCON->P1_8,
  &LPC_IOCON->P1_9,
  &LPC_IOCON->P1_11,
  &LPC_IOCON->P1_10,
  &LPC_IOCON->P1_12,
  &LPC_IOCON->P1_13,
  &LPC_IOCON->P1_14,
  &LPC_IOCON->P1_15,

  &LPC_IOCON->P1_16,
  &LPC_IOCON->P1_17,
  &LPC_IOCON->P1_18,
  &LPC_IOCON->P1_19,
  &LPC_IOCON->P1_20,
  &LPC_IOCON->P1_21,
  &LPC_IOCON->P1_22,
  &LPC_IOCON->P1_23,

  &LPC_IOCON->P1_24,
  &LPC_IOCON->P1_25,
  &LPC_IOCON->P1_26,
  &LPC_IOCON->P1_27,
  &LPC_IOCON->P1_28,
  &LPC_IOCON->P1_29,
  &LPC_IOCON->P1_30,
  &LPC_IOCON->P1_31},

  {&LPC_IOCON->P2_0,
  &LPC_IOCON->P2_1,
  &LPC_IOCON->P2_2,
  &LPC_IOCON->P2_3,
  &LPC_IOCON->P2_4,
  &LPC_IOCON->P2_5,
  &LPC_IOCON->P2_6,
  &LPC_IOCON->P2_7,

  &LPC_IOCON->P2_8,
  &LPC_IOCON->P2_9,
  &LPC_IOCON->P2_10,
  &LPC_IOCON->P2_11,
  &LPC_IOCON->P2_12,
  &LPC_IOCON->P2_13,
  &LPC_IOCON->P2_14,
  &LPC_IOCON->P2_15,

  &LPC_IOCON->P2_16,
  &LPC_IOCON->P2_17,
  &LPC_IOCON->P2_18,
  &LPC_IOCON->P2_19,
  &LPC_IOCON->P2_20,
  &LPC_IOCON->P2_21,
  &LPC_IOCON->P2_22,
  &LPC_IOCON->P2_23,

  &LPC_IOCON->P2_24,
  &LPC_IOCON->P2_25,
  &LPC_IOCON->P2_26,
  &LPC_IOCON->P2_27,
  &LPC_IOCON->P2_28,
  &LPC_IOCON->P2_29,
  &LPC_IOCON->P2_30,
  &LPC_IOCON->P2_31},

  {&LPC_IOCON->P3_0,
  &LPC_IOCON->P3_1,
  &LPC_IOCON->P3_2,
  &LPC_IOCON->P3_3,
  &LPC_IOCON->P3_4,
  &LPC_IOCON->P3_6,
  &LPC_IOCON->P3_5,
  &LPC_IOCON->P3_7,

  &LPC_IOCON->P3_8,
  &LPC_IOCON->P3_9,
  &LPC_IOCON->P3_11,
  &LPC_IOCON->P3_10,
  &LPC_IOCON->P3_12,
  &LPC_IOCON->P3_13,
  &LPC_IOCON->P3_14,
  &LPC_IOCON->P3_15,

  &LPC_IOCON->P3_16,
  &LPC_IOCON->P3_17,
  &LPC_IOCON->P3_18,
  &LPC_IOCON->P3_19,
  &LPC_IOCON->P3_20,
  &LPC_IOCON->P3_21,
  &LPC_IOCON->P3_22,
  &LPC_IOCON->P3_23,

  &LPC_IOCON->P3_24,
  &LPC_IOCON->P3_25,
  &LPC_IOCON->P3_26,
  &LPC_IOCON->P3_27,
  &LPC_IOCON->P3_28,
  &LPC_IOCON->P3_29,
  &LPC_IOCON->P3_30,
  &LPC_IOCON->P3_31},

  {&LPC_IOCON->P4_0,
  &LPC_IOCON->P4_1,
  &LPC_IOCON->P4_2,
  &LPC_IOCON->P4_3,
  &LPC_IOCON->P4_4,
  &LPC_IOCON->P4_6,
  &LPC_IOCON->P4_5,
  &LPC_IOCON->P4_7,

  &LPC_IOCON->P4_8,
  &LPC_IOCON->P4_9,
  &LPC_IOCON->P4_11,
  &LPC_IOCON->P4_10,
  &LPC_IOCON->P4_12,
  &LPC_IOCON->P4_13,
  &LPC_IOCON->P4_14,
  &LPC_IOCON->P4_15,

  &LPC_IOCON->P4_16,
  &LPC_IOCON->P4_17,
  &LPC_IOCON->P4_18,
  &LPC_IOCON->P4_19,
  &LPC_IOCON->P4_20,
  &LPC_IOCON->P4_21,
  &LPC_IOCON->P4_22,
  &LPC_IOCON->P4_23,

  &LPC_IOCON->P4_24,
  &LPC_IOCON->P4_25,
  &LPC_IOCON->P4_26,
  &LPC_IOCON->P4_27,
  &LPC_IOCON->P4_28,
  &LPC_IOCON->P4_29,
  &LPC_IOCON->P4_30,
  &LPC_IOCON->P4_31},

  {&LPC_IOCON->P5_0,
  &LPC_IOCON->P5_1,
  &LPC_IOCON->P5_2,
  &LPC_IOCON->P5_3,
  &LPC_IOCON->P5_4}
};

void PinconPulloff(uint8_t port, uint8_t pin){
  //Clear the two pull resistor bits
  *PIN_ARR[port][pin] &= ~(0b11 << 3);
}

void PinconPulldown(uint8_t port, uint8_t pin){
  PinconPulloff(port, pin);
  //Set the pulldown
  *PIN_ARR[port][pin] |= (0b1 << 3);
}

void PinconPullup(uint8_t port, uint8_t pin){
  PinconPulloff(port, pin);
  //Set the pulldown
  *PIN_ARR[port][pin] |= (0b1 << 4);
}

void PinconSetFunc(uint8_t port, uint8_t pin, uint8_t func){
  //Function bits are only 3 bits wide
  if(func > 0b111){
    LOG_ERROR("Invalid function 0x%x on P%d_%d", func, port, pin);
    return;
  }
  //Set the function of the pin
  *PIN_ARR[port][pin] |= func;
}

void PinconOpendrain(uint8_t port, uint8_t pin, bool drain){
  if(drain){
    *PIN_ARR[port][pin] |= (0b1 << 10);
  }
  else{
    *PIN_ARR[port][pin] &= ~(0b1 << 10);
  }
}
