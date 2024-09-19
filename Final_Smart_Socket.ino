//SMART SOCKET
//H2AL
//Version-3
//ADDED byte4,5 
//Made covert to celsius and convert to sec fnctions

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include <OneWire.h>

uint8_t Incomming_byte1=0;//byte1 stores info about switch case number
uint8_t Incomming_byte2=0;//byte2 stores value from text box(ex-temp,count value)
uint8_t Incomming_byte3=0;//byte3 stores the unit
                          //(0-none)
                          //1-sec
                          //2-min
                          //3-hrs
                          //4-C
                          //5-F
uint8_t Incomming_byte4=0;//byte4 stores value from text box2(ex-temp value only )
uint8_t Incomming_byte5=0;////byte5 stores the unit
                          //4-C
                          //5-F

uint8_t Buzzerpin=4;
uint8_t SmartSocket=8;
uint8_t TempSensorPin = 9;

LiquidCrystal_I2C lcd(0x27,16,2);

OneWire oneWire(TempSensorPin);
DallasTemperature sensors(&oneWire);

void setup() 
{
  Serial.begin(9600);
  pinMode(Buzzerpin,OUTPUT);
  pinMode(SmartSocket,OUTPUT);
  digitalWrite(SmartSocket,HIGH);
  lcd.init();
  lcd.setBacklight(30);
  lcd.setCursor(0, 0);
  lcd.print("  Smart Socket");
  for(int i=0;i<3;i++)
  {
    digitalWrite(Buzzerpin,HIGH);
    delay(100);
    digitalWrite(Buzzerpin,LOW);
    delay(100);

  }
}

int Convert_to_Seconds(uint8_t TimeUnModified,uint8_t TimeMode){
  int TimeInSec=0;
  if(TimeMode==1)
  {
    TimeInSec=TimeUnModified;
  }
  else if(TimeMode==2)
  {
    TimeInSec=TimeUnModified*60;
  }
  else if(TimeMode==3)
  {
    TimeInSec=TimeUnModified*60*60;
  }
  else
  {
    TimeInSec=0;
    Serial.println("Invalid time");
  }
  return TimeInSec;
}

uint8_t Convert_to_Celsius(uint8_t Threshhold_temp,uint8_t TempMode)
{
  Serial.println("In Convert_to_Celsius mode");
  uint8_t Mode_Threshhold_temp=0;
  if(TempMode==4)
  {
    Mode_Threshhold_temp=Threshhold_temp;
  }
  else if(TempMode==5)
  {
    Mode_Threshhold_temp=(Threshhold_temp*(9/5))+32;
  }
  else
  {
    Mode_Threshhold_temp=0;
    Serial.println("Invalid unit");
  }
  return Mode_Threshhold_temp;
}

void lock_timer(uint8_t time_value,uint8_t time_mode,uint8_t temp_value,uint8_t temp_mode)
{
   Serial.println("i am in lock_counter");
   int time_in_sec=0;
   uint8_t temp_in_C=0;
   time_in_sec=Convert_to_Seconds(time_value,time_mode);//Buffer time counter
   temp_in_C=Convert_to_Celsius(temp_value,temp_mode);
  for(int i=0;i<time_in_sec;i++)
  {
    if(Serial.available()>0)
    {
      break;
    }
    else
    {
      if(Read_temp() >= temp_in_C)
      {
        Mannual_mode(2);//turning off
      }
      else
      {
        Mannual_mode(1);//turning on to reheat
      }
      delay(1000);
    }
  }
  Mannual_mode(2);
  fastbuzzer();
}

void Sensor_Lock_Mode(uint8_t time_value,uint8_t time_mode,uint8_t temp_value,uint8_t temp_mode)
{
  Sensor_Mode(temp_value,temp_mode);//first step acc to flow chart(pre heating stage)
  lock_timer(time_value,time_mode,temp_value,temp_mode);//entering the lock mode

}

void fastbuzzer()
{
  //if task completed ring 3 times
  Serial.println("I am in fastbuzzzer");
  for(int j=0;j<3;j++)
  {
    for(int i=0;i<3;i++)
    {
      digitalWrite(Buzzerpin,HIGH);
      delay(50);
      //Serial.println(i);
      digitalWrite(Buzzerpin,LOW);
      delay(50);
    }
  delay(1000);
  }
  Serial.println("Buzzer OFF");
}

void buzzer(){
  //if task completed ring 3 times
  Serial.println("I am in buzzzer");
  for(int i=0;i<3;i++){
  digitalWrite(Buzzerpin,HIGH);
  delay(1000);
  //Serial.println(i);
  digitalWrite(Buzzerpin,LOW);
  delay(1000);
  }
  Serial.println("Buzzer OFF");
}

int timer(uint8_t TimeUnModified,uint8_t TimeMode){
  int TimeInSec=0;
  TimeInSec=Convert_to_Seconds(TimeUnModified,TimeMode);
  //take count value and trigger after counting
  
  if( TimeInSec==0){
    Serial.println("Invalid time");
     Mannual_mode(2); 
  }
  else{
    Serial.println("TIMER STARTED");
    countdown(TimeInSec);
    Serial.println("TIME UP");
    if(Serial.available()<=0){
     Mannual_mode(2); 
      buzzer();
    }
    
  }
  
}

