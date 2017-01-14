/*
 * This sketch is used to automatically turn on IR controlled candles.
 *     
 *     - IR led to PIN 3 on Uno/Nano, 9 on Leonardo/Uno, 9 on Mega2560 
 *     - Motion detector to MOTION_SENSOR_PIN
 */

#include <IRremote.h>
#include <Wire.h>
#include <TimedBlink.h>

#define REPEAT_SEQUENCE     20
#define DO_IT_TIMES         5
#define WAIT_IN_BETWEEN     500
#define MOTION_SENSOR_PIN   6
#define TIME_ON_MS          (60UL*1000UL)
#define IR_FREQUENCY        38
#define OUTPUT_LED          LED_BUILTIN
#define SETUP_ON_OFF_DELAY  1000

#define INIT_BLINK              100,100
#define IDLE_BLINK              200,5000
#define CANDLES_ON_BLINK        1000,500
#define CANDLES_OFF_BLINK       500,1000
#define CANDLES_TURNING_ON      50,10
#define CANDLES_TURNING_OFF     10,50

bool candles_status = false;
bool motion_status  = false;
unsigned long time_last_motion;

IRsend irsend;
TimedBlink monitor(OUTPUT_LED);

// These are the signals for MIRAGE Candles

static unsigned int on_signal[] = {
  1232, 416, 1240, 448, 424, 1256, 1268, 396, 448, 1264, 416, 1268, 424, 1236, 444, 1268, 424, 1256, 428, 1236, 1292, 420, 424, 8060
};

static unsigned int off_signal[] = {
  1175, 481, 1175, 493, 356, 1312, 1181, 487, 350, 1331, 343, 1318, 350, 1318, 356, 1306, 1193, 462, 356, 1337, 343, 1318, 343, 8056
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialization

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  monitor.blink(INIT_BLINK); // INITIALIZATION BLINK
  pinMode(MOTION_SENSOR_PIN, INPUT);

  print_value("Turning candles ","on");
  turn_on();
  print_candle_status(candles_status);
  monitor.blinkDelay(SETUP_ON_OFF_DELAY);

  print_value("Turning candles ","off");
  turn_off();
  print_candle_status(candles_status);
  monitor.blinkDelay(SETUP_ON_OFF_DELAY);
  print_value("Start monitoring","");
  monitor.blink(IDLE_BLINK);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Turn ON candle

void change_candle(bool status, unsigned int *irSignal, int irSignalLen) {
  candles_status = status;
  for (int j = 0; j < DO_IT_TIMES; j++) {
    for (int i = 0; i < REPEAT_SEQUENCE; i++) {
        irsend.sendRaw(irSignal, irSignalLen, IR_FREQUENCY);
        monitor.blink();
    }
    delay(WAIT_IN_BETWEEN);
  }
}


void turn_on() {
  monitor.blink(CANDLES_TURNING_ON);
  change_candle(true,on_signal,sizeof(on_signal)/sizeof(on_signal[0]));
  monitor.blink(CANDLES_ON_BLINK);
}

void turn_off() {
  monitor.blink(CANDLES_TURNING_OFF);
  change_candle(false,off_signal,sizeof(off_signal)/sizeof(off_signal[0]));
  monitor.blink(CANDLES_OFF_BLINK);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Motion detection

bool check_for_motion() {
  motion_status = (digitalRead(6) == HIGH);
  return motion_status;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void print_value(const char *label,const char *value) {
  //Serial.print(label);
  //Serial.println(value);
}

void print_str(const char *label) {
  print_value(label,"");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Printe Candles Status

void print_candle_status(char *status) {
  print_value("Candles: ",status);
}

void print_candle_status(bool status) {
  print_candle_status((char *)(status? "ON":"off"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Print Motion Status

void print_motion_status(bool status) {
  print_value("Motion: ",status? "DETECTED":"none");
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main LOOP

void loop() {

  monitor.blink();
  check_for_motion();

  if (motion_status == true) {
    if (time_last_motion == 0) {
        print_motion_status(true);
    }
    time_last_motion = millis();
    if (candles_status == false) {
      print_value("Turning candles ","on");
      turn_on();
      print_candle_status(candles_status);      
      monitor.blink(CANDLES_ON_BLINK);
    }
  } 
  else {
    if (candles_status == true) {
      unsigned long ct = millis();
      unsigned long diff = abs(ct - time_last_motion);
      if (diff > TIME_ON_MS)  {
        print_value("Turning candles ","off");        
        turn_off();
        print_candle_status(candles_status);
        time_last_motion=0;
        monitor.blink(IDLE_BLINK);
      } 
      else {
        char buf[5];
        ultoa((TIME_ON_MS-diff)/1000L,buf,10);
        monitor.blinkDelay(1000);
      }
    }
  }
}

