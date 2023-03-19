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
int pos1 = 0;
int pos2 = 0;
int i = 0;
int a;
int b;

//=============================================================
//Values for Servos
//Still need to test all 0 values
const int Claw_Open = 490;    
const int Claw_Closed = 1890;
const int Wrist_Max = 1490;      
const int Wrist_Straight = 800; //Wrist parallel with arm
const int Wrist_Min = 290;
//Leg1: 41
//Leg2: 42
const int Leg1_Max = 400;     //Legs holding the bot at maximum upright
const int Leg2_Max = 2100;
const int Leg1_Min = 1282;  //Legs parallel with bot (not 180 to max)
const int Leg2_Min = 1315;



//=============================================================
//=============================================================
/* Notes:
map(value, fromLow, fromHigh, toLow, toHigh)
              
value: the number to map.
fromLow: the lower bound of the value’s current range.
fromHigh: the upper bound of the value’s current range.
toLow: the lower bound of the value’s target range.
toHigh: the upper bound of the value’s target range.
*/
//============================================================

void setup() 
{
  Serial.begin(9600);

  Bot.servoBegin("S1", LEG1);
  Bot.servoBegin("S2", LEG2);
  Bot.servoBegin("S3", CLAW);
  Bot.servoBegin("S4", WRIST);
  
  //Legs(Leg1_Min, Leg2_Min);
  //add code to have the claw start closed or open

}
void Legs(int Leg1, int Leg2){
  int pos1 = map(Leg1 ,0, 4096, 0, 4096);
  int pos2 = map(Leg2 ,0, 4096, 0, 4096);      
        
  Bot.ToPosition("S1", pos1);
  Bot.ToPosition("S2", pos2); 
}

void loop() {
  
  current_Millis = millis();
  if((current_Millis - previous_Millis) >= 2000){
    previous_Millis = current_Millis;
    Time_Up_1 = true;
  }
  
  if(Time_Up_1){
    Time_Up_1 = false;
    /* Order in which the Robot Phases Should Occur:
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
        Legs(Leg1_Max, Leg2_Max); 
        Serial.printf("Legs Extended\n");
        Bot_Phase = 1;
        
        break;  
              
      }case 1:
      {
        //Straighten the wrist
        Bot.ToPosition("S4", Wrist_Straight);
        Serial.printf("Wrist Straightened\n");
        Bot_Phase = 2;
        break;
      }case 2:
      {
        
        Bot.ToPosition("S3", Claw_Closed);
        Serial.printf("Claw Closed\n");          
        Bot_Phase = 0;
        
        
      }    

    }
  }

}
