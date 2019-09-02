#include <FastLED.h>
#include <Bounce2.h>

#define SHUTDOWN_BUTTON_PIN 2
#define SHUTDOWN_PIEZO_PIN 11
#define SHUTDOWN_BUTTON_INTERVAL_MILLIS 3000
#define SHUTDOWN_BUZZ_MILLIS 500
#define SHUTDOWN_BUZZ_TONE 1000
#define SHUTDOWN_TIMEOUT_MILLIS 10000
#define STATE_SHUTDOWN 'D'

#define NUM_RAIL_STRIPS 2
#define NUM_LEDS_PER_RAIL_STRIP 16

#define NUM_EYES_STRIPS 1
#define NUM_LEDS_PER_EYES_STRIP 3

#define LED_BRIGHTNESS 22
#define LED_ON_OFF_RND_RANGE 100

#define LED_PROCESSING_DELAY_MILLIS 125
#define LED_LISTEN_DELAY_MILLIS 250

#define RAIL_1_STRIP_PIN 12
#define RAIL_2_STRIP_PIN 13
#define EYES_1_STRIP_PIN 10

#define SATE_UNASSIGNED '-'

#define STATE_IDLE 'I'
#define NUM_IDLE_COLORS 2
CRGB::HTMLColorCode woprProcessingColors[NUM_IDLE_COLORS] = {CRGB::Red, CRGB::Orange};

#define STATE_ALEXA_ERROR 'E'
#define STATE_ALEXA_SPEAKING 'S'
#define LED_PULSE_DELAY_MILLIS 2
int pluseAdjustmentStepCount = 0;
int pulseAdjustmentDirection = -1; // start by adjusting down
CRGB speakingBaseColor = CRGB::Blue;
CRGB errorBaseColor = CRGB::Red;

#define STATE_ALEXA_THINKING 'T'
#define STATE_ALEXA_LISTENING 'L'
#define NUM_LISTEN_COLORS 3
CRGB::HTMLColorCode woprBlueColors[NUM_LISTEN_COLORS] = {CRGB::Blue, CRGB::Aqua, CRGB::Azure};

CRGB ledRail[NUM_RAIL_STRIPS][NUM_LEDS_PER_RAIL_STRIP];
CRGB ledEyes[NUM_EYES_STRIPS][NUM_LEDS_PER_EYES_STRIP];

char currentState;
char lastState;

// Instantiate a Bounce object for shutdown button
Bounce shutdownDebouncer = Bounce(); 

void setup() {
  // Setup the shutdown button with an internal pull-up and attach shutdownDebouncer with time interval:
  pinMode(SHUTDOWN_BUTTON_PIN,INPUT_PULLUP);
  shutdownDebouncer.attach(SHUTDOWN_BUTTON_PIN);
  shutdownDebouncer.interval(SHUTDOWN_BUTTON_INTERVAL_MILLIS);
  pinMode(SHUTDOWN_PIEZO_PIN, OUTPUT); // for shutdown sound
  
  FastLED.addLeds<NEOPIXEL, RAIL_1_STRIP_PIN>(ledRail[0], NUM_LEDS_PER_RAIL_STRIP);
  FastLED.addLeds<NEOPIXEL, RAIL_2_STRIP_PIN>(ledRail[1], NUM_LEDS_PER_RAIL_STRIP);
  FastLED.addLeds<NEOPIXEL, EYES_1_STRIP_PIN>(ledEyes[0], NUM_LEDS_PER_EYES_STRIP);
  LEDS.setBrightness(LED_BRIGHTNESS);
  currentState = SATE_UNASSIGNED;
  lastState = SATE_UNASSIGNED;

  Serial.begin(9600);
}

