/**********************************************************
 * Eduard Petrenko
 * 2021-08
 * робототехническая смена Физтех-Лицей
 * Управление движением робота: движение на заданное число мм, поворот на заданное число градусов
 * функции moveMM(), rotateDeg(), используют moveTicks()
 * в начале исполнения вызывется функция onTaskBegin(), в конце - onTaskEnd()
 * геометрия/физика робота описывается в отдельной секции
Энкодеры повешены на порты 18, 19 (прерывания 4,5)
Передаточное число редуктора: 19, при 11 тиках на оборот оси энкодера 
получаем 209 тиков на оборот колеса
пины 32, 34 используеи для получения направления вращения

Задача программы - контроль скорости (в тиках/сек)
**********************************************************/ 
#define MYDEBUG

/*************************************************************************************************
 * task management
 */

#define TASK_READY 0
#define TASK_PROCESSING 1
int taskStatus = TASK_READY;

int targTicks1 = 0;
int targTicks2 = 0;

int doneTicks1 = 0;
int doneTicks2 = 0;

float targSpeed1 = 0;
float targSpeed2 = 0;

#define SPEED_MOVE 400
#define SPEED_ROTATE 300

void onTaskBegin(){
}

void onTaskEnd(){
}

void startTask(int _ticks1, int _ticks2){
  taskStatus = TASK_PROCESSING;
  doneTicks1 = 0;
  doneTicks2 = 0;
  targTicks1 = _ticks1;
  targTicks2 = _ticks2;
  onTaskBegin();
  if((float)_ticks1*(float)_ticks2 >= 0) { //если скорости направлены в одну сторону
    if(_ticks1>0) {
      targSpeed1=SPEED_MOVE;
    }
    else {
      if(_ticks1<0) {
        targSpeed1=-SPEED_MOVE;
      }
      else {
        targSpeed1=0;
      }
    }
    if(_ticks2>0) {
      targSpeed2=SPEED_MOVE;
    }
    else {
      if(_ticks2<0) {
        targSpeed2=-SPEED_MOVE;
      }
      else {
        targSpeed2=0;
      }
    }
  }
  else {
    if(_ticks1>0) {
      targSpeed1=SPEED_ROTATE;
    }
    else {
      if(_ticks1<0) {
        targSpeed1=-SPEED_ROTATE;
      }
      else {
        targSpeed1=0;
      }
    }
    if(_ticks2>0) {
      targSpeed2=SPEED_ROTATE;
    }
    else {
      if(_ticks2<0) {
        targSpeed2=-SPEED_ROTATE;
      }
      else {
        targSpeed2=0;
      }
    }
  }
}

void checkTask(){
  if((taskStatus==TASK_PROCESSING) && (abs(doneTicks1)-abs(targTicks1)>=0) && (abs(doneTicks2)-abs(targTicks2)>=0)){
    taskStatus = TASK_READY;
    targSpeed1=0;
    targSpeed2=0;
    onTaskEnd();
  }
}

/*************************************************************************************************
 * physics & geometry of robot. Task compute
 */


/*************************************************************************************************
 * motors control
 */
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
  if(taskStatus==TASK_PROCESSING){
    doneTicks1+=(lastTicks1p - lastTicks1n);
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
  if(taskStatus==TASK_PROCESSING){
    doneTicks2+=(lastTicks2p - lastTicks2n);
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
float power1 = 0;
float power2 = 0;

float prevE1 = 0;
float prevE2 = 0;

float intE1 = 0;
float intE2 = 0;

unsigned long prevCalcTime = 0;

float intFactor=0.9;

// коэфициенты ПИД-регулятора
float pid_p = 0.005;
float pid_d = 0.7;
float pid_i = 0.001;

#define PID_P_LIMIT 0.8
#define PID_D_LIMIT 0.5
#define PID_I_LIMIT 0.5

// рассчёт целевой мощности моторов
void calcSpeed(){
  float e1;
  float e2;

  if(abs(targSpeed1)<0.5){
    power1=0;
    e1=0;
    prevE1=0;
  }
  else{
    e1=(targSpeed1-speed1)/abs(targSpeed1);
    float diff1=0;
    if(prevCalcTime>0){
      diff1=(e1-prevE1)/(float)(millis()-prevCalcTime);
    }
    prevE1 = e1;
    intE1 = e1+intFactor*intE1;
    power1+=constrain(e1*pid_p,-PID_P_LIMIT,PID_P_LIMIT)+
           constrain(diff1*pid_d,-PID_D_LIMIT,PID_D_LIMIT)+
           constrain(intE1*pid_i,-PID_I_LIMIT,PID_I_LIMIT);
  }

  if(abs(targSpeed2)<0.5){
    power2=0;
    e2=0;
    prevE2=0;
  }
  else{
    e2=(targSpeed2-speed2)/abs(targSpeed2);
    float diff2=0;
    if(prevCalcTime>0){
      diff2=(e2-prevE2)/(float)(millis()-prevCalcTime);
    }
    prevE2 = e2;
    intE2 = e2+intFactor*intE2;
    power2+=constrain(e2*pid_p,-PID_P_LIMIT,PID_P_LIMIT)+
           constrain(diff2*pid_d,-PID_D_LIMIT,PID_D_LIMIT)+
           constrain(intE2*pid_i,-PID_I_LIMIT,PID_I_LIMIT);
  }

  prevCalcTime=millis();
}
 
/*************************************************************************************************
 * trace info
 */

//удобно для просмотра на serial plotter
void trace(){
  Serial.print(millis());
  Serial.print("\t");
  Serial.print(taskStatus);
  Serial.print("\t");
  Serial.print(targSpeed1);
  Serial.print("\t");
  Serial.print(targSpeed2);
  Serial.print("\t");
  Serial.print(speed1);
  Serial.print("\t");
  Serial.print(speed2);
  Serial.print("\t");
  Serial.print(targTicks1);
  Serial.print("\t");
  Serial.print(doneTicks1);
  Serial.print("\t");
  Serial.print(targTicks2);
  Serial.print("\t");
  Serial.print(doneTicks2);
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

unsigned long prevTask = 0;

void loop(){

  readEnc();
  checkTask();
  calcSpeed();
  trace();
  motors(power1,power2);

  if(millis()-prevTask>10000){
    startTask(-100,100);
    prevTask=millis();
  }
  delay(10);
}
