/**********************************************************
 * Eduard Petrenko
 * 2021-08
 * робототехническая смена Физтех-Лицей
 * Игра в футбол по bluetooth

**********************************************************/ 

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
//********************************************************************

unsigned long lastTime=0;

 
#define STOP_WHEN_KICK

#define SPEED_RANGES 10
float MS_MIN[SPEED_RANGES] = {0, 0.15, 0.20, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
float MS_MAX[SPEED_RANGES] = {0, 0.20, 0.35, 0.5, 0.7, 0.8, 0.9, 1.0, 1.0, 1.0};
#define TS_MIN 0.2
#define TS_MAX 0.7
#define DS_MIN 0.1
#define DS_MAX 0.3

int speedRange = 4;

#define MAX_SPEED_TIME 2000

#define KICK_PIN 30
unsigned long prevKick=0;
void kick(){
  if(millis()-prevKick>500){
    digitalWrite(KICK_PIN,HIGH);
    delay(300);
    digitalWrite(KICK_PIN,LOW);
    prevKick=millis();
  }
}

void kick2(){
  if(millis()-prevKick>500){
    motors(1.0,1.0);
    delay(250);
    motors(0,0);
    delay(100);
    motors(-1.0,-1.0);
    delay(250);
    motors(0,0);
    prevKick=millis();
  }
}

  
void setup()
{
  Serial3.begin(9600);
  setupMotors();
  pinMode(KICK_PIN,OUTPUT);
//  testDrive();
//  kick();
}

int prevCmd=0;
unsigned long cmdTime=0;
void checkCmd(int _cmd){
  if((_cmd!=prevCmd) && ((_cmd <'0') || (_cmd>'9'))){
    cmdTime=millis();
    prevCmd=_cmd;
  }
}
  
void loop(){
  if(Serial3.available()){
    lastTime=millis();
    int inByte=Serial3.read(); 
//    Serial.write(inByte);
     checkCmd(inByte);
     switch (inByte){
      case 'S':
        motors(0,0);
        break;
      case 'F':
        motors(constrain(MS_MIN[speedRange]+(MS_MAX[speedRange]-MS_MIN[speedRange])*(millis()-cmdTime)/MAX_SPEED_TIME,MS_MIN[speedRange],MS_MAX[speedRange]),constrain(MS_MIN[speedRange]+(MS_MAX[speedRange]-MS_MIN[speedRange])*(millis()-cmdTime)/MAX_SPEED_TIME,MS_MIN[speedRange],MS_MAX[speedRange]));
        break;
      case 'B':
        motors(-constrain(MS_MIN[speedRange]+(MS_MAX[speedRange]-MS_MIN[speedRange])*(millis()-cmdTime)/MAX_SPEED_TIME,MS_MIN[speedRange],MS_MAX[speedRange]),-constrain(MS_MIN[speedRange]+(MS_MAX[speedRange]-MS_MIN[speedRange])*(millis()-cmdTime)/MAX_SPEED_TIME,MS_MIN[speedRange],MS_MAX[speedRange]));
        break;
      case 'R':
        motors(-constrain(TS_MIN+(TS_MAX-TS_MIN)*(millis()-cmdTime)/MAX_SPEED_TIME,TS_MIN,TS_MAX),constrain(TS_MIN+(TS_MAX-TS_MIN)*(millis()-cmdTime)/MAX_SPEED_TIME,TS_MIN,TS_MAX));
        break;
      case 'L':
        motors(constrain(TS_MIN+(TS_MAX-TS_MIN)*(millis()-cmdTime)/MAX_SPEED_TIME,TS_MIN,TS_MAX),-constrain(TS_MIN+(TS_MAX-TS_MIN)*(millis()-cmdTime)/MAX_SPEED_TIME,TS_MIN,TS_MAX));
        break;
      case 'I':
        motors(constrain(MS_MIN[speedRange]+(MS_MAX[speedRange]-MS_MIN[speedRange])*(millis()-cmdTime)/MAX_SPEED_TIME,MS_MIN[speedRange],MS_MAX[speedRange])-constrain(DS_MIN+(DS_MAX-DS_MIN)*(millis()-cmdTime)/MAX_SPEED_TIME,DS_MIN,MS_MAX[speedRange]),
               constrain(MS_MIN[speedRange]+(MS_MAX[speedRange]-MS_MIN[speedRange])*(millis()-cmdTime)/MAX_SPEED_TIME,MS_MIN[speedRange],MS_MAX[speedRange])+constrain(DS_MIN+(DS_MAX-DS_MIN)*(millis()-cmdTime)/MAX_SPEED_TIME,DS_MIN,MS_MAX[speedRange]));
        break;
      case 'G':
        motors(constrain(MS_MIN[speedRange]+(MS_MAX[speedRange]-MS_MIN[speedRange])*(millis()-cmdTime)/MAX_SPEED_TIME,MS_MIN[speedRange],MS_MAX[speedRange])+constrain(DS_MIN+(DS_MAX-DS_MIN)*(millis()-cmdTime)/MAX_SPEED_TIME,DS_MIN,MS_MAX[speedRange]),
               constrain(MS_MIN[speedRange]+(MS_MAX[speedRange]-MS_MIN[speedRange])*(millis()-cmdTime)/MAX_SPEED_TIME,MS_MIN[speedRange],MS_MAX[speedRange])-constrain(DS_MIN+(DS_MAX-DS_MIN)*(millis()-cmdTime)/MAX_SPEED_TIME,DS_MIN,MS_MAX[speedRange]));
        break;
      case 'H':
        motors(-constrain(MS_MIN[speedRange]+(MS_MAX[speedRange]-MS_MIN[speedRange])*(millis()-cmdTime)/MAX_SPEED_TIME,MS_MIN[speedRange],MS_MAX[speedRange])+constrain(DS_MIN+(DS_MAX-DS_MIN)*(millis()-cmdTime)/MAX_SPEED_TIME,DS_MIN,MS_MAX[speedRange]),
               -constrain(MS_MIN[speedRange]+(MS_MAX[speedRange]-MS_MIN[speedRange])*(millis()-cmdTime)/MAX_SPEED_TIME,MS_MIN[speedRange],MS_MAX[speedRange])-constrain(DS_MIN+(DS_MAX-DS_MIN)*(millis()-cmdTime)/MAX_SPEED_TIME,DS_MIN,MS_MAX[speedRange]));
        break;
      case 'J':
        motors(-constrain(MS_MIN[speedRange]+(MS_MAX[speedRange]-MS_MIN[speedRange])*(millis()-cmdTime)/MAX_SPEED_TIME,MS_MIN[speedRange],MS_MAX[speedRange])-constrain(DS_MIN+(DS_MAX-DS_MIN)*(millis()-cmdTime)/MAX_SPEED_TIME,DS_MIN,MS_MAX[speedRange]),
               -constrain(MS_MIN[speedRange]+(MS_MAX[speedRange]-MS_MIN[speedRange])*(millis()-cmdTime)/MAX_SPEED_TIME,MS_MIN[speedRange],MS_MAX[speedRange])+constrain(DS_MIN+(DS_MAX-DS_MIN)*(millis()-cmdTime)/MAX_SPEED_TIME,DS_MIN,MS_MAX[speedRange]));
        break;
      case 'W':
        kick();
        break;
      case 'w': 
        kick();
        break;
      case 'x':
        kick2();
        break;
      case 'X': 
        kick2();
        break;
      case '0':
        speedRange=0;
        break;
      case '1':
        speedRange=1;
        break;
      case '2':
        speedRange=2;
        break;
      case '3':
        speedRange=3;
        break;
      case '4':
        speedRange=4;
        break;
      case '5':
        speedRange=5;
        break;
      case '6':
        speedRange=6;
        break;
      case '7':
        speedRange=7;
        break;
      case '8':
        speedRange=8;
        break;
      case '9':
        speedRange=9;
        break;
    }
  }
  if(millis()-lastTime>5000){
    motors(0,0);
  }
//  delay(100);
}
