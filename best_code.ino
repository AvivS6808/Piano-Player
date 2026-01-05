#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// PCA9685 Instances
Adafruit_PWMServoDriver pwm2 = Adafruit_PWMServoDriver(0x43);
Adafruit_PWMServoDriver pwm3 = Adafruit_PWMServoDriver(0x42);
Adafruit_PWMServoDriver pwm4 = Adafruit_PWMServoDriver(0x41);
Adafruit_PWMServoDriver pwm5 = Adafruit_PWMServoDriver(0x40);

// Constants
#define MIN_MIDI 36
#define NUM_KEYS 48
#define PWM_MAX 4095
#define HOLD_PWM 2000
#define SERIAL_BUFFER 64


// Strike State Struct
struct Strike {
  bool active;              //note should be on or off
  uint8_t phase;            // 0=off, 1=kick, 2=shape, 3=hold
  uint32_t t_start;         // microsecond time when last phase begins
  uint32_t kick_us;         // duration of kick phase in microseconds
  uint32_t shape_us;        // duration of shape phase in microseconds
  uint16_t shape_pwm;       // shape phase PWM value, range 2000-4095, decides strength of note
};

Strike strikes[NUM_KEYS];   //initialize Strike values for all 48 keys


// Serial Input Buffer
char serialBuffer[SERIAL_BUFFER];  //stores up to 64 characters from serial messages, will reset with each new message
uint8_t bufferIndex = 0;


// lowest level note trigger, called for all note on and off events
//takes in note MIDI value & PWM velocity range 0-4095
void triggerNote(int note, int velocity) { 
  if (note >= 36 && note <= 47) {
    pwm2.setPin(note - 36, velocity);
  } else if (note >= 48 && note <= 59) {
    pwm3.setPin(note - 48, velocity);
  } else if (note >= 60 && note <= 71){
    pwm4.setPin(note - 60, velocity);
  } else if (note >= 72 && note <= 83){
    pwm5.setPin(note - 72, velocity);
  }
}


// Start strike
//takes in MIDI note, kick time in micros, shape pwm strength, shape time
void startStrike(uint8_t note, uint32_t kick_us, uint16_t shape_pwm, uint32_t shape_us) {
  int index = note - MIN_MIDI;
  if (index < 0 || index >= NUM_KEYS) return;       //note out of range of physical solenoids

  Strike &s = strikes[index];
  s.active    = true;
  s.phase     = 1;                 // enters kick phase
  s.t_start   = micros();
  s.kick_us   = kick_us;
  s.shape_us  = shape_us;
  s.shape_pwm = shape_pwm;

  triggerNote(note, PWM_MAX);      // full-power kick
}


// Process a single serial message in  
// format note:velocity ---> e.g. '60:105' ---> MIDI note 60 (middle C) at velocity 105
void processSerialMessage(const char* msg) {
  int colonIndex = -1;
  for (int i = 0; msg[i] != '\0'; i++) {
    if (msg[i] == ':') {
      colonIndex = i;
      break;
    }
  }
  if (colonIndex <= 0) return;  // if no colon present, message is malformed for some reason and cannot be used

  int note = atoi(msg);                  // before ':'
  int velocity = atoi(msg + colonIndex + 1);  // after ':'

  int index = note - MIN_MIDI;
  if (index < 0 || index >= NUM_KEYS) return; // out of range

  if (velocity == 0) {
    strikes[index].active = false;  // mark for hold → off
  } else {
    uint32_t kick_time = 9000 + 30 * velocity;  //every note gets at least 9ms of initial high-power kick
    uint32_t shape_time = 15000 + 60 * velocity; //longer shape time for stronger notes
    uint16_t shape_pwm = map(velocity, 0, 127, 2500, 4095);
    startStrike(note, kick_time, shape_pwm, shape_time);
  }
}


// Strike state updater
// Iterates through each key's Strike, ensures key is set at proper pwm & updates Strike state
void updateStrikes() {
  uint32_t now = micros();

  for (int i = 0; i < NUM_KEYS; i++) {
    Strike &s = strikes[i];

    if (s.phase == 0) continue;

    uint32_t elapsed = now - s.t_start;   //elapsed stores how long current phase has lasted

    if (s.phase == 1 && elapsed >= s.kick_us) {
      // Kick ---> Shape transition
      triggerNote(i + MIN_MIDI, s.shape_pwm);
      s.phase = 2;
      s.t_start = now;
    }
    else if (s.phase == 2 && elapsed >= s.shape_us) {
      // Shape ---> Hold transition
      triggerNote(i + MIN_MIDI, HOLD_PWM);
      s.phase = 3;
      s.t_start = now; //may include more phases in future
    }
    else if (s.phase == 3) {
      // Hold ---> Off transition if note inactive
      if (!s.active) {
        triggerNote(i + MIN_MIDI, 0);
        s.phase = 0;
      }
    }
  }
}


// Setup
void setup() {
  Serial.begin(115200);
  pwm2.begin();
  pwm3.begin();
  pwm4.begin();
  pwm5.begin();

  pwm2.setPWMFreq(1000);
  pwm3.setPWMFreq(1000);
  pwm4.setPWMFreq(1000);
  pwm5.setPWMFreq(1000);

  // Initialize all strikes to off
  for (int i = 0; i < NUM_KEYS; i++) {
    strikes[i].active = false;
    strikes[i].phase = 0;
    triggerNote(i + MIN_MIDI, 0);
  }
  // test for figuring out proper kick and shape times
  // for (int i=2000; i < 4000; i+=200) {
  //   triggerNote(60, 4095);
  //   delay(10);
  //   triggerNote(60,i);
  //   delay(10);
  //   triggerNote(60, 2000);
  //   delay(1000);
  //   triggerNote(60, 0);
  //   delay(500);
  // }
}


// Main Loop
void loop() {
  // SERIAL INPUT READING
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      serialBuffer[bufferIndex] = '\0';  // null-terminate
      processSerialMessage(serialBuffer);
      bufferIndex = 0;
    } else if (bufferIndex < SERIAL_BUFFER - 1) {
      serialBuffer[bufferIndex] = c;
      bufferIndex += 1;
    } else {
      // Buffer full, reset
      bufferIndex = 0;
    }
  }

  // STRIKE ENGINE UPDATE
  updateStrikes();
}
