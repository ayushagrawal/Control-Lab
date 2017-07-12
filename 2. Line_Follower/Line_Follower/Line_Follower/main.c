#include "lcd.h"
#include "motion.h"
#include <math.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

unsigned char ADC_Value;
unsigned char leftLine,rightLine,centerLine = 0;

float error = 0;
int shift = 1;
float maxValue = 0;

float prev_error = 0;
float integral_error = 0;
float diff_error = 0;
float pid = 0;

#define MAX_SPEED 230
int velocity = MAX_SPEED;

float Kp = 1.5;
float Ki = 0;
float Kd = 10;

int flag = 0;

int min(int a, int b){
	return (a < b) ? a : b;
}

int max(int a, int b){
	return (a > b) ? a : b;
}
//Function to Initialize ADC
void adc_init()
{
	ADCSRA = 0x00;
	ADMUX = 0x20;		//Vref=5V external --- ADLAR=1 --- MUX4:0 = 0000
	ACSR = 0x80;
	ADCSRA = 0x86;		//ADEN=1 --- ADIE=1 --- ADPS2:0 = 1 1 0
}

//ADC pin configuration
void adc_pin_config (void)
{
	DDRA = 0x00;   //set PORTF direction as input
	PORTA = 0x00;  //set PORTF pins floating
}

//This Function accepts the Channel Number and returns the corresponding Analog Value
unsigned char ADC_Conversion(unsigned char Ch)
{
	unsigned char a;
	Ch = Ch & 0x07;
	ADMUX= 0x20| Ch;
	ADCSRA = ADCSRA | 0x40;		//Set start conversion bit
	while((ADCSRA&0x10)==0);	//Wait for ADC conversion to complete
	a=ADCH;
	ADCSRA = ADCSRA|0x10;      //clear ADIF (ADC Interrupt Flag) by writing 1 to it
	return a;
}

// Used for velocity control
//TIMER1 initialize - prescale:64
// WGM: 5) PWM 8bit fast, TOP=0x00FF
// desired value: 450Hz
// actual value: 450.000Hz (0.0%)
void timer1_init(void)
{
	TCCR1B = 0x00; //stop
	TCNT1H = 0xFF; //setup
	TCNT1L = 0x01;
	OCR1AH = 0x00;
	OCR1AL = 0xFF;
	OCR1BH = 0x00;
	OCR1BL = 0xFF;
	ICR1H  = 0x00;
	ICR1L  = 0xFF;
	TCCR1A = 0xA1;
	TCCR1B = 0x0D; //start Timer
}

void init()
{
	cli();					// Clears the global interrupts
	lcd_port_config();
	adc_init();
	adc_pin_config();
	timer1_init();
	motion_pin_config();	// Refer the function
	sei();					// Enables the global interrupts
	lcd_set_4bit();			// Setting the LCD to 4 bit mode
	lcd_init();
}

// Used for controlling the rotation of the motors
void motor_control(void)
{
	leftLine = ADC_Conversion(3) >> shift;
	rightLine = ADC_Conversion(5) >> shift;
	centerLine = ADC_Conversion(4) >> shift;

	int direction = leftLine - rightLine;
	if(direction > 0){
		if(direction > maxValue/1.5)
			left();
		else if (direction > maxValue/5 && centerLine < 50)
			soft_left();
		else
			forward();
	}
	else{
		direction = -direction;
		if(direction > maxValue/1.5)
			right();
		else if (direction > maxValue/5 && centerLine < 50)
			soft_right();
		else
			forward();
	}

	if(leftLine + rightLine > 120 && centerLine < 50)
	{
		left();
		flag = 1;
		_delay_ms(1000);;
	}

	if (flag == 1)
		if (leftLine + rightLine > 120 && centerLine > 50)
		{
			left();
			flag = 0;
			_delay_ms(500);
		}
}

int main(void)
{
	init();
	lcd_init();
	lcd_string("Starting SPARK V");
	_delay_ms(5000);
	lcd_home();

	lcd_string("Calibrating!  ");

	/*for(int i = 0; i < 100; i++){*/
	/*while(1){
		centerLine = ADC_Conversion(4) >> shift;
		leftLine = ADC_Conversion(3) >> shift;
		rightLine = ADC_Conversion(5) >> shift;

		lcd_print(2,1,leftLine,3);
		lcd_print(2,5,centerLine,3);
		lcd_print(2,9,rightLine,3);
		_delay_ms(10);
	}/*

	for(int i = 0; i < 100; i++){
		centerLine = ADC_Conversion(4) >> shift;
		maxValue = (maxValue > centerLine) ? maxValue : centerLine;
		_delay_ms(100);
	}*/

	maxValue = 90;
	lcd_init();
	forward();
	set_velocity((char)velocity,(char)velocity);

	while(1)
	{
		centerLine = ADC_Conversion(4) >> shift;
		error = maxValue - centerLine;

		/*lcd_print(1,1,leftLine,3);
		lcd_print(1,5,centerLine,3);
		lcd_print(1,9,rightLine,3);/*
		// Till now we would have the value of the error

		/****************************** P.I.D. ******************************/
		integral_error = integral_error + error;
		diff_error = error - prev_error;
		pid = abs(Kp*error) + abs(Ki*integral_error) + (Kd*diff_error);
		prev_error = error;
		/****************************** P.I.D. ******************************/

		set_velocity((char)(MAX_SPEED - pid), (char)(MAX_SPEED - pid));
		motor_control();
		//_delay_ms(10);
	}

	return 0;
}