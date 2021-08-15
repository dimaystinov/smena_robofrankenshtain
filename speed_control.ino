/**********************************************************
 * Eduard Petrenko
 * 2021-08
 * робототехническая смена Физтех-Лицей
 * Проверка работоспособности энкодеров (измерение скорости в тиках/сек.)
Энкодеры повешены на порты 18, 19 (прерывания 4,5)
Передаточное число редуктора: 19, при 11 тиках на оборот оси энкодера 
получаем 209 тиков на оборот колеса
пины 32, 34 используеи для получения направления вращения

Задача программы - контроль скорости (в тиках/сек)
**********************************************************/ 

// управление моторами
#define PORT_DIR_1_R 8
#define PORT_DIR_2_R 7
#define PORT_POWER_R 5
#define PORT_DIR_1_L 4
#define PORT_DIR_2_L 9
#define PORT_POWER_L 6

void setupMotors(){
  pinMode(PORT_DIR_1_L,OUTPUT);
  pinMode(PORT_DIR_2_L,OUTPUT);
  pinMode(PORT_POWER_L,OUTPUT);
  pinMode(PORT_DIR_1_R,OUTPUT);
  pinMode(PORT_DIR_2_R,OUTPUT);
  pinMode(PORT_POWER_R,OUTPUT);
}

void motors(float _rPower, float _lPower){
  if(abs(_rPower)>0.004){
    if(_rPower>0){
      digitalWrite(PORT_DIR_1_R,HIGH);
      digitalWrite(PORT_DIR_2_R,LOW);
    }
    else{
      digitalWrite(PORT_DIR_1_R,LOW);
      digitalWrite(PORT_DIR_2_R,HIGH);
    }
    analogWrite(PORT_POWER_R,map(constrain(round(abs(_rPower)*255),0,255),0,255,50,255));
  }
  else{
    digitalWrite(PORT_POWER_R,LOW);
    digitalWrite(PORT_DIR_1_R,HIGH);
    digitalWrite(PORT_DIR_2_R,HIGH);
  }
  //left
  if(abs(_lPower)>0.004){
    if(_lPower>0){
      digitalWrite(PORT_DIR_1_L,HIGH);
      digitalWrite(PORT_DIR_2_L,LOW);
    }
    else{
      digitalWrite(PORT_DIR_1_L,LOW);
      digitalWrite(PORT_DIR_2_L,HIGH);
    }
    analogWrite(PORT_POWER_L,map(constrain(round(abs(_lPower)*255),0,255),0,255,50,255));
  }
  else{
    digitalWrite(PORT_POWER_L,LOW);
    digitalWrite(PORT_DIR_1_L,HIGH);
    digitalWrite(PORT_DIR_2_L,HIGH);
  }
}

void testDrive(){
  motors(0.5,0);
  delay(700);
  motors(1,0);
  delay(300);
  motors(0,0);
  delay(300);
  motors(-0.5,0);
  delay(700);
  motors(0,0);
  delay(300);
  motors(0,0.5);
  delay(700);
  motors(0,1);
  delay(300);
  motors(0,0);
  delay(300);
  motors(0,-0.5);
  delay(700);
  motors(0,0);
}

/**************************************************************************************
 * interrupts / encoders
 */
 

#define PIN_ENC_2 18
#define PIN_ENC_2_DIR 32
#define PIN_ENC_1 19
#define PIN_ENC_1_DIR 34


volatile unsigned long enc1p=0;
volatile unsigned long enc1n=0;
volatile unsigned long enc2p=0;
volatile unsigned long enc2n=0;

void encoder1(){
  if(digitalRead(PIN_ENC_1_DIR)) enc1p++; else enc1n++;
}

void encoder2(){
  if(digitalRead(PIN_ENC_2_DIR)) enc2n++; else enc2p++;
}

unsigned long encPrevReadTime1 = 0;
unsigned long encPrevReadTime2 = 0;

