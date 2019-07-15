#include <DS3231.h>
#include <Wire.h>
#include <EEPROM.h>
DS3231 Clock;
#define BUFFSIZE 50
#define PRZEKAZNIK 9
#define LED A3
#define LED2 A1
#define eepron 0 //adres eeprom
#define eeproff 4 //adres eeprom przesuniety o 4 bajty na eepron
#define BUTTON 5
byte Hour;
byte Minute;
byte Second;
byte Htmp;
byte Mtmp;
byte Stmp;
int cmpoff;
int cmpon;
enum menu { defa, ustawczas, wprowadzczas, ustawczasoff, ustawczason, podajczas, wylacz, wlacz};
enum menu men;
char input[BUFFSIZE];
bool Century = false;
bool h12;
bool PM;
byte j = 0; //licznik na ilosc bajtow w buforze i do dlugosci napisu
int buttonState = 0;
bool Relstate = false;
boolean GotString = false;
volatile bool interrtohandle = false; //flaga do obslugi przerwania
void RelOn(void){
  digitalWrite(PRZEKAZNIK, HIGH);
  digitalWrite(LED, HIGH);
  Serial.println("przekaznik ON");
  Relstate = true;
}
void RelOff(void){
  digitalWrite(PRZEKAZNIK, LOW);
  digitalWrite(LED, LOW);
  Serial.println("przekaznik OFF");
  Relstate = false;
}
void PrintBuff(char arr[]){
  for(int i = 0; i < BUFFSIZE; i++){
    Serial.print(arr[i]);
    if(arr[i] == '\n') return;
  }
}
void PrintTime(DS3231 Clock){
        Serial.print(Clock.getYear(), DEC);
        Serial.print("-");
        Serial.print(Clock.getMonth(Century), DEC);
        Serial.print("-");
        Serial.print(Clock.getDate(), DEC);
        Serial.print(" ");
        Serial.print(Clock.getHour(h12, PM), DEC); //24-hr
        Serial.print(":");
        Serial.print(Clock.getMinute(), DEC);
        Serial.print(":");
        Serial.println(Clock.getSecond(), DEC);
}
bool ProccessInput(char arr[], byte &Hour, byte &Minute, byte &Second){
  byte tmp1;
  byte tmp2;
  if(input[j-1] == ';' && j == 8){
    return false;
  }
  tmp1 = (byte)arr[0] - 48;
  tmp2 = (byte)arr[1] - 48;
  Hour = tmp1*10 + tmp2;
 
  tmp1 = (byte)arr[3] - 48;
  tmp2 = (byte)arr[4] - 48;
  Minute = tmp1*10 + tmp2;
  
  tmp1 = (byte)arr[6] - 48;
  tmp2 = (byte)arr[7] - 48;
  Second = tmp1*10 + tmp2;
  return true;
}
void SetTime(DS3231 &Clock, byte Hour, byte Minute, byte Second){
  
  Clock.setClockMode(false);
  Clock.setHour(Hour);
  Clock.setMinute(Minute);
  Clock.setSecond(Second);
}
void GetTimeStuff(byte& Hour, byte& Minute, byte& Second) {
  Hour = Clock.getHour(h12, PM);
  Minute = Clock.getMinute();
  Second = Clock.getSecond();
}
void interr(void){
  interrtohandle = true;
}
void setup() {
  pinMode(BUTTON, INPUT);
  pinMode(PRZEKAZNIK, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED2, HIGH);
  pinMode(3, INPUT_PULLUP);
  Serial.begin(9600);
  Wire.begin();
  attachInterrupt(digitalPinToInterrupt(3), interr, RISING);
  Clock.enableOscillator(true, false, 0);

  Serial.println("Restart");

  //EEPROM

  Htmp = EEPROM.read( eepron);
  Mtmp = EEPROM.read( eepron + 1);
  Stmp = EEPROM.read( eepron + 2);
  cmpon = Htmp * 100 + Mtmp;

  Htmp = EEPROM.read( eeproff);
  Mtmp = EEPROM.read( eeproff + 1);
  Stmp = EEPROM.read( eeproff + 2);
  cmpoff = Htmp * 100 + Mtmp;


  GetTimeStuff( Hour, Minute, Second);
  int cmp = Hour * 100 + Minute;
  if(cmpon < cmp && cmp < cmpoff){
    RelOn();
  }else{
    RelOff();
  }
 
  // put your setup code here, to run once:

}

