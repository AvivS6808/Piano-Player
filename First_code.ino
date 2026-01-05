#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm2 = Adafruit_PWMServoDriver(0x43);
Adafruit_PWMServoDriver pwm3 = Adafruit_PWMServoDriver(0x42);
Adafruit_PWMServoDriver pwm4 = Adafruit_PWMServoDriver(0x41);
Adafruit_PWMServoDriver pwm5 = Adafruit_PWMServoDriver(0x40);

void setup() {
  
  Serial.begin(115200);

  pwm2.begin();
  pwm2.setPWMFreq(1000);

  pwm3.begin();
  pwm3.setPWMFreq(1000);
 
  pwm4.begin();
  pwm4.setPWMFreq(1000);

  pwm5.begin();
  pwm5.setPWMFreq(1000);
  //pwm5.setPin(11, 4000);
  //triggerNote(37,100);
  
  /*int i = 36;
  while (i <= 83){
    triggerNote(i,);
    delay(25);
    triggerNote(i,0);
    i = i+1;
  
  }*/   
  
}



void loop() {
  static String input = "";

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      int colonIndex = input.indexOf(':');
      int note = input.substring(0, colonIndex).toInt();
      int velocity = input.substring(colonIndex + 1).toInt();

      triggerNote(note, velocity);
      input = "";
    } else {
      input += c;
    }
  }
}


void triggerNote(int note, int velocity) {
  if (note >= 36 && note <= 47) {
    int channel = note - 36;
    pwm2.setPin(channel, velocity);
  } else if (note >= 48 && note <= 59) {
    int channel = note - 48;
    pwm3.setPin(channel, velocity);
  } else if (note >=60 && note <=71){
    int channel = note - 60;
    pwm4.setPin(channel, velocity);
  } else if (note >=72 && note <=83){
    int channel = note - 72;
    pwm5.setPin(channel, velocity);
  }
}
