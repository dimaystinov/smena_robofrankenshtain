/********************************************
 * Eduard Petrenko
 * 2021-08
 * робототехническая смена в Физтех-Лицее
 * тест класса для JSON коммуникаций от "головы" до Arduino
 
 от openmv
коэффициенты в ардуино
{
    type:"move, rotate";
    value: "mm, degrees";
}

от ардуинки
{
    status:"0 : ok, -1 : error (пришла комана, а предыдущая ещё не выполнена) ";
}

defined error codes
-1 пришла следующая команда, выполнение предыдущей не завершено
-2 ошибка парсинга команды
-3 недопустимый тип команды

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


CommandJSON cmd;

void setup() {
  // инциируем коммуникацию
  Serial.begin(9600);
  // test cmd parse
  Serial.println("parse {\"type\":\"move\",\"value\":-300}");
  Serial.println(cmd.parseJSON("{\"type\":\"move\",\"value\":-300}"));
  Serial.println(cmd.getType());
  Serial.println(cmd.getValue());
  Serial.println("parse {\"type\":\"hello\",\"value\":-300}");
  Serial.println(cmd.parseJSON("{\"type\":\"hello\",\"value\":-300}"));
  Serial.println(cmd.getType());
  Serial.println(cmd.getValue());
  Serial.println("parse m-50hello");
  Serial.println(cmd.parseSimple("m-50hello"));
  Serial.println(cmd.getType());
  Serial.println(cmd.getValue());
}

void loop() {
  // читаем из Serial и парсим команды. В ответ отправляем статус
  char inBuff[BUFF_LEN];
  for(int i=0; i<BUFF_LEN; i++) inBuff[i]='\0';
  Serial.println("=========================");
  if(Serial.readBytesUntil('\n',inBuff,BUFF_LEN-1)>0){
    CommandJSON inCmd;
    Serial.print("parse ");
    Serial.println(inBuff);
    int retCode = cmd.parseJSON(inBuff);
    CommandJSON outCmd;
    Serial.println(retCode);
    Serial.println(cmd.getType());
    Serial.println(cmd.getValue());
    outCmd.setType('s');
    outCmd.setValue(retCode);
    Serial.println(outCmd.serializeJSON());
  }
  delay(5000);
}
