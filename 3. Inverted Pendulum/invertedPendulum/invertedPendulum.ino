//Code for LQR Control of the Inverted Pendulum
#include <Metro.h>              //Metro libary for timed differential calculations

#define sel1 38
#define sel2 39
#define rst_ 40
#define clk 12

#define pin1 8
#define pin2 9

float theta,alpha;
float alpha_dash,theta_dash;
float alpha_prev = 0;
float theta_prev = 0;
Metro diffMetro = Metro(20);    //Metro instance for 20 ms 

float k1 = -3.535;
float k2 = 58.1041;
float k3 = -2.0935;
float k4 = 7.6010;

void encoderDecoderSetup(){
  pinMode(sel1, OUTPUT);
  pinMode(sel2, OUTPUT);
  pinMode(rst_, OUTPUT);
  pinMode(clk, OUTPUT);

  TCCR1B=0x01;
  analogWrite(clk, 127);

  digitalWrite(rst_, LOW);
  delay(1000);
  digitalWrite(rst_, HIGH);

  DDRC=0b00000000;;
}

int encoderDecoderRead(bool pick){
  digitalWrite(sel2, LOW);
  byte alpha1, alpha2;
  digitalWrite(sel1, LOW);
  alpha2 = pick ? PINC : PINA;
  digitalWrite(sel1, HIGH);
  alpha1 = pick ? PINC : PINA;
  return word(alpha2, alpha1);
}

void setup() {
  encoderDecoderSetup();
  Serial.begin(9600);
}

void loop() {
  if(diffMetro.check() == 1){
    theta = encoderDecoderRead(true)*PI/1024;
    alpha = ((encoderDecoderRead(false))*PI)/1024;
    alpha_dash = (alpha - alpha_prev)/0.02;
    theta_dash = (theta - theta_prev)/0.02;
    alpha_prev = alpha;
    theta_prev = theta;
    Serial.print(theta*180/PI);
    Serial.print(',');
    Serial.println(alpha);
    //do something
    float voltage = k1*theta + k2*alpha + k3*theta_dash + k4*alpha_dash;
    if(voltage>0)
    {
      analogWrite(pin2,min(voltage*255/12.0,255));
      analogWrite(pin1,0);
    }
    else
    {
      analogWrite(pin2,0);
      analogWrite(pin1,min(-voltage*255/12.0,255));
    }
  }
}
