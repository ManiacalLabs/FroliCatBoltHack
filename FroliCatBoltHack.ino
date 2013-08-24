//arduino-tiny "ATTiny45 @ 1 MHz"

#include <avr/wdt.h>        // Supplied Watch Dog Timer Macros 
#include <avr/sleep.h>      // Supplied AVR Sleep Macros
#include <avr/power.h>
#include <EEPROM.h>
#include "notes.h"

volatile uint32_t _intervalCount = 0;
uint32_t _interval = 0;
volatile bool _doPulse = false;

void playSong()
{
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < _songDur; thisNote++) {

    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000/_noteDurations[thisNote];
    tone(_speakerPin, _melody[thisNote],noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(_speakerPin);
  }
}

void setup()
{
  //power savings 
  ADCSRA &= ~(1<<ADEN); //Disable ADC
  power_usi_disable();
  ACSR = (1<<ACD); //Disable the analog comparator
  DIDR0 = 0x3F; //Disable digital input buffers on all ADC0-ADC5 pins.

  cli();
  // Set up Watch Dog Timer for Inactivity
  WDTCR |= (_BV(WDCE) | _BV(WDE));   // Enable the WD Change Bit
  WDTCR =   _BV(WDIE) |              // Enable WDT Interrupt
  _BV(WDP2) | _BV(WDP1);   // Set Timeout to ~1 seconds

  if(MCUSR & _BV(WDRF)){            // If a reset was caused by the Watchdog Timer...
    MCUSR &= ~_BV(WDRF);                 // Clear the WDT reset flag
    WDTCR |= (_BV(WDCE) | _BV(WDE));   // Enable the WD Change Bit
    WDTCR = 0x00;                      // Disable the WDT
  }

  // Enable Sleep Mode for Power Down
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // Set Sleep Mode: Power Down
  sleep_enable();                     // Enable Sleep Mode  

    DDRB |= _BV(PINB4); // Set PB4 as output;
  PORTB &= ~_BV(PINB4); //Set it to low, just to be safe

  //avrdude -C C:\avrdude.conf -P usb -p t45 -c avrispmkII -U eeprom:w:0x16:m
  uint8_t val = EEPROM.read(0);
  if(val == 0){ 
    val =  22; 
    EEPROM.write(0, val); 
  }

  _interval = map(val, 0, 255, 0, 86400);

  //_interval = (uint32_t)val;//for testing, when you want the interval to be mere seconds

  sei();  // Enable Interrupts 

  _doPulse = true;
}

void loop()
{
  if(_doPulse)
  {
    _doPulse = false;
    playSong();
    PORTB |= _BV(PINB4);
    delay(100);
    PORTB &= ~_BV(PINB4);
  }

  if (MCUCR & _BV(SE)){    // If Sleep is Enabled...
    sleep_cpu();           // Go to Sleep
  }

}

ISR(WDT_vect)
{
  sleep_disable();          // Disable Sleep on Wakeup

  _intervalCount++;
  if(_intervalCount >= _interval)
  {
    _intervalCount = 0;
    _doPulse = true;
  }

  sleep_enable();           // Enable Sleep Mode
}


















