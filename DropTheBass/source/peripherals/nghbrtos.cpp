#include "nghbrtos.hpp"

void Motor::Init( uint8_t sig_a_port,
                  uint8_t sig_a_pin,
                  uint8_t sig_b_port,
                  uint8_t sig_b_pin){
  _sig_a = new GPIO(sig_a_port, sig_a_pin);
  _sig_a->SetAsOutput();
  _sig_a->SetLow();

  _sig_b = new GPIO(sig_b_port, sig_b_pin);
  _sig_b->SetAsOutput();
  _sig_b->SetLow();
}

void Motor::Forward(uint16_t time){
  _sig_a->SetHigh();
  _sig_b->SetLow();
  vTaskDelay(time);
  _sig_a->SetLow();
  _sig_b->SetLow();
}

void Motor::Backward(uint16_t time){
  _sig_b->SetHigh();
  _sig_a->SetLow();
  vTaskDelay(time);
  _sig_a->SetLow();
  _sig_b->SetLow();
}

void Motor::Toggle(uint16_t time){
  time = time/2;
  Forward(time);
  Backward(time);
}

Motor::~Motor(){
  delete _sig_a;
  delete _sig_b;
}
