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
Принимает и исполняет JSON команды
Примеры команд:
{"type":"rotate","value":50}
{"type":"move","value":-150}
**********************************************************/ 
#define MYDEBUG

#define COMMAND_SERIAL Serial2

/*************************************************************************************************
 * JSON Command object
 */

#include <stdlib.h> 
#include <string.h>
#include <Arduino_JSON.h>

#define BUFF_LEN 50

class Command{
protected:
  char outBuff[BUFF_LEN];
  char cType; //'m'|'r'|'s'|'u' for move (straight) | rotate | status | undefined (if parse error)
  int value; // millimeters | degrees | status/error code
public:
  Command();
  char getType();
  int getValue();
  void setType(char _type);
  void setValue(int _value);
  int parseSimple(char *data); //returns '0' if successfull, '-2' or '-3' if parse error
  char *serializeSimple();
};

Command::Command(){
  cType = 'u';
  value = 0;
}

char Command::getType() {return cType;}

int Command::getValue() {return value;}

void Command::setType(char _type) {cType = _type;}

void Command::setValue(int _value) {value = _value;}

int Command::parseSimple(char *data){
  if((data[0]=='m')||(data[0]=='r')||(data[0]=='s')){
    cType = data[0];
  }
  else {
    cType = 'u';
    value = 0;
    return -3;
  }
  for(int i=1; data[i]!='\0';i++){
    if(!((data[i]>='0') && (data[i]<='9')) && (data[i]!='-')){
      cType = 'u';
      value = 0;
      return(-2);
    }
  }
  value = atoi(data+1);
  return 0;
}

char *Command::serializeSimple(){
  outBuff[0] = cType;
  itoa(value,outBuff+1,10);
  return outBuff;
}

class CommandJSON: public Command {
public:
  int parseJSON(char *data);  
  char *serializeJSON();
};

int CommandJSON::parseJSON(char *data){
  cType = 'u';
  value = 0;
  JSONVar obj = JSON.parse(data); 
  if(JSON.typeof(obj) == "undefined") {
    cType = 'u';
    value = 0;
    return -2;
  }
  if(obj.hasOwnProperty("type")) {
    JSONVar _type = obj["type"];
    if(_type==JSONVar("move")) cType='m';
    if(_type==JSONVar("rotate")) cType='r';
    if(_type==JSONVar("status")) cType='s';
    //if still undefined then type was incorrect
    if(cType=='u'){
      value = 0;
      return -3;
    }
  }
  else{
    cType = 'u';
    value = 0;
    return -2;
  }
  if(obj.hasOwnProperty("value")) {
    value = (int)obj["value"];
  }
  else{
    cType = 'u';
    value = 0;
    return -2;
  }
  return 0;
}

char *CommandJSON::serializeJSON(){
  JSONVar obj;
  if(cType=='m') obj["type"] = "move";
  if(cType=='r') obj["type"] = "rotate";
  if(cType=='s') obj["type"] = "status";
  if(cType=='u') obj["type"] = "undefined";
  obj["value"] = value;
  String str = JSONVar::stringify(obj);
  strlcpy(outBuff,str.c_str(),BUFF_LEN-1);
  return outBuff;
}

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
#define SPEED_ROTATE 150

void onTaskBegin(){
}