void loop() {
   buttonState = digitalRead(BUTTON);
  if (buttonState == HIGH) {
    Serial.println("button");
    if(Relstate == false){
     RelOn(); 
    }else{
      RelOff();
    }
    delay(200);
  }
  if(Serial.available()){
    if(j == BUFFSIZE-1){
      Serial.println("przepelnienie bufora");
      j = 0;
    }
    input[j] = Serial.read();
    if(input[j] == '\n'){
      input[j+1] = '\0';
      GotString = true;
      PrintBuff(input);
    }else j++; 
  }
  if(GotString == true){


    
    if(strcmp(input, "czas\n") == 0){
      PrintTime(Clock);
    }
    else if(strcmp(input, "wlacz\n") == 0){
      RelOn();
    }
    else if(strcmp(input, "wylacz\n") == 0){
      RelOff();
    }
    else if(strcmp(input, "ustawczas\n") == 0){
      men = ustawczas;
      Serial.println("Podaj czas w formacie GG:MM:SS;");
    }
    else if(men == ustawczas){
      men = defa;
      Serial.println("ustawiam czas");
      if(ProccessInput(input, Htmp, Mtmp, Stmp)){
        SetTime(Clock, Htmp, Mtmp, Stmp);
        PrintTime(Clock);
      }else{
        Serial.println("Nieprawidlowe dane wejsciowe");        
      }
    }
    else if(strcmp(input, "ustawczason\n") == 0){
      men = ustawczason;
      Serial.println("Podaj czas w formacie GG:MM:SS;");

    }
    else if(strcmp(input, "ustawczasoff\n") == 0){
      men = ustawczasoff;
      Serial.println("Podaj czas w formacie GG:MM:SS;");

    }
    else if(strcmp(input, "czason\n") == 0){
      Serial.print(cmpon);

    }
    else if(strcmp(input, "czasoff\n") == 0){
      Serial.print(cmpoff);

    }
    else if(men == ustawczason){

      Serial.println("ustawiam czasON");
      if(ProccessInput(input, Htmp, Mtmp, Stmp)){
        cmpon = Htmp*100 + Mtmp;
        EEPROM.write( eepron, Htmp);
        EEPROM.write( eepron + 1, Mtmp);
        EEPROM.write( eepron + 2, Stmp);
      }else{
        Serial.println("Nieprawidlowe dane wejsciowe");        
      }      
      men = defa;
    }
    else if(men == ustawczasoff){

      if(ProccessInput(input, Htmp, Mtmp, Stmp)){
  
        cmpoff = ((long)Htmp * 100) + Mtmp;
  
        EEPROM.write( eeproff, Htmp);
        EEPROM.write( eeproff + 1, Mtmp);
        EEPROM.write( eeproff + 2, Stmp);
      }else{
        Serial.println("Nieprawidlowe dane wejsciowe");        
      }        
      men = defa;
    }
    else{
      men = defa;
      //print komendy
      Serial.println("komendy:");
      Serial.println("\t ustawczas");
      Serial.println("\t ustawczason");
      Serial.println("\t ustawczasoff");
      Serial.println("\t czas");
      Serial.println("\t czason");
      Serial.println("\t czasoff");
      Serial.println("\t wlacz");
      Serial.println("\t wylacz");

    }
    //zerowanie flag
    GotString = false;
    j = 0;

  }
  if(interrtohandle == true){
    long cmp;
    //PrintTime(Clock);
    GetTimeStuff( Hour, Minute, Second);
    cmp = Hour * 100 + Minute;
    if(cmpon == cmp && cmp < cmpoff){
      RelOn();
    }
    if(cmpon < cmp && cmp == cmpoff){
      RelOff();
    }
    
    interrtohandle = false;
  }

}