void loop() {
  handleSerial();
  handleShutdown();

  switch (currentState) {
    case STATE_IDLE:
      woprStandbyLights();
      break;
    case STATE_ALEXA_LISTENING:
      if (lastState != currentState) {
        alexaListenLights();
      }
      break;
    case STATE_ALEXA_THINKING:
      // set the lights for thinking
      alexaThinkingLights();
      break;
    case STATE_ALEXA_SPEAKING:
      if (lastState != currentState) {
        allLightsTo(speakingBaseColor);
      }
      alexaSpeakingLights();
      break;
    case STATE_ALEXA_ERROR:
      if (lastState != currentState) {
        allLightsTo(errorBaseColor);
      }
      alexaErrorLights();
      break;
    case SATE_UNASSIGNED:
      allLightsTo(CRGB::Black);
      break;
  }

  lastState = currentState;
}

void handleShutdown() {
  shutdownDebouncer.update();
  // Turn on the LED if either button is pressed :
  if (shutdownDebouncer.read() == HIGH) {
    tone(SHUTDOWN_PIEZO_PIN, SHUTDOWN_BUZZ_TONE);
    delay(SHUTDOWN_BUZZ_MILLIS);       
    noTone(SHUTDOWN_PIEZO_PIN);
    Serial.write(STATE_SHUTDOWN); // send shutdown command over serial
    delay(SHUTDOWN_TIMEOUT_MILLIS); // set a delay to wait for shutdown (i.e. back to normal if shutdown doesn't occur)
  }
}

void handleSerial() {
  while (Serial.available() > 0) {
    char incomingCharacter = Serial.read();
    switch (incomingCharacter) {
      case STATE_IDLE:
      case STATE_ALEXA_LISTENING:
      case STATE_ALEXA_THINKING:
      case STATE_ALEXA_SPEAKING:
      case STATE_ALEXA_ERROR:
        currentState = incomingCharacter;
        break;
      default:
        currentState = SATE_UNASSIGNED;
        break;
    }
  }
}

void alexaSpeakingLights() {
  // for each strip
  for (int x = 0; x < NUM_RAIL_STRIPS; x++) {
    // for each led in rail strip
    for (int i = 0; i < NUM_LEDS_PER_RAIL_STRIP; i++) {
      if (pulseAdjustmentDirection == -1) {
        ledRail[x][i] += CHSV(0, 1, 1);
      } else {
        ledRail[x][i] -= CHSV(0, 1, 1);
      }
    }
  }

  for (int x = 0; x < NUM_EYES_STRIPS; x++) {
    // for each led in eyes strip
    for (int i = 0; i < NUM_LEDS_PER_EYES_STRIP; i++) {
      if (pulseAdjustmentDirection == -1) {
        ledEyes[x][i] += CHSV(0, 1, 1);
      } else {
        ledEyes[x][i] -= CHSV(0, 1, 1);
      }
    }
  }
  
  pluseAdjustmentStepCount++;
  if (pluseAdjustmentStepCount >= 200) {
    pluseAdjustmentStepCount = 0;
    pulseAdjustmentDirection *= -1;
  }

  FastLED.show();
  FastLED.delay(LED_PULSE_DELAY_MILLIS);
}

void alexaThinkingLights() {
  // for each rail strip
  for (int x = 0; x < NUM_RAIL_STRIPS; x++) {
    // for each led in rail strip
    for (int i = 0; i < NUM_LEDS_PER_RAIL_STRIP; i++) {
      if (random8(100) > 40) {
        // switching current rail led on
        ledRail[x][i] = woprBlueColors[random8(NUM_LISTEN_COLORS)];
      } else {
        // switch rail led off
        ledRail[x][i] = CRGB::Black;
      }
    }
  }

  // for each eyes strip
  for (int x = 0; x < NUM_EYES_STRIPS; x++) {
    // for each led in eyes strip
    for (int i = 0; i < NUM_LEDS_PER_EYES_STRIP; i++) {
      if (random8(100) > 40) {
        // switching current eyes led on
        ledEyes[x][i] = woprBlueColors[random8(NUM_LISTEN_COLORS)];
      } else {
        // switch eyes led off
        ledEyes[x][i] = CRGB::Black;
      }
    }
  }
  
  FastLED.show();
  FastLED.delay(LED_PROCESSING_DELAY_MILLIS);
}