void onTaskEnd(){
  CommandJSON outCmd;
  outCmd.setType('s');
  outCmd.setValue(0);
  COMMAND_SERIAL.println(outCmd.serializeJSON());
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

#define WHEEL_DIAMETER 65 //диаметр колёс в мм
#define WHEELBASE 180 //колёсная база робота в мм

#define TICKS_PER_ROTATION 11 //число тиков на оборот вала двигателя
#define GEAR_RATIO 19 //обратное передаточное число (оборотов вала двигателя на оборот колеса)

#define BRAKING_DISTANCES_MOVE_R 15 //тормозной путь правого колеса при рекомендованной скорости (прямолинейное движение)
#define BRAKING_DISTANCES_MOVE_L 15 //тормозной путь левого колеса при рекомендованной скорости (прямолинейное движение)

#define BRAKING_DISTANCES_ROTATE_R 10 //тормозной путь правого колеса при рекомендованной скорости (вращение)
#define BRAKING_DISTANCES_ROTATE_L 10 //тормозной путь левого колеса при рекомендованной скорости (вращение)
 
void moveMM(int _distance){
  //считаем число оборотов для требуемого пути, умножаем на передаточное число и число тиков на оборот вала
  int ticksR = round(((float)_distance/(PI*WHEEL_DIAMETER))*TICKS_PER_ROTATION*GEAR_RATIO);
  int ticksL = round(((float)_distance/(PI*WHEEL_DIAMETER))*TICKS_PER_ROTATION*GEAR_RATIO);
  if(_distance>0){
    startTask(ticksR - BRAKING_DISTANCES_MOVE_R, ticksL - BRAKING_DISTANCES_MOVE_L);
  }
  else {
    startTask(ticksR + BRAKING_DISTANCES_MOVE_R, ticksL + BRAKING_DISTANCES_MOVE_L);
  }
}

void rotateDeg(int _angle) {
  float dist = PI*(float)WHEELBASE*abs(_angle)/360;
  if(_angle>=0){
    startTask(round(dist/(PI*WHEEL_DIAMETER)*TICKS_PER_ROTATION*GEAR_RATIO)-BRAKING_DISTANCES_ROTATE_R,
             -round(dist/(PI*WHEEL_DIAMETER)*TICKS_PER_ROTATION*GEAR_RATIO)+BRAKING_DISTANCES_ROTATE_L);
  }
  else {
    startTask(-round(dist/(PI*WHEEL_DIAMETER)*TICKS_PER_ROTATION*GEAR_RATIO)+BRAKING_DISTANCES_ROTATE_R,
               round(dist/(PI*WHEEL_DIAMETER)*TICKS_PER_ROTATION*GEAR_RATIO)-BRAKING_DISTANCES_ROTATE_L);
  }
}

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

unsigned long prevKick=0;
void kick(){
  if(millis()-prevKick>500){
    digitalWrite(PORT_KICK,HIGH);
    delay(300);
    digitalWrite(PORT_KICK,LOW);
    prevKick=millis();
  }
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

#ifdef MYDEBUG
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
#endif
  
}

 
/*************************************************************************************************
 * main loop
 */

void setup() {
  setupEnc();
  setupMotors();
  setupKicker();
#ifdef MYDEBUG  
  Serial.begin(9600);
#endif
  COMMAND_SERIAL.begin(9600);
//  testDrive();
}

void loop(){
  if(taskStatus==TASK_READY)
  {
    char inBuff[BUFF_LEN];
    for(int i=0; i<BUFF_LEN; i++) inBuff[i]='\0';
    if(COMMAND_SERIAL.readBytesUntil('\n',inBuff,BUFF_LEN-1)>0){
      CommandJSON inCmd;
#ifdef MYDEBUG  
      Serial.print("parse ");
      Serial.println(inBuff);
#endif
      int retCode = inCmd.parseJSON(inBuff);
      if(retCode<0){
        CommandJSON outCmd;
#ifdef MYDEBUG  
        Serial.println(retCode);
        Serial.println(inCmd.getType());
        Serial.println(inCmd.getValue());
#endif
        outCmd.setType('s');
        outCmd.setValue(retCode);
        COMMAND_SERIAL.println(outCmd.serializeJSON());
      }
      else{
        switch(inCmd.getType()){
          case 'm':
            moveMM(inCmd.getValue());
            break;
          case 'r':
            rotateDeg(inCmd.getValue());
            break;
          case 's':
            if (inCmd.getValue() == 7){
              kick();
              break;
            }
        }
      }
    }
  }
  readEnc();
  checkTask();
  calcSpeed();
  trace();
  motors(power1,power2);
  delay(10);
}
