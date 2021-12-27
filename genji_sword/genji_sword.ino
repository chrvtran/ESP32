// Test buzzer 
// Test making a noise

/*
Test making a noise when acceleration is large enough
*/

// Make a sequence of tones to create swinging noise?
// Potentially store noise into flash memory 
#include <Wire.h>
#include <GY6050.h>
#include <math.h>
#include <Adafruit_NeoPixel.h>

#define BUZZER 26


#define BUTTON0 34
#define BUTTON1 0
#define BUTTON2 35
#define PIN 25 // NeoPixel PIN

#define stripshow_noglitch() {delay(1);strip.show();delay(1);strip.show();}
#define brightness 128 //128

int sound[] = {128};
const int numNotes = 1;
int duration[] = {3};


float accQ[] = {90, 90, 90, 90, 90}; 
int qIndex = 0; 
GY6050 gyro(0x68);

// Store the X, Y, and Z acceleration as a vector array
// 0 = X acceleration, 1 = Y acceleration, 2 = Z acceleration
int accVector[3];
double prevAcc; 
double epsilon = 50.0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(58, PIN, NEO_GRB + NEO_KHZ800);

unsigned long event_time = 0;

// may need to remove IRAM_ATTR for arduino
volatile int curmode = 0;
void IRAM_ATTR displaymode() {
   if (millis() > event_time + 400) { // debounce
    curmode = (curmode + 1) % 4;
    event_time = millis();
  }
}


void setup() {
  gyro.initialisation();
  Serial.begin(115200);  

  for (int i = 0; i < 3; i++) {
    accVector[i] = 0;  
  }

  pinMode(BUTTON0, INPUT_PULLUP);
//  pinMode(BUTTON1, INPUT_PULLUP);
  // pinMode(BUTTON2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON0), displaymode, FALLING);
//  attachInterrupt(digitalPinToInterrupt(BUTTON1), lightmode, FALLING);
  // attacheInterrupt(digitalPinToInterrupt(BUTTON2), pulsing, FALLING);
  
  strip.begin();
  strip.setBrightness(brightness);
  stripshow_noglitch();

  
  ledcSetup(0, 5000, 8);
  ledcAttachPin(BUZZER, 0);
  ledcWriteTone(0, 440);
  delay(50);
  ledcWrite(0, 0);
  delay(2500);
}

double magnitude() {
  double squareSum = (sq(accVector[0]) + sq(accVector[1]) + sq(accVector[2]));
  double result = sqrt(squareSum);
  return result;
}

// Get the average of the acceleration queue 
float average() {
  float sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += accQ[i];   
  }
  return sum / 5;   
}

// changes one LED at a time for given wait time
void colorWipe(uint32_t color, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i,color);
    stripshow_noglitch();
    delay(wait);
  }
}

void staticlight(uint32_t color) {
  for (uint16_t i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  stripshow_noglitch();
}

void pulsing(uint32_t color) {
  for (uint16_t i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  stripshow_noglitch();
  int curbrightness = brightness;
  int fadeSpeed = 5;
  for (int i = 0; i < 2*brightness; i = i+fadeSpeed) {
    strip.setBrightness(curbrightness);
    stripshow_noglitch();
    curbrightness = curbrightness - fadeSpeed;
    if (curbrightness == brightness || curbrightness == 0) {
      fadeSpeed = -fadeSpeed;
    }
  }
}


void theatreChase(uint32_t color, uint8_t wait) {
  for(int q=0; q < 3; q++) {
    for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
      strip.setPixelColor(i+q, color); // turn every third pixel on
    }
    stripshow_noglitch();
    
    delay(wait);

    for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
      strip.setPixelColor(i+q, 0); // turn every third pixel off
    }
  }
}

void light_mode() {
  if (curmode != 3) {
    strip.setBrightness(brightness);
  }
  if (curmode == 0) {
    staticlight(strip.Color(182,239,19));
  }
  else if (curmode == 1) {
    theatreChase(strip.Color(182,239,19), 50);
  }
  else if (curmode == 2) {
    pulsing(strip.Color(182,239,19));
  }
  else if (curmode == 3) {
    strip.setBrightness(0);
    stripshow_noglitch();
  }  
}

void play_sound() {
  for (int i = 0; i < numNotes; i++) {
    float noteDuration = 1000 / duration[i]; 
    ledcWriteTone(0, sound[i]);
    delay(noteDuration);
    ledcWrite(0, 0);

    int pause = noteDuration * 0.05; 
    delay(pause);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  accVector[0] = gyro.refresh('A', 'X');
  accVector[1] = gyro.refresh('A', 'Y');
  accVector[2] = gyro.refresh('A', 'Z');

  double curAcc = magnitude();
  Serial.print("Current Acceleration: ");
  Serial.print(curAcc);

  Serial.print(" | Average Acceleration: ");
  Serial.print(average());
  Serial.println("");

  light_mode();
  
  if (abs(curAcc - average()) > epsilon) {
    Serial.println("Woosh");
    accQ[qIndex] = curAcc;
    qIndex = (qIndex + 1) % 5;   
    play_sound();
    delay(50);
  } else {
    prevAcc = curAcc;
    ledcWrite(0, 0);
    accQ[qIndex] = curAcc;
    qIndex = (qIndex + 1) % 5;
    delay(50);  
  }
}
