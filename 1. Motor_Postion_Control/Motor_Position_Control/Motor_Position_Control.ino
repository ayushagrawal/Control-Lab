int control_1 = 2;  //PWM Pin 2 - Forward
int control_2 = 13;  //PWN Pin 13 - Backward

int pot = 0;        //Analog Read pin 0

int set_point = 0;  //Set point in degrees
long error = 0;

float kp = 25;      //Proportional Coeff
float kd = -5;     //Differential Coeff
float ki = 0.001;   //Integration Coeff

float prev = 0;         //Previous error value
float integrate = 0;  //Integrated value
float valRead = 0;      //Value read
float potPos = 0;       //Position by the potentiometer

unsigned long startTime;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(control_1, OUTPUT);
  pinMode(control_2, OUTPUT);
}

void loop() {
  /*
  // put your main code here, to run repeatedly:
  byte duty = 255;
  analogWrite(control_1, duty);
  analogWrite(control_2, 0);
  while(true){
    Serial.println(float(analogRead(pot))*360/1023);
    if(Serial.available())
    {
      char stop= Serial.read();
      if (stop == 's')
      {
        analogWrite(control_1, 0);
        while(true)
        {
          if(Serial.available())
          {
            char stop = Serial.read();
            if (stop == 's')
            {
              analogWrite(control_1, duty);
              break; 
            }
          }
        } 
      }
    }
  }
  */
  //To read a change in set point
  if(Serial.available())
  {
    //Read the set point value
    set_point = (Serial.parseInt());
    
    //Stop the motor
    analogWrite(control_1, 0);
    analogWrite(control_2, 0);

    //Wait for it to stop
    delay(1000);
    
    Serial.print("New set point ");
    Serial.println(set_point, DEC);
    integrate = 0;

    //Reset Start Time
    startTime = millis();
  }

  //Read potentiometer value
  potPos = (360 - float(analogRead(pot))/1024*360);

  //Print value read
  Serial.print(millis() - startTime, DEC);
  Serial.print(',');
  //Serial.print(set_point);
  //Serial.print(',');
  Serial.print(int(potPos), DEC);
  
  potPos = (set_point > potPos) ? (set_point - potPos) : ((set_point - potPos) + 360);  // Gives the anti-clockwise direction angle(considered negative in the next statement)
  valRead = (potPos < 360 - potPos) ? (potPos) : (potPos - 360);                        //360 - potPos gives the clockwise direction angle

  //Calculate errors
  integrate += valRead;
  error = kp*valRead + kd*(valRead - prev) + ki*integrate;
  prev = valRead; // Used for differential
  //Serial.print(',');
  //Serial.print(int(valRead));
  //Provide it to the motor
  
  if (error > 0){ // Positive error
    //Serial.print("Positive ");
    analogWrite(control_1, min(error, 255));
    analogWrite(control_2, 0);
  }
  else{           // Negative Error
    //Serial.print("Negative ");
    analogWrite(control_1, 0);
    analogWrite(control_2, min((-1)*error,255));
  } 
  Serial.print('\n');
  //analogWrite(control_1, 255);
  //analogWrite(control_2, 0);
}
