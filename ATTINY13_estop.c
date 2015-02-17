/*--- CPU_CLK = internal 4.8MHz oscillator -----------*/

#include <avr/io.h>
#include <avr/interrupt.h>

#define STARTUP_DELAY 3000 //start up delay in ms
#define PULSE_COUNT  3 //number of pulses expected in the window
#define TRIGGER_WINDOW 4000 //window time in ms to wait befor triggering alarm
#define ALARM_TIME 2000//time in ms for which alarm remains on before reactivating

volatile unsigned int timer,no_pulses,count,started;


void init_mcu(void)
{
PCMSK|=(1<<PCINT0);//PCINT0 interrupt enable
DDRB=0xFF;
TCCR0A|=(1<<WGM01);//CTC mode
OCR0A=75;//interrupt every 1ms
TCCR0B|=(1<<CS01);//|(1<<CS00);//start timer0
}

unsigned char detected()
{
unsigned char detected=0;
if((no_pulses>=(PULSE_COUNT*2-1))&&(no_pulses<=(PULSE_COUNT*2+1)))
detected=1;

return detected;
}


int main(void)
{
init_mcu();
sei();

GIMSK|=(1<<PCIE);//pin change interrupt enable
TIMSK0|=(1<<OCIE0A);//enable output compare match interrupt
while(timer<STARTUP_DELAY);//wait for start_up delay to be over

GIMSK&=~(1<<PCIE);//pin change interrupt disable
TIMSK0&=~(1<<OCIE0A);//disable output compare match interrupt

while(1)
{
/*------------- Clear variables -------*/
timer=0;
no_pulses=0;
count=0;
started=0;
/*------------------------------------*/

GIMSK|=(1<<PCIE);//pin change interrupt enable
TIMSK0|=(1<<OCIE0A);//enable output compare match interrupt
TCNT0=0;//clear timer0

while(timer<TRIGGER_WINDOW);//wait for sampling window to be over

GIMSK&=~(1<<PCIE);//pin change interrupt disable
TIMSK0&=~(1<<OCIE0A);//disable output compare match interrupt

/*--------------- check if the pulses are detected and respond accordingly --------*/
if(detected())//pulse has been detected
{
PORTB|=(1<<4);//pull up PIN4
PORTB&=~(1<<3);//alarm off
}

else//pulse has not been detected
{
PORTB&=~(1<<4);//pull down PIN4
PORTB|=(1<<3);//alarm on
timer =0;
TCNT0=0;
TIMSK0|=(1<<OCIE0A);//disable output compare match interrupt
while(timer<ALARM_TIME);//wait till alarm time is over
TIMSK0&=~(1<<OCIE0A);//disable output compare match interrupt
PORTB&=~(1<<3);//alarm off
}
/*--------------------------------------------------------------------------------*/

}
}

/*----------------------------- PCINT0 ISR --------*/
ISR(PCINT0_vect)
{
  no_pulses++;//counts number of pulses
}
/*-------------------------------------------------*/

/*------------ interrupt every ms -------------------*/
ISR(TIM0_COMPA_vect)
{
timer++;
TCNT0=0;
}
/*-----------------------------------------------*/