void alexaErrorLights() {
  // for each strip
  for (int x = 0; x < NUM_RAIL_STRIPS; x++) {
    // for each led in rail strip
    for (int i = 0; i < NUM_LEDS_PER_RAIL_STRIP; i++) {
      if (pulseAdjustmentDirection == -1) {
        ledRail[x][i] += CHSV(0, 1, 1);
      } else {
        ledRail[x][i] -= CHSV(0, 1, 1);
      }
    }
  }

  for (int x = 0; x < NUM_EYES_STRIPS; x++) {
    // for each led in eyes strip
    for (int i = 0; i < NUM_LEDS_PER_EYES_STRIP; i++) {
      if (pulseAdjustmentDirection == -1) {
        ledEyes[x][i] += CHSV(0, 1, 1);
      } else {
        ledEyes[x][i] -= CHSV(0, 1, 1);
      }
    }
  }
  
  pluseAdjustmentStepCount++;
  if (pluseAdjustmentStepCount >= 200) {
    pluseAdjustmentStepCount = 0;
    pulseAdjustmentDirection *= -1;
  }

  FastLED.show();
  FastLED.delay(LED_PULSE_DELAY_MILLIS);
}

void alexaListenLights() {
  // for each rail strip
  for (int x = 0; x < NUM_RAIL_STRIPS; x++) {
    // for each led in rail strip
    for (int i = 0; i < NUM_LEDS_PER_RAIL_STRIP; i++) {
      // switching current rail led on
      ledRail[x][i] = woprBlueColors[random8(NUM_LISTEN_COLORS)];
    }
  }

  // for each eyes strip
  for (int x = 0; x < NUM_EYES_STRIPS; x++) {
    // for each led in eye strip
    for (int i = 0; i < NUM_LEDS_PER_EYES_STRIP; i++) {
      // switching current eyes led on
      ledEyes[x][i] = woprBlueColors[random8(NUM_LISTEN_COLORS)];
    }
  }
  
  FastLED.show();
}

void woprStandbyLights() {
  // for each rail strip
  for (int x = 0; x < NUM_RAIL_STRIPS; x++) {
    // for each led in rail strip
    for (int i = 0; i < NUM_LEDS_PER_RAIL_STRIP; i++) {
      if (random8(100) > 40) {
        // switching current rail led on
        ledRail[x][i] = woprProcessingColors[random8(NUM_IDLE_COLORS)];
      } else {
        // switch rail led off
        ledRail[x][i] = CRGB::Black;
      }
    }
  }

  // for each eyes strip
  for (int x = 0; x < NUM_EYES_STRIPS; x++) {
    // for each led in eyes strip
    for (int i = 0; i < NUM_LEDS_PER_EYES_STRIP; i++) {
      if (random8(100) > 40) {
        // switching current eyes led on
        ledEyes[x][i] = woprProcessingColors[random8(NUM_IDLE_COLORS)];
      } else {
        // switch eyes led off
        ledEyes[x][i] = CRGB::Black;
      }
    }
  }
  
  FastLED.show();
  FastLED.delay(LED_PROCESSING_DELAY_MILLIS);
}

void allLightsTo(CRGB color) {
  // for each rail strip
  for (int x = 0; x < NUM_RAIL_STRIPS; x++) {
    fill_solid(ledRail[x], NUM_LEDS_PER_RAIL_STRIP, color);
  }

  // for each eyes strip
  for (int x = 0; x < NUM_EYES_STRIPS; x++) {
    fill_solid(ledEyes[x], NUM_LEDS_PER_EYES_STRIP, color);
  }
  FastLED.show();
}

