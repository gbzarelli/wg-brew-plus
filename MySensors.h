#include <max6675.h>
#include <PID_v1.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define PIN_RESISTENCE 12

const int SENSOR_MAX6675=1;
const int SENSOR_LM35=2;
const int SENSOR_DS18B20=3;
const int SENSOR_DEBUG=4;

const int TYPE_SENSOR = SENSOR_DS18B20;

//temperatura de chaveamento em Cº
const int SWITCH_TEMP_VARIABLE_NEGATIVE = 1;//temperatura de troca para ligar/desligar para temperatura negativa;
const int SWITCH_TEMP_VARIABLE_VALIDATION = 2;//range de temperatura utilizado na validação.

const int TIME_REFRESH_PID = 1000;//TEMpo para atualizacao do PID (em millis)
const int TIME_REFRESH_TEMP = 1000;//Tempo para atualizacao do sensor de temperatura. Evitar lentidao nas interações.
const int VALUE_TURN_ON_RESISTENCE_PID = 75;//Valor de acionamento resistencia (pid retorna de 0-255)

const int PIN_DS18B20=A5;
const int PIN_LM35_0 = A5;
const int PIN_LM35_1 = A4;
const int PIN_THERMO_DO=A4;
const int PIN_THERMO_CS=A3;
const int PIN_THERMO_CLK=A2;
const int PIN_READ_DEBUG = A4;

MAX6675 thermocouple(PIN_THERMO_CLK, PIN_THERMO_CS, PIN_THERMO_DO);

OneWire oneWire_in(PIN_DS18B20);
DallasTemperature sensors(&oneWire_in);
DeviceAddress insideThermometer;
long myTimeUpdateSensor;//Variavel para controlar update de leitura do sensor dalas

void setupSensors();
void loopSensors();
void refreshResistence(int tempDesired);
void refreshResistencePID(int tempDesired);
void refreshResistenceRefri(int tempDesired);
double getThermoC();
void turnOnResistence();
void turnOffResistence();
double getThermoLM35();
double getThremoMax6675();
double getThremoDs18B20(DeviceAddress deviceAddress);
void controllPID();
void printAddress(DeviceAddress deviceAddress);
boolean inTemperature(int temperature);

void setupSensors() {
  // Configuração de pinos arduino
  pinMode(PIN_RESISTENCE, OUTPUT);

  if(TYPE_SENSOR == SENSOR_DEBUG){
    pinMode(PIN_READ_DEBUG,INPUT);
  }else{
    Serial.print("Locating devices...");
    sensors.begin();
    Serial.print("Found ");
    Serial.print(sensors.getDeviceCount(), DEC);
    Serial.println(" devices.");
    Serial.print("Parasite power is: "); 
    if (sensors.isParasitePowerMode()) Serial.println("ON");
    else Serial.println("OFF");
    if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
    Serial.print("Device 0 Address: ");
    printAddress(insideThermometer);
    Serial.println();
    sensors.setResolution(insideThermometer, 9);
    Serial.print("Device 0 Resolution: ");
    Serial.print(sensors.getResolution(insideThermometer), DEC); 
    Serial.println();
  }
  
  turnOffResistence();
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

//*************************************************************************************
//**PID CONTROLL************************************************************************
//*************************************************************************************
boolean startPID=false;

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
double Kp=20, Ki=0, Kd=0;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
long myTimePID=0;
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************

void loopSensors(){
  if(startPID && (millis()-myTimePID)>TIME_REFRESH_PID){
      myTimePID=millis();
      controllPID();
  }
}

void controllPID(){
  Input = getThermoC();
  myPID.Compute();
  Serial.print("PID CONTROL VALUE: ");
  Serial.println(Output);
  if (Output >= VALUE_TURN_ON_RESISTENCE_PID){
    turnOnResistence();
  }else{
    turnOffResistence();
  }
  
}

/**
    Verifica se a temperatura do sensor esta adequada a desejada e
    realiza o chaveamento necessario para adequar utilizando controle PID.
    Método async.
*/
void refreshResistencePID(int tempDesired){
  Setpoint = tempDesired;
  if(!startPID){
    Input = getThermoC();
    myPID.SetMode(AUTOMATIC);
    startPID=true;
  }
}

/**
  Para com o processo de controle de temperatura PID
*/
void turnOffResistencePID(){
  turnOffResistence();
  startPID=false;
  Setpoint=0;
}

/**
    Verifica se a temperatura do sensor esta adequada a desejada e
    realiza o chaveamento necessario para adequar.
*/
void refreshResistence(int tempDesired) {
  tempDesired = getThermoC() - tempDesired;
  if (tempDesired <= 0) {
    turnOnResistence();
  } else{
    turnOffResistence();
  }
}

/**
    Verifica se a temperatura do sensor esta adequada a desejada e
    realiza o chaveamento necessario para adequar.
*/
void refreshResistenceRefri(int tempDesired) {
  tempDesired = getThermoC() - (tempDesired - SWITCH_TEMP_VARIABLE_NEGATIVE);
  if (tempDesired > 0) {
    turnOnResistence();
  } else{
    turnOffResistence();
  }
}

/**
 * Liga a resistencia
 */
void turnOnResistence(){
  digitalWrite(PIN_RESISTENCE, HIGH);
}

/**
 * Desliga a resistencia
 */
void turnOffResistence(){
  digitalWrite(PIN_RESISTENCE, LOW);
}
//40 -37
boolean inTemperature(int temperature){
  int readTemp = getThermoC();
  if(readTemp >= (temperature-SWITCH_TEMP_VARIABLE_VALIDATION) && readTemp <= (temperature + SWITCH_TEMP_VARIABLE_VALIDATION)){
    Serial.print("VALIDADO: ");Serial.print(temperature);Serial.print(" THERMO: ");Serial.println(readTemp);
    return true;
  }
  return false;
}

/**
   Retorno da temperatura do sensor.
*/
double getThermoC() {
  if(TYPE_SENSOR==SENSOR_MAX6675){
    return getThremoMax6675();
  }else if(TYPE_SENSOR==SENSOR_LM35){
    return getThermoLM35();
  }else if(TYPE_SENSOR==SENSOR_DEBUG){
    return map(analogRead(PIN_THERMO_CS),0,1023,-20,200);
  }else if(TYPE_SENSOR==SENSOR_DS18B20){
    return getThremoDs18B20(insideThermometer);
  }
  return -100;
}

/**
  Pega a temperatura do sensor MAX 6675
*/
double getThremoMax6675(){
  return thermocouple.readCelsius();
}

/**
  Pega a temperatura do sensor SENSOR_DS 18B20
*/
double getThremoDs18B20(DeviceAddress deviceAddress){
  if((millis()-myTimeUpdateSensor)>TIME_REFRESH_TEMP){
    myTimeUpdateSensor=millis();
    sensors.requestTemperatures(); // Send the command to get temperatures
  }
  return sensors.getTempC(deviceAddress);
}


/**
  Pega a temperatura do sensor LM35
*/
double getThermoLM35(){
  double temp = analogRead(PIN_LM35_0) - analogRead(PIN_LM35_1);
  return (500*temp)/1023;
}

