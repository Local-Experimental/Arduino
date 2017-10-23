#include <stdlib.h>
#include <stddef.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include "functions.h"

//0x3F need to find out with :LINK:http://playground.arduino.cc/Main/I2cScanner
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6,7,3, POSITIVE );

const static  int pusher = 10;
const static  int wind = 11;

//b1(12) set, b2(13) down; b3(A0) up
//buttons
const static  int8_t BUTTON_SET = 12;
const static  int8_t BUTTON_DOWN = 13;
const static  int8_t BUTTON_UP = A0;
////////////////////
//Program Variables

controls MAXTEMP;
controls FANSPEED;
controls PELLETPUSHER;


void init_memory () 
  {
      
        MAXTEMP.address = 0;
        
        //
        FANSPEED.address = 1;
     
        //
        PELLETPUSHER.address = 3;
        
  
  };


void setup() {
  //MAXTEMP.address = 0;
  
  lcd.begin(16,2);
  init_memory (); // initiliaze objective arguments from the local scope 
  lcd.blink();
  
  // put your setup code here, to run once:
  pinMode(pusher,OUTPUT);
  pinMode(wind,OUTPUT);
  pinMode (BUTTON_SET,INPUT);
  pinMode (BUTTON_DOWN,INPUT);
  pinMode (BUTTON_UP,INPUT);
   Serial.begin(9600);
   Serial.println ("Load Complete");
   delay(500);
   
  Serial.print ("EEPROM:" + String (readMemory(0) ) + ", ");
  delay (2000);
}

bool buttonRelease (int btn ) {
       bool pressed = false; 
         while (digitalRead(btn) > 0)
         {
              pressed = true;
              delay(100);
              Serial.println("Button Pressed: "+ String(btn));
          }
             if (pressed) {
                 Serial.println("Button Released");
                 pinMode (13,OUTPUT);
                  delay(100);
                  digitalWrite(13,HIGH);
                  delay(100);
                  digitalWrite(13,LOW);
                 pinMode (13,INPUT);
             }
        return pressed;
  }



void bln (int times=1) {
      
      for (int i=0; i <= times; i++){
      digitalWrite(13,HIGH);
      delay(500);
      digitalWrite(13,LOW);
      delay(500);
     
   }   
  }


int CLK_TIME=0;


void loop() {
  if (CLK_TIME < 10) CLK_TIME++; else CLK_TIME=0; // program one second timer
  int8_t __set = digitalRead(BUTTON_SET) ;
  int8_t __up = digitalRead(BUTTON_UP);
  int8_t __down = digitalRead(BUTTON_DOWN);


     Serial.println("__set:"+ String(__set) + ",__up:"+ String(__up) + ",__down:" + String(__down)  );

  if (CLK_TIME ==10) {} // slow components
  {
         lcd.clear();
         lcd.print("MEMORY:" + String (FANSPEED.value));
    }
  
//  buttonRelease (BUTTON_SET );
//  buttonRelease (BUTTON_UP );
//  buttonRelease (BUTTON_DOWN );
     
  if (__up) {
        FANSPEED.addValue();
    }

    if (__down) {
        FANSPEED.subValue();
    }
  

          if (__set) {
              FANSPEED.setValue();
            }

 delay (100);
   
  
  // put your main code here, to run repeatedly:
//delay (500);
//analogWrite(wind, 0);
//analogWrite(pusher, 0);
//bln ();
//
//  for (int i=0; i <= 255; i++){
//      
//      analogWrite(pusher,i);
//      delay(50);
//   } 
//
//  delay (5000); 
//digitalWrite (13,HIGH);
//analogWrite(wind, 0);
//analogWrite(pusher, 0);
//delay (5000);
//
//digitalWrite (13,LOW);
// bln(2);  
// for (int i=0; i <= 255; i++){
//      analogWrite(wind, i);
//      
//      delay(50);
//   }   
//
//delay (5000);
//digitalWrite (13,HIGH);
//analogWrite(wind, 0);
//analogWrite(pusher, 0);
//delay (5000);

// bln(3);  
// for (int i=0; i <= 255; i++){
//      analogWrite(wind, i);
//       delay(50);
//      analogWrite(pusher, i);
//      delay(50);
//   }   
//delay (5000);
//digitalWrite (13,HIGH);
//analogWrite(wind, 0);
//analogWrite(pusher, 0);
//delay (5000);



}

