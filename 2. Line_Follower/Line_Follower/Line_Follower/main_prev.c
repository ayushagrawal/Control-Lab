/*
 * Line_Follower.c
 *
 * Created: 24-01-2017 17:35:50
 * Author : Shashank
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.c"
#include "motion.c"
	
#define		THRESHOLD		160       // set the pots such that all three sensor 
                                      // calibrated to show its min value on LCD. 
                                      // i.e on LCD Sensor values are betwn 168 to 172
									  // on black line  
#define		VELOCITY_MAX	50
#define		VELOCITY_MIN	30
#define 	VELOCITY_LOW	0

unsigned char ADC_Conversion(unsigned char);
unsigned char ADC_Value;
unsigned char Left_white_line = 0;
unsigned char Center_white_line = 0;
unsigned char Right_white_line = 0;

//ADC pin configuration
void adc_pin_config (void) {
	DDRA = 0x00;   //set PORTF direction as input
	PORTA = 0x00;  //set PORTF pins floating
}

//Function to Initialize PORTS
void port_init()
{
	lcd_port_config();
	adc_pin_config();		
	motion_pin_config();
}

//TIMER1 initialize - prescaler:64
// WGM: 5) PWM 8bit fast, TOP=0x00FF
// desired value: 450Hz
// actual value: 450.000Hz (0.0%)
void timer1_init(void) {
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

//Function to Initialize ADC
void adc_init() {
	ADCSRA = 0x00;
	ADMUX = 0x20;		//Vref=5V external --- ADLAR=1 --- MUX4:0 = 0000
	ACSR = 0x80;
	ADCSRA = 0x86;		//ADEN=1 --- ADIE=1 --- ADPS2:0 = 1 1 0
}

//This Function accepts the Channel Number and returns the corresponding Analog Value 
unsigned char ADC_Conversion(unsigned char Ch) {
	unsigned char a;
	Ch = Ch & 0x07;  			
	ADMUX= 0x20| Ch;	   		
	ADCSRA = ADCSRA | 0x40;		//Set start conversion bit
	while((ADCSRA&0x10)==0);	//Wait for ADC conversion to complete
	a=ADCH;
	ADCSRA = ADCSRA|0x10;		//clear ADIF (ADC Interrupt Flag) by writing 1 to it
	return a;
}

// This Function prints the Analog Value Of Corresponding Channel No. at required Row
// and Column Location. 
void print_sensor(char row, char column, unsigned char channel) {
	ADC_Value = ADC_Conversion(channel);
	lcd_print(row, column, ADC_Value, 3);
}

void init_devices (void) {
	cli();          //Clears the global interrupts
	port_init();
	timer1_init();
	adc_init();
	sei();          //Enables the global interrupts
}

//Main Function
int main(void) {
	unsigned char flag ;

	init_devices();

	lcd_set_4bit();
	lcd_init();

	velocity(VELOCITY_MAX,VELOCITY_MAX);    // Set the speed to max velocity
	lcd_print (2,1,VELOCITY_MAX,3);
	lcd_print (2,5,VELOCITY_MAX,3);
	forward();                              // start to move froward

	while(1) {
		Left_white_line = ADC_Conversion(3);	//Getting data of Left WL Sensor
		Center_white_line = ADC_Conversion(4);	//Getting data of Center WL Sensor
		Right_white_line = ADC_Conversion(5);	//Getting data of Right WL Sensor

		print_sensor(1,1,3);		//Prints value of White Line Sensor Left
		print_sensor(1,5,4);		//Prints value of White Line Sensor Center
		print_sensor(1,9,5);		//Prints value of White Line Sensor Right
		flag = 0;
		if(Center_white_line > THRESHOLD) {		// Is middle Whiteline is within threshold limit                                             
			flag = 1;
			velocity(VELOCITY_MAX, VELOCITY_MAX);      // Run robot at max velocity 
			lcd_print (2,1,VELOCITY_MAX,3);
			lcd_print (2,5,VELOCITY_MAX,3);
		}

		if((Left_white_line<THRESHOLD) && (flag==0)) { // Is left Whiteline is not within threshold limit
		//if((Left_white_line>THRESHOLD) && (flag==0)) {  // Is left Whiteline is not within threshold limit                                      
			flag = 1;                       
			velocity(VELOCITY_MAX,VELOCITY_MIN);      // Run robot left wheel at max velocity and right wheel 
			lcd_print (2,1,VELOCITY_MAX,3);           // at min velocity
			lcd_print (2,5,VELOCITY_MIN,3);         
		/*	velocity(VELOCITY_MIN,60);      // Run robot right wheel at max velocity and left wheel 
			lcd_print (2,1,VELOCITY_MIN,3);           // at min velocity
			lcd_print (2,5,VELOCITY_MAX,3);*/
		}

		if((Right_white_line<THRESHOLD) && (flag==0)) { // Is right Whiteline is not within threshold limit
		//if((Right_white_line>THRESHOLD) && (flag==0)) { // Is right Whiteline is not within threshold limit
			flag = 1;    
			velocity(VELOCITY_MIN,VELOCITY_MAX);      // Run robot right wheel at max velocity and left wheel 
			lcd_print (2,1,VELOCITY_MIN,3);           // at min velocity
			lcd_print (2,5,VELOCITY_MAX,3);
		/*	velocity(60,VELOCITY_MIN);      // Run robot left wheel at max velocity and right wheel 
			lcd_print (2,1,VELOCITY_MAX,3);           // at min velocity
			lcd_print (2,5,VELOCITY_MIN,3);*/
		}

		if(Center_white_line<THRESHOLD && Left_white_line<THRESHOLD && Right_white_line<THRESHOLD && (flag == 0)) {
										// if all Whiteline sensor are not within threshold limit    
			flag=1;
			velocity(VELOCITY_LOW,VELOCITY_LOW);      // stop the robot
			lcd_print (2,1,VELOCITY_LOW,3);
			lcd_print (2,5,VELOCITY_LOW,3);
		}
	}
}