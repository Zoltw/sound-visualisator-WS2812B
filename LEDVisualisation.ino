/*
 * @audioVisulizer.ino
 * @author F.Z, A.Z, N.Z, S.Z
 * @brief Sound visualisation on LED strip (WS2812B)
 * @version 1.0
 * @date 2022-05-27
 * 
 */

#include "FastLED.h"                                          
 
#define LED_PIN 12                                            
#define COLOR_ORDER GRB                                       
#define LED_TYPE WS2812B                                       
#define NUM_LEDS 18                                           
#define MAXSTEPS 16
#define MIC_PIN  A0                                          
#define DC_OFFSET 0  
#define NSAMPLES 64

struct CRGB leds[NUM_LEDS];                                            

int maxBright = 255;

unsigned int arraySample[NSAMPLES];
unsigned long sumSample = 0, oldTime = 0, newTime = 0;
unsigned int avgSample = 0, sample = 0;
int countSample = 0;

uint8_t colour, myFade = 255, backgroundColor = 0;                                             
int center = 0, step = -1;                                                                                      
int peaksPerSec = 0, peakCount = 0;
                                          
void setup() {
  pinMode(12, OUTPUT);                                       
 
  Serial.begin(57600);

  LEDS.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);        //Declaration of diod model WS2812B

  FastLED.setBrightness(maxBright);
}

void loop() {
  EVERY_N_MILLISECONDS(250) {
    Serial.println(analogRead(MIC_PIN));                               //Data from microphone colected by serial port                                                                   
  }                                                                    //Values will be different depending on potentiometer

  EVERY_N_MILLISECONDS(1000) {
    peaksPerSec = peakCount;                                           //Counting peaks per sec
    peakCount = 0;                                                     //Reset counter
  }

  soundReact();

  EVERY_N_MILLISECONDS(20) {
   fade();
  }
   show_at_max_brightness_for_power();
   FastLED.show();
}


void soundReact() {                                                
  newTime = millis();
  int microRead = analogRead(MIC_PIN);
  map(microRead, 0, 45, 512, 1024);                                    //Mapping value from microphone [0,45] - [512,1024]
  sample = abs(microRead);

  sumSample = sumSample + sample - arraySample[countSample];           //Add new sample, delete last from array
  avgSample = sumSample/NSAMPLES;                                      //Computing of the average
  arraySample[countSample] = sample;                                   //Update of last sample
  countSample = (countSample + 1) % NSAMPLES;                          //Counter

  if (newTime > (oldTime + 200))
    digitalWrite(12, LOW);                                             //Turn off diod after last high state

  if ((sample > (avgSample + 40)) && (newTime > (oldTime + 60))) {     //Check next value
    step = -1;
    peakCount++;
    digitalWrite(12, HIGH);
    oldTime = newTime;
  }
} 


void fade() {                                         
  for (int i = 0; i < NUM_LEDS; i++) 
      leds[i] = CHSV(backgroundColor, 255, avgSample * 2);              //Init background color from CHSV color plate                 

  switch (step) {
    case -1:                                                            //Init
      center = random(NUM_LEDS);    
      colour = (peaksPerSec * 10) % 255;                                //Higher state = higher color valur                 
      step = 0;
      backgroundColor = backgroundColor + 8;
      break;
    case 0:                                                             //First diod faded
      leds[center] = CHSV(colour, 255, 255);                                           
      step++;
      break;
    case MAXSTEPS:                                                      //End of fade       
      break;
    default:                                                            //Middle of fade             
      leds[(center + step + NUM_LEDS) % NUM_LEDS] += CHSV(colour, 255, myFade/step * 2);  
      leds[(center - step + NUM_LEDS) % NUM_LEDS] += CHSV(colour, 255, myFade/step * 2);
      step++;                                                                          
      break;  
  }
}