unsigned long lastTicks1p = 0;
unsigned long lastTicks1n = 0;
unsigned long lastTicks2p = 0;
unsigned long lastTicks2n = 0;

float speed1 = 0;
float speed2 = 0;

void readEnc1(){
  lastTicks1p = enc1p;
  enc1p=0;
  lastTicks1n = enc1n;
  enc1n=0;
  if(millis()-encPrevReadTime1>0){
    speed1 = 1000*((float)lastTicks1p - (float)lastTicks1n)/(millis() - encPrevReadTime1);
    encPrevReadTime1 = millis();
  }
}

void readEnc2(){
  lastTicks2p = enc2p;
  enc2p=0;
  lastTicks2n = enc2n;
  enc2n=0;
  if(millis()-encPrevReadTime2>0){
    speed2 = 1000*((float)lastTicks2p - (float)lastTicks2n)/(millis() - encPrevReadTime2);
    encPrevReadTime2 = millis();
  }
}

void readEnc(){
  readEnc1();
  readEnc2();
}

void setupEnc(){
   attachInterrupt(digitalPinToInterrupt(PIN_ENC_1), encoder1, FALLING);
   attachInterrupt(digitalPinToInterrupt(PIN_ENC_2), encoder2, FALLING);
}


/************************************************************************************************/

#define PORT_KICK 30

void setupKicker(){
  pinMode(PORT_KICK, OUTPUT);
  digitalWrite(PORT_KICK,LOW);
}

/*************************************************************************************************
 * speed control
 */

float targSpeed1 = 0;
float targSpeed2 = 0;

#define SPEED_STEP_BY_STEP

#ifdef SPEED_STEP_BY_STEP

#define TOTAL_STEPS 36
float r[TOTAL_STEPS] = {0,0,0.5,0.5,0.5,1,1,1,0,0,-0.5,-0.5,-0.5,-1,-1,-1,0,0,0,0,0  ,0  ,0  ,0,0,0,0,0,0   ,0   ,0   ,0 ,0 ,0 ,0,0};
float l[TOTAL_STEPS] = {0,0,0  ,0  ,0  ,0,0,0,0,0,0   ,0   ,0   ,0 ,0 ,0 ,0,0,0,0,0.5,0.5,0.5,1,1,1,0,0,-0.5,-0.5,-0.5,-1,-1,-1,0,0};
int step=0;

unsigned long prevSpeedSwitch = 0;
int stepSwitchPeriod = 100;

void calcSpeed(){
  if(millis()>=prevSpeedSwitch+stepSwitchPeriod){
    prevSpeedSwitch = millis();
    step++;
    if(step==TOTAL_STEPS) step=0;
    targSpeed1=r[step];
    targSpeed2=l[step];
  }  
}

#else

void calcSpeed(){
  
}

#endif
 
/*************************************************************************************************
 * trace info
 */

void trace(){
  Serial.print(millis()); //текущее время
  Serial.print("\t");
  Serial.print(step); // текущий такт в управлении скоростями
  Serial.print("\t");
  Serial.print(r[step]); // сколько подаём на правый мотор
  Serial.print("\t");
  Serial.print(l[step]);  // сколько подаём на левый мотор
  Serial.print("\t");
  Serial.print(lastTicks1p); // тиков вперёд справа
  Serial.print("\t");
  Serial.print(lastTicks1n); // тиков назад справа
  Serial.print("\t");
  Serial.print(lastTicks2p); // тиков вперёд слева
  Serial.print("\t");
  Serial.print(lastTicks2n); // тиков назад слева
  Serial.print("\t");
  Serial.print(speed1); // скорость справа
  Serial.print("\t");
  Serial.print(speed2); // скорость слева
  Serial.print("\n"); 
}
 
/*************************************************************************************************
 * main loop
 */

void setup() {
  setupEnc();
  setupMotors();
  setupKicker();
  Serial.begin(9600);
//  testDrive();
}


void loop(){

  readEnc();
  calcSpeed();
  trace();
  motors(targSpeed1,targSpeed2);
  
  delay(25);
}
