/*
  examples of a state machine using switch/case
*/
//states of our car
#define GO 1
#define BRAKE 2
#define COAST 3
#define PARK 4
#define CRUISE 5
#define REMOTE 6

// motorspeed for remote
#define SPEEDLOW 88
#define SPEEDMED 90
#define SPEEDHIGH 95
#define MAXACCDATA 25

// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
const int led = 13;
const int loopTime = 100;
const int headLightFET = 4;
const int motorFET = 5;
const int brakeLightFET = 2;
const int brakeButton = 7;
const int AcceleraterPin = A5;
const int battLevelPin = A0;
const int headLightSwitch = 6;
//const int accButton = 3;
const int RPMopto = 3;

struct MotorRates {
  unsigned int Acc;
  unsigned int MotorRate;
};


int  mode=PARK;
int  SwlightState=0;
int  CmdlightState=0;
volatile int RPMcounter = 0;


unsigned int minAccelerationValue = 0;
unsigned int myspeed=0;
unsigned int speedIndex=0;
unsigned int cruisespeed=0;
unsigned int pwmValue=0;
unsigned int battValue=0;
unsigned int currentAccelerationValue=0;

unsigned long currentMillis = 0;
unsigned long previousMillis = 0;

struct MotorRates MRate[MAXACCDATA] = {
  {0,0},
  {10,0},
  {15,90},
  {25,95},
  {50,123},
  {100,153},
  {150,171},
  {200,184},
  {250,194},
  {300,202},
  {350,208},
  {400,214},
  {450,219},
  {500,224},
  {550,228},
  {600,232},
  {650,235},
  {700,239},
  {750,242},
  {800,245},
  {850,247},
  {900,250},
  {950,252},
  {995,254},
  {1023,255}
};

long interval = 1000; //amount of time for auto status

int cmd=0;  // the command we get from the tablet


// the setup routine runs once when you press reset:
void setup() {                
  // set initial mode
  mode=PARK;
  
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  Serial.begin(9600);  
  pinMode(brakeButton, INPUT_PULLUP);     
  pinMode(brakeLightFET, OUTPUT);  
  pinMode(headLightFET, OUTPUT);  
  pinMode(motorFET, OUTPUT);
  pinMode(headLightSwitch, INPUT_PULLUP);
  //pinMode(accButton, INPUT_PULLUP);  
  pinMode(RPMopto, INPUT_PULLUP);  
  
  //attach interrupt
  attachInterrupt(1, RPMtask, FALLING);
  
  // set pwm freq
  //setPwmFrequency(5,8);

  // calibrate accelerate pedal at zero
  minAccelerationValue = analogRead(AcceleraterPin);
  battValue = analogRead(battLevelPin);
}


void setMotorSpeed(unsigned int motorRate)
{
    // write it to motorFET
      pwmValue = motorRate;
      analogWrite (motorFET, pwmValue);  
}
void RPMtask()
{
  RPMcounter=RPMcounter+1;
}

// the loop routine runs over and over again forever:
void loop() {

  int i=0;

  switch(mode) {
  
    case PARK:
      // power off head lights
      digitalWrite(headLightFET, LOW);
      // turn off brake light  
      digitalWrite(brakeLightFET, LOW); 
      //turn off motor
      setMotorSpeed(0);  
      break;
    case GO:
      // set motor speed according to accelerator
      // read pedal value 0-1023
      currentAccelerationValue=analogRead(AcceleraterPin);
     
      //what i read > what the min is then myspeed=0;
      if (currentAccelerationValue<minAccelerationValue) {
        myspeed=0;
      }
      else {
            myspeed=currentAccelerationValue-minAccelerationValue;
      }
      
      
      // find where in our rate table is the motor set
      for (i=0;i<MAXACCDATA;i++) {
        if (myspeed <= MRate[i].Acc) {
            break;
        }
      }
      
      setMotorSpeed(MRate[i].MotorRate);
      speedIndex = i;

      //turn off brake lights
      digitalWrite(brakeLightFET, LOW);

      break;
      
    case BRAKE:
      // turn on brake lights
      digitalWrite(brakeLightFET, HIGH);
      // turn PARK motor
      analogWrite(motorFET, 0);
      
      if (digitalRead(brakeButton)==0) {
        mode=COAST;
        // turn PARK brake lights
      }
          
      break;
      
    case COAST:
      // check if the accelerator is pressed, if so then GO!!!
      if (analogRead(AcceleraterPin) > minAccelerationValue) {
         mode=GO;
      }
      else {
      // figure out how fast we are going
      // some math to figure out to decellerate accordingly
      //setMotorSpeed(someValue);
      }
      break;
    case CRUISE:

    //maintain current speed
  
      break;
    case REMOTE:
    
    break;  
    default:
      mode = PARK;
  
  }  
  
  // control the breaks
  if (digitalRead(brakeButton)==1) {
    mode=BRAKE;
  }
  else {
    digitalWrite(brakeLightFET, LOW); 
  }
  
  // control the headlights
  if (SwlightState==1)
     digitalWrite(headLightFET, HIGH);
     
   if (CmdlightState==1&&SwlightState==0)
     digitalWrite(headLightFET, HIGH);
  
   if (CmdlightState==0&&SwlightState==0)
     digitalWrite(headLightFET, LOW);
  
  if ((digitalRead(headLightSwitch)==0)) 
    SwlightState=1;
  else  
    SwlightState=0;
  
  //Check for commands
  if (Serial.available() > 0) {
    
    cmd = Serial.read();
    
    switch(cmd) {
      
      case 'S':
              Serial.print("S:[");
        Serial.print(mode);
        Serial.print(",");
        Serial.print(RPMcounter);
        Serial.print(",");
        Serial.print(pwmValue);
        Serial.print(",");
        Serial.print(battValue);
        Serial.print(",");
        Serial.print(myspeed);
        Serial.print("]\n");

        break;
      case 'L':
            CmdlightState=1;
        break;
      case 'l':
            CmdlightState=0;
        break;
      case 'C':
      mode=CRUISE;
        break;
      case 'c':
        mode=GO;
        break;
      case 'P':
        mode=PARK;
        break;
      case '1':
         mode=REMOTE;
         setMotorSpeed(SPEEDLOW);
          break;
       case '2':
         mode=REMOTE;
         setMotorSpeed(SPEEDMED);
          break;
       case '3':
         mode=REMOTE;
         setMotorSpeed(SPEEDHIGH);
        break;
       default:
         break;
       
    } // end switch
  
  } // end if we have a serial command
  
  //Check if enough time passed to send status
   currentMillis = millis();
 
  if(currentMillis - previousMillis > interval) {
    // save the last time you sent status
    previousMillis = currentMillis;   
    battValue = analogRead(battLevelPin);
        Serial.print("S:[");
        Serial.print(mode);
        Serial.print(",");
        Serial.print(RPMcounter);
        Serial.print(",");
        Serial.print(pwmValue);
        Serial.print(",");
        Serial.print(battValue);
        Serial.print(",");
        Serial.print(myspeed);
        Serial.print("]\n");
        
        //clear RPM counter for next reading
        RPMcounter=0;
        
  } //end of auto timer



}
