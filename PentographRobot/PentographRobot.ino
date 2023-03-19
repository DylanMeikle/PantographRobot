#include <MSE2202_Lib.h>

#define LEG1      41
#define LEG2      42
#define CLAW      40
#define WRIST     39
#define MOTOR     38
#define MSWITCH   10
#define POT       1

Motion Bot = Motion();


boolean Time_Up_1 = false;
boolean Time_Up_2 = false;
boolean Time_Up_3 = false;

unsigned int current_Millis;
unsigned int previous_Millis;
unsigned int Bot_Phase = 0;
int pos = 0;
int i = 0;

//=============================================================
//Values for Servos
//Still need to test all 0 values
const int Claw_Open = 0;    
const int Claw_Closed = 0;
const int Wrist_Max = 0;      
const int Wrist_Straight = 0; //Wrist parallel with arm
const int Wrist_Min = 0;
//Leg1: 41
//Leg2: 42
const int Leg1_Max = 1;     //Legs holding the bot at maximum upright
const int Leg2_Max = 2443;
const int Leg1_Min = 1282;  //Legs parallel with bot (not 180 to max)
const int Leg2_Min = 1315;

//=============================================================



void setup() 
{
  Serial.begin(9600);

  Bot.servoBegin("S1", LEG1);
  Bot.servoBegin("S2", LEG2);
  Bot.servoBegin("S3", CLAW);
  Bot.servoBegin("S4", WRIST);
  

}

void loop() {
  
  current_Millis = millis();
  if((current_Millis - previous_Millis) >= 1000){
    previous_Millis = current_Millis;
    Time_Up_1 = true;
  }
  
  if(Time_Up_1){
    Time_Up_1 = false;
    /*
    0 - Stand up (Lowest servo can go)
    1 - Drive Forward until microcswitch1 detects edge
    2 - Based on distance from bar, calculate the angle the servos
        should be at to reach the bar.
    3 - Begin to exted the arm using the gear motor (fully extended)
    4 - Retract the arm and stop once microswitch2 detectes bar
    5 - Continue to retract the bar and maybe angle the wrist servo
    6 - Once retracted like 70 %, extend again to lower bot onto platform
    7 - A few seconds after microswitch1 detects ground and arm is 
        fully exteded, release the claw
    8 - Retract arm
    9 - Drive to edge
    10 - Drive back
    11 - Signal finished (Maybe just fold legs or have them go 
        up and down)
    */
    switch(Bot_Phase){
      case 0:
      {
        /*
        map(value, fromLow, fromHigh, toLow, toHigh)
              
        value: the number to map.
        fromLow: the lower bound of the value’s current range.
        fromHigh: the upper bound of the value’s current range.
        toLow: the lower bound of the value’s target range.
        toHigh: the upper bound of the value’s target range.
        */
        pos1 = map(Leg1_Max ,0, 4096, 0, 4096);
        pos2 = map(Leg2_Max ,0, 4096, 0, 4096)        
        
        Bot.ToPosition("S1", pos1);
        Bot.ToPosition("S2", pos2);  
        //Serial.println(pos); 
        Bot_Phase = 0;
        break;  
        //42: min: 1315
        // max: 2443
        //41:min:1282
        // max: 1
        /* tester
        max:611
        min:1380
        */        
      }case 1:
      {
        //Straighten the wrist
        Bot.ToPosition("S4", Wrist_Straight);
        Serial.printf("Wrist Straightened");
        Bot_Phase = 2;
        break;
      }case 2:
      {
      
        if(i >= 5){
          Bot.ToPosition("S3", Claw_Closed);
          Serial.printf("Claw Closed");          
          Bot_Phase = 0;
        }
        i++;
        break;
      }    

    }
  }

}