void countdown(int Countdown_value){
  Serial.println("i am in counter");
  for(int i=Countdown_value;i>0;i--){
    if(Serial.available()>0)
    {
      break;
    }
    else
    { 
      
      while (Countdown_value > 0) 
      {
      if(Serial.available() > 0)
      {
        Mannual_mode(2);
        break;
      }
      int remainingHours = Countdown_value / 3600;
      int remainingMinutes = (Countdown_value % 3600) / 60;
      int remainingSeconds = Countdown_value % 60;

      if(remainingSeconds<10)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Timer Mode   ON");
      }
      lcd.setCursor(0, 1);
      lcd.print(String(remainingHours) + "h " + String(remainingMinutes) + "m " + String(remainingSeconds) + "s");

      
      Countdown_value--; // Decrement the total time
      delay(1000); // Delay for 1 second
      }
      lcd.setCursor(0,1);
      lcd.print("0h 0m 0s");

    }
  }
}

int Read_temp()
{
  Serial.println("Reading temperature in celcius");
  //read temperature from dallas
  sensors.requestTemperatures();
  //dallas code here
  int celsius = sensors.getTempCByIndex(0);
  Serial.println(celsius);

  lcd.setCursor(15,1);
  lcd.print("C");
  if(celsius == -127)
  {

  }
  if(celsius<100){
  //lcd.setCursor(12, 1);
  //lcd.print("0");
  lcd.setCursor(13, 1);
  lcd.print(celsius);
  }
  if(celsius == 100){
  lcd.setCursor(12, 1);
  lcd.print(celsius);
  }

  //return the temp value always in celcius 
  return celsius;
}

void Sensor_Mode(uint8_t Threshhold_temp,uint8_t TempMode){
  Serial.println("In sensor mode");
  uint8_t Mode_Threshhold_temp=0;
  Mode_Threshhold_temp=Convert_to_Celsius(Threshhold_temp,TempMode);

  while(1){
    int Current_temp=Read_temp();
    if(Serial.available()>0){
      Mannual_mode(2);
      break;
    }
    else if(Mode_Threshhold_temp==0){
      Mannual_mode(2);
      //buzzer();
      break;
    }
    else if(Current_temp>=Mode_Threshhold_temp){
      Mannual_mode(2);
      buzzer();
      break;
    }
    else{
      Mannual_mode(1);
    }
  }
}

void Mannual_mode(uint8_t on_off){
Serial.println("In manual mode");
  Serial.println(on_off);
  if(on_off==1){
    lcd.setCursor(13, 0);
    lcd.print("ON ");
    Serial.println("Socket On");
    digitalWrite(SmartSocket,LOW);
  }
  else if(on_off==2){
    lcd.setCursor(13,0);
    lcd.print("OFF");
    Serial.println("Socket OFF");
    digitalWrite(SmartSocket,HIGH);
  }
  else{
    Serial.println("Invalid On_off value");
  }
}


void loop()
{
  //timer(10);
  //Sensor_Mode(10);
  //Mannual_mode(1);
  //while(1){;}}
  

  if(Serial.available()>0){
    Incomming_byte1=Serial.read();
    Serial.println(Incomming_byte1);
    delay(50);
    if(Serial.available()>0){
      Incomming_byte2=Serial.read();
      Serial.println(Incomming_byte2);
      delay(50);
        if(Serial.available()>0){
          Incomming_byte3=Serial.read();
          Serial.println(Incomming_byte3);
          delay(50);
          if(Serial.available()>0){
            Incomming_byte4=Serial.read();
            Serial.println(Incomming_byte4);
            delay(50);
            if(Serial.available()>0){
              Incomming_byte5=Serial.read();
              Serial.println(Incomming_byte5);
              delay(50);
            }
          }
        }
    }
    
    
    switch(Incomming_byte1)
    {
      case 1: //mannual mode ON
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Manual Mode");
        Serial.println("In case1");
        Mannual_mode(1);
        break;

      case 2://mannual mode OFF   
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Manual Mode"); 
        Serial.println("In case2");
        Mannual_mode(2);
        delay(1000);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("  Smart Socket");
        break;

      case 3://timer mode
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Timer Mode");
        Serial.println("In case3");
        Mannual_mode(1);
        delay(500);
        timer(Incomming_byte2,Incomming_byte3);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("  Smart Socket");
        break;

      case 4://sensor mode
        lcd.clear();
       lcd.setCursor(0,0);
        lcd.print("Sensor Mode");
        Serial.println("In case4");
        Mannual_mode(1);
        delay(500);
        Sensor_Mode(Incomming_byte2,Incomming_byte3);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("  Smart Socket");
        delay(100);
        break;

      case 5://temperature lock
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Temp Lock");
        Serial.println("In case5");
        Mannual_mode(1);
        delay(100);
        Sensor_Lock_Mode(Incomming_byte2,Incomming_byte3,Incomming_byte4,Incomming_byte5);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("  Smart Socket");
        delay(100);
        break;

      default:
        Serial.println("found in default");
        Mannual_mode(2);
        break;

      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("  Smart Socket");
    }
    
  }
  
}

