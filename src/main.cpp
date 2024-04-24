#include <Arduino.h>
#include "Wheel_odom.h"
#include "Clock_utils.h"
#include "Timer_utils.h"
#include "PID_controller.h"
#include "Diff_drive_unicycle.h"

#define MAIN_LOOP_FREQ 100.0

Timer_utils timer(MAIN_LOOP_FREQ);
Clock_utils clock;
Wheel_odom wheel_odom;
Diff_drive_unicycle ddr_uni;
PID_controller controller_R, controller_L;

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

#define ENC_1_CPR 1700
#define ENC_2_CPR 1700
#define WHEEL_R 0.0425
#define WHEEL_L 0.2

#define V_C_MAX 1.0
#define W_C_MAX 1.0
#define V_BAT_MAX 12.0
#define PWM_MAX 255.0

#define Kp_R    0.0
#define Ki_R    0.0
#define Kd_R    0.0
#define dt_R    1.0/MAIN_LOOP_FREQ
#define I_max_R V_BAT_MAX*0.95
#define u_max_R V_BAT_MAX*0.95
#define fc_R    MAIN_LOOP_FREQ*0.5

#define Kp_L    0.0
#define Ki_L    0.0
#define Kd_L    0.0
#define dt_L    1.0/MAIN_LOOP_FREQ
#define I_max_L V_BAT_MAX*0.95
#define u_max_L V_BAT_MAX*0.95
#define fc_L    MAIN_LOOP_FREQ*0.5

volatile long count1 = 0;
volatile long count2 = 0;

double x, y, th, v, w;
double v_0, w_0;

double w_R, w_L;
double w_R_0, w_L_0;

float V_R, V_L;
float PWM_R, PWM_L;

enum Mode{
  unicycle = 0,
  differential_drive = 1
};

Mode mode;

void setup() {
  Serial.begin(9600);
  wheel_odom.set_param(ENC_1_CPR, WHEEL_R, WHEEL_L);
  wheel_odom.set_dt(1.0/MAIN_LOOP_FREQ);
  ddr_uni.set_param(WHEEL_R, WHEEL_L);
  ddr_uni.set_v_max(V_C_MAX);
  ddr_uni.set_w_max(W_C_MAX);
  clock.init();
  timer.init(MAIN_LOOP_FREQ);
  init_encoders();
  init_motors();
  command_motors(0, 0);
  controller_R.set_param(Kp_R, Ki_R, Kd_R, dt_R, I_max_R, u_max_R, fc_R);
  controller_L.set_param(Kp_L, Ki_L, Kd_L, dt_L, I_max_L, u_max_L, fc_L);
}

void loop() {
  // 0. Odometry
  wheel_odom.update(count1, count2);
  wheel_odom.get_pose(&x, &y, &th);
  wheel_odom.get_twist(&v, &w);
  wheel_odom.get_wheel_speed(&w_R, &w_L);

  // 1. communication

  // 2. v_0, w_0 --> w_R_0, w_L_0
  ddr_uni.uni2ddr(v_0, w_0, &w_R_0, &w_L_0);

  // 3. w_R_0, w_L_0 --> V_R, V_L
  V_R = controller_R.cal_u(w_R_0, w_R, false);
  V_L = controller_L.cal_u(w_L_0, w_L, false);

  // 4. V_R, V_L --> PWM_R, PWM_L
  PWM_R = (PWM_MAX / V_BAT_MAX) * V_R;
  PWM_L = (PWM_MAX / V_BAT_MAX) * V_L;

  command_motors(PWM_R, PWM_L);

  // Serial.print(clock.get_t_now_s(),2);
  // Serial.print(", ");
  // Serial.print(count1);
  // Serial.print(", ");
  // Serial.print(count2);
  // Serial.print(", ");
  // Serial.print(x_c*100.0);
  // Serial.print(", ");
  // Serial.print(y_c*100.0);
  // Serial.print(", ");
  // Serial.print(th_c*180.0/(22.0/7.0));
  // Serial.print(w_R);
  // Serial.print(", ");
  // Serial.print(w_R);
  // Serial.print(", ");
  // Serial.print(v);
  // Serial.print(", ");
  // Serial.println(w);
  
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