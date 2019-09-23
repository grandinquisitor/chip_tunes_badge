#include <avr/sleep.h>
#include <avr/pgmspace.h>


#define BODCR _SFR_IO8(0x30)
#define BODSE 0
#define BODS 1

#define LED_PIN PB0
#define UNUSED_PIN PB2
#define OUTPUT_PIN_A PB3
#define OUTPUT_PIN_B PB4
#define INPUT_PIN PB1

typedef struct Note {
  const uint16_t freq: 12;
  const uint8_t dur: 4;
} Note;


const Note NOTES[] PROGMEM = {
  {1517, 4}, {1432, 2}, {0, 8}, {1517, 4}, {1432, 2}, {0, 8}, {1517, 2}, {1432, 2}, {1517, 2}, {1432, 2}, {1517, 2}, {1432, 2}, {1517, 2}, {1432, 2}, {1702, 2}, {1517, 2}, {1517, 2}, {1432, 2}, {1702, 2}, {1517, 2}, {1517, 2}, {1432, 2}, {1517, 2}, {1432, 2}, {1517, 2}, {1432, 2}, {1517, 2}, {1432, 2}, {1517, 2}, {1432, 2}, {1702, 2}, {1517, 2}, {1517, 2}, {1432, 2}, {1702, 2}, {1517, 2}, {1517, 2}, {1432, 2}, {1517, 2}, {1432, 2}, {1517, 2}, {1432, 2}, {0, 2}, {0, 1}, {1607, 1}, {1276, 1}, {902, 8}, {0, 8}, {1607, 1}, {1276, 1}, {902, 1}, {804, 1}, {1073, 1}, {1607, 1}, {1804, 8}
};

#define NUM_NOTES 55

#define APPLY_DURATION(x) (x << 7) + (x << 2) + x + x 



static inline boolean readButton() {
  return !bitRead(PINB, INPUT_PIN);
}


// from https://www.embedded.com/electronics-blogs/break-points/4024981/My-favorite-software-debouncers
inline boolean DebounceSwitch2() {
  //  return readButton();
  static uint16_t State = 0; // Current debounce status
  State = (State << 1) | !readButton() | 0xe000;
  return (State == 0xf000);
}

#define USE_AC 1

// Inspired by TimerFreeTone and ToneAC
inline boolean actone(uint16_t frequency, uint16_t duration) {
  if (frequency == 0) { // If frequency or volume are zero, just wait duration and exit.
    delay(duration);
  } else {

    uint16_t duty = frequency >> 1;

    uint32_t startTime = millis();           // Starting time of note.
    while (millis() - startTime < duration) { // Loop for the duration.
#ifdef USE_AC
      PORTB = (PORTB | _BV(OUTPUT_PIN_B)) & ~_BV(OUTPUT_PIN_A);
#else
      PORTB |= _BV(OUTPUT_PIN_A);
#endif

      // give the led something to do
      PORTB |= _BV(LED_PIN);
      uint16_t subduty = min(duration, duty);
      delayMicroseconds(subduty);
      PORTB &= ~_BV(LED_PIN);
      delayMicroseconds(duty - subduty); // Square wave duration (how long to leave pin high).

#ifdef USE_AC
      PORTB = (PORTB | _BV(OUTPUT_PIN_A)) & ~_BV(OUTPUT_PIN_B);
#else
      PORTB &= ~_BV(OUTPUT_PIN_A);
#endif
      delayMicroseconds(frequency - duty); // Square wave duration (how long to leave pin low).

      if (DebounceSwitch2()) {
        return true;
      }
    }
  }
  return false;
}

void setup() {

  // Enable pull-ups
  PORTB |= _BV(INPUT_PIN) | _BV(UNUSED_PIN);

  // Enable outputs
  DDRB |= _BV(LED_PIN) | _BV(OUTPUT_PIN_A) | _BV(OUTPUT_PIN_B);

  // disable ADC
  ADCSRA &= ~(1 << ADEN);
  ACSR = (1 << ACD); //Disable the analog comparator
  DIDR0 = 1 << ADC3D | 1 << ADC2D | 1 << ADC1D | 1 << ADC0D | 1 << AIN0D; // disable all other input

}

void loop() {

  while (readButton()) {
  }

  // helps debounce
  delay(50);

  goToSleep();

  // helps debounce
  delay(200);

  for (uint8_t i = 0; i < NUM_NOTES; ++i) {
    //      Note note;
    //      memcpy_P(&note, &(NOTES[i]), sizeof(Note));

    // saves ~100 bytes of program space vs. memcopy
    uint16_t packed = pgm_read_word(&NOTES[i]);

    //      Note note = { packed & 0b111111111111, packed >> 12 };

    uint16_t freq = packed & 0b111111111111;
    uint16_t dur = packed >> 12;

    boolean shouldStop = actone(freq, APPLY_DURATION(dur));

    if (shouldStop) {
      break;
    }
    // sounds better with slight delay between the notes
    delay(12);
  }
  PORTB &= ~_BV(OUTPUT_PIN_A) & ~_BV(OUTPUT_PIN_B);
}

static inline void goToSleep() {
  // if button happens to be depressed at this exact moment, causes freezing
  if (readButton()) {
    return;
  }

  bitSet(GIFR, INTF0);
  bitSet(GIMSK, INT0);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  cli();

  // Disable BOD during sleep
  BODCR = (1 << BODSE) | (1 << BODS);
  BODCR = (1 << BODS);

  sei();
  sleep_cpu(); 
  
  // zzz
  
  sleep_disable();
}


ISR (INT0_vect) {
  // Ignore any pending interrupt that occurred during debounce
  bitClear(GIMSK, INT0);
  GIFR |= (1 << INTF0);
}
