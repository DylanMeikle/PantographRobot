#include <MSE2202_Lib.h>

#define WRIST             2
#define CLAW              45
#define LEG1              41
#define LEG2              42
#define CENTER_MOTOR_A    40
#define CENTER_MOTOR_B    39

#define LEFT_MOTOR_A      35  // GPIO35 pin 28 (J35) Motor 1 A
#define LEFT_MOTOR_B      36  // GPIO36 pin 29 (J36) Motor 1 B
#define RIGHT_MOTOR_A     37  // GPIO37 pin 30 (J37) Motor 2 A
#define RIGHT_MOTOR_B     38  // GPIO38 pin 31 (J38) Motor 2 B

#define MODE_BUTTON       0
#define MSWITCH           10
#define POT               1

//#define TESTING           1


Motion Bot = Motion();
//Remember to add encoder code so that you can measure
// distance travelled
Encoders driveEncoders = Encoders();



boolean Time_Up_1 = false;
boolean Time_Up_2 = false;
boolean Time_Up_3 = false;

unsigned int current_Millis;
unsigned int previous_Millis;
unsigned int Bot_Phase = 0;
int pos = 0;
int pos1 = 0;
int pos2 = 0;
int i = 0;
int a;
int b;
unsigned char Drive_Speed;
unsigned int ui_Mode_PB_Debounce = 0;

const unsigned int Bar_Height = 45;
const unsigned int Gap_Width = 30;
unsigned int angle = arctan(Bar_Height/Gap_Width);


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
const int Leg1_Min = 0;       //Above the base
const int Leg2_Min = 0;
const int Leg1_Side = 1282;   //Legs parallel with bot (not 180 to max)
const int Leg2_Side = 1315;



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
  Bot.motorBegin("M1", CENTER_MOTOR_A, CENTER_MOTOR_B);
  Bot.driveBegin("D1", LEFT_MOTOR_A, LEFT_MOTOR_B, RIGHT_MOTOR_A, RIGHT_MOTOR_B);
  
  pinMode(MODE_BUTTON, INPUT_PULLUP);  
  //Legs(Leg1_Side, Leg2_Side);
  //add code to have the claw start closed or open

}
void Legs(int Leg1, int Leg2){
  pos1 = map(Leg1 ,0, 4096, Leg1_Min, Leg1_Mas);
  pos2 = map(Leg2 ,0, 4096, Leg2_Min, Leg2_Max);      
        
  Bot.ToPosition("S1", pos1);
  Bot.ToPosition("S2", pos2); 
}

void loop() {
  
  current_Millis = millis();
  if((current_Millis - previous_Millis) >= 2000){
    previous_Millis = current_Millis;
    Time_Up_1 = true;

      // Mode pushbutton debounce and toggle
      if(!digitalRead(MODE_BUTTON))                                           // If pushbutton GPIO goes LOW (nominal push)
      {
         // Start debounce
         if(ui_Mode_PB_Debounce <= 25)                                        // 25 millisecond debounce time
         {
            ui_Mode_PB_Debounce = ui_Mode_PB_Debounce + 1;                    // Increment debounce timer count
            if(ui_Mode_PB_Debounce > 25)                                      // If held for at least 25 mS
            {
               ui_Mode_PB_Debounce = 1000;                                    // Change debounce timer count to 1 second
            }
         }
         if(ui_Mode_PB_Debounce >= 1000)                                      // Maintain 1 second timer count until release
         {
            ui_Mode_PB_Debounce = 1000;
         }
      }
      else                                                                    // Pushbutton GPIO goes HIGH (nominal release)
      {
         if(ui_Mode_PB_Debounce <= 26)                                        // If release occurs within debounce interval
         {
            ui_Mode_PB_Debounce = 0;                                          // Reset debounce timer count
         }
         else
         {
            ui_Mode_PB_Debounce = ui_Mode_PB_Debounce + 1;                    // Increment debounce timer count
            if(ui_Mode_PB_Debounce >= 1025)                                   // If pushbutton was released for 25 mS
            {
               ui_Mode_PB_Debounce = 0;                                       // Reset debounce timer count
               Bot_Phase++;                                         // Switch to next mode
               Bot_Phase = Bot_Phase & Phase_Number;                 // Keep mode index between 0 and 7
                                                        
            }
         }
      }    
  }
  
  if(Time_Up_1){
    Time_Up_1 = false;
    /* Order in which the Robot Phases Should Occur:
    0 - Stand up to initial height
    1 - Drive towards edge until microswitch slips off edge
    2 - Angle the Bot so that the arm is angled towards the upper bar
    3 - 
    */
#ifndef TESTING
    switch(Bot_Phase){
      case 0:
      {
        //Maybe find a good initial value that isn't max
        Legs(Leg1_Max, Leg2_Max); 
        Serial.printf("Legs Extended\n");
        Bot_Phase = 1;
        
        break;  
              
      }case 1:
      {
        Bot.Forward("D1", Drive_Speed);
        //Test the microswitch to see what values it outputs when pushed
        // for now assuming OFF means not pressed
        if(analogRead(MSWITCH) == OFF){
          Bot_Phase = 2;
          Bot.Stop("D1");
        }
        break;
      }case 2:
      {
        
        Legs();
      }
      case 2:
      {
        //Straighten the wrist
        Bot.ToPosition("S4", Wrist_Straight);
        Serial.printf("Wrist Straightened\n");
        Bot_Phase = 2;
        break;
      }case 3:
      {
        Bot.ToPosition("S3", Claw_Closed);
        Serial.printf("Claw Closed\n");          
        Bot_Phase = 3;
        break;
      }case 4:
      {
        Bot.ToPosition("S3", Claw_Open);
        Bot_Phase = 4;
        break;        
      }

    }
#else
    //Testing Parts
    switch(Bot_Phase){
      /*
      0 - Test Values for Leg 1 using POT
      1 - Test Values for Leg 2
      2 - Claw Values
      3 - Wrist Values
      4 - Pentograph Rod Motor
      5 - 
      */
      case 0:
      {
        pos = map(analogRead(POT), 0, 4096, Leg1_Min, Leg1_Max); 
        Bot.ToPosition("S1", pos);
        Serial.println(pos);
        
        break;
      }case 1:
      {
        pos = map(analogRead(POT), 0, 4096, Leg2_Min, Leg2_Max); 
        Bot.ToPosition("S2", pos);
        Serial.println(pos);
        
        break;
      }case 2:
      {
        pos = map(analogRead(POT), 0, 4096, Claw_Closed, Claw_Open); 
        Bot.ToPosition("S3", pos);
        Serial.println(pos);
        break;
      }case 3:
      {
        pos = map(analogRead(POT), 0, 4096, Wrist_Min, Wrist_Max); 
        Bot.ToPosition("S4", pos);
        Seria.println(pos);
        break;
      }case 4:
      {
        Drive_Speed = map(analogRead(POT), 0, 4096, 150, 255); 
        Bot.Forward("M1", Drive_Speed);
        Serial.println(millis()/1000);
        break;
      }case 5:
      {
        Drive_Speed = map(analogRead(POT), 0, 4096, 150, 255);   
        Bot.Forward("D1", Drive_Speed);
        Serial.println(Drive_Speed);
        break;
      }
    }
#endif
  }

}
