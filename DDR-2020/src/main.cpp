#include <Arduino.h>
#include "Clock_utils.h"
#include "Timer_utils.h"
#include "Wheel_odom.h"

#define MAIN_LOOP_FREQ 100.0

Timer_utils timer(MAIN_LOOP_FREQ);
Clock_utils clock;

void init_motors();
void command_motors(float pwm_1, float pwm_2);
void command_motor_1(float pwm_1);
void command_motor_2(float pwm_2);
void init_encoder_interrupt();
void init_encoders();

#define MOTOR_1_PIN_A   8
#define MOTOR_1_PIN_B   9
#define MOTOR_1_PIN_PWM 7

#define MOTOR_2_PIN_A   4
#define MOTOR_2_PIN_B   5
#define MOTOR_2_PIN_PWM 6

#define ENC_1_PIN_A 18
#define ENC_1_PIN_B 19
#define ENC_2_PIN_A 2
#define ENC_2_PIN_B 3

volatile long count1 = 0;
volatile long count2 = 0;

void setup() {
  clock.init();
  timer.init(100);
  Serial.begin(9600);
  init_encoders();
  init_motors();
  command_motors(0, 0);
}

void loop() {
  command_motors(0, 0);
  Serial.print(count1);
  Serial.print(", ");
  Serial.print(count2);
  Serial.print(", ");
  Serial.println(clock.get_t_now_s(),2);
  timer.sleep();
}

void init_encoder_interrupt(){
  //interrupt [2,3,4,5]->[19,18,2,3]
  EICRA = (1 << ISC20) | (1 << ISC30);
  EICRB = (1 << ISC40) | (1 << ISC50);
  EIMSK = (1 << INT2) | (1 << INT3) | (1 << INT4) | (1 << INT5);
  sei();
}

void init_encoders(){
  init_encoder_interrupt();
}

ISR(INT2_vect){
  int a1 = digitalRead(ENC_1_PIN_A);
  int b1 = digitalRead(ENC_1_PIN_B);
  
  if(a1==1){
    if(b1==1){
      count1--;
    }
    else{
      count1++;
    }
  }
  else{
    if(b1==0){
      count1--;
    }
    else{
      count1++;
    }
  }
}

ISR(INT3_vect){
  int a1 = digitalRead(ENC_1_PIN_A);
  int b1 = digitalRead(ENC_1_PIN_B);
  
  if(b1==1){
    if(a1==0){
      count1--;
    }
    else{
      count1++;
    }
  }
  else{
    if(a1==1){
      count1--;
    }
    else{
      count1++;
    }
  }
}

ISR(INT4_vect){
  int a2 = digitalRead(ENC_2_PIN_A);
  int b2 = digitalRead(ENC_2_PIN_B);
  
  if(a2==1){
    if(b2==1){
      count2++;
    }
    else{
      count2--;
    }
  }
  else{
    if(b2==0){
      count2++;
    }
    else{
      count2--;
    }
  }
}

ISR(INT5_vect){
  int a2 = digitalRead(ENC_2_PIN_A);
  int b2 = digitalRead(ENC_2_PIN_B);
  
  if(b2==1){
    if(a2==0){
      count2++;
    }
    else{
      count2--;
    }
  }
  else{
    if(a2==1){
      count2++;
    }
    else{
      count2--;
    }
  }
}

void init_motors(){
  pinMode(MOTOR_1_PIN_A, OUTPUT);
  pinMode(MOTOR_1_PIN_B, OUTPUT);
  pinMode(MOTOR_1_PIN_PWM, OUTPUT);

  pinMode(MOTOR_2_PIN_A, OUTPUT);
  pinMode(MOTOR_2_PIN_B, OUTPUT);
  pinMode(MOTOR_2_PIN_PWM, OUTPUT);
}

void command_motor_1(float pwm_1){
  if(pwm_1<0){
    digitalWrite(MOTOR_1_PIN_A,0);
    digitalWrite(MOTOR_1_PIN_B,1);
    analogWrite(MOTOR_1_PIN_PWM,(int)(-1*pwm_1));
  }
  else{
    digitalWrite(MOTOR_1_PIN_A,1);
    digitalWrite(MOTOR_1_PIN_B,0);
    analogWrite(MOTOR_1_PIN_PWM,(int)pwm_1);
  }
}

void command_motor_2(float pwm_2){
  if(pwm_2<0){
    digitalWrite(MOTOR_2_PIN_A,0);
    digitalWrite(MOTOR_2_PIN_B,1);
    analogWrite(MOTOR_2_PIN_PWM,(int)(-1*pwm_2));
  }
  else{
    digitalWrite(MOTOR_2_PIN_A,1);
    digitalWrite(MOTOR_2_PIN_B,0);
    analogWrite(MOTOR_2_PIN_PWM,(int)pwm_2);
  }
}

void command_motors(float pwm_1, float pwm_2){
  command_motor_1(pwm_1);
  command_motor_2(pwm_2);
}