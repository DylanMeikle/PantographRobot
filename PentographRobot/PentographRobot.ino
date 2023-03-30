#include <MSE2202_Lib.h>
#include <stdio.h>
//#include<math.h>

#define WRIST           2
#define CLAW            45
#define LEG1            41
#define LEG2            42
#define CENTER_MOTOR_A  40
#define CENTER_MOTOR_B  39

#define LEFT_MOTOR_A    35   // GPIO35 pin 28 (J35) Motor 1 A
#define LEFT_MOTOR_B    36   // GPIO36 pin 29 (J36) Motor 1 B
#define RIGHT_MOTOR_A   37  // GPIO37 pin 30 (J37) Motor 2 A
#define RIGHT_MOTOR_B   38  // GPIO38 pin 31 (J38) Motor 2 B

#define ECHO            10
#define TRIG            9

#define MODE_BUTTON     0
#define MSWITCH_1       4  //The Dragging one
#define MSWITCH_2       5  //The one to stop the pentograph exterder
#define MSWITCH_3       6  //The one in the Claw
#define MSWITCH_4       7  //
#define POT             1

//Uncomment to test motors and servos
#define TESTING 1

//=============================================================

//Motion Bot_Motors = Motion();
//Motion Bot_Servos = Motion();
Motion Bot = Motion();

//Remember to add encoder code so that you can measure
// distance travelled
Encoders driveEncoders = Encoders();

//=============================================================

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

unsigned char Drive_Speed;
unsigned int ui_Mode_PB_Debounce = 0;
long duration;
int distance;
int Button_State;

const unsigned int Bar_Height = 45;
const unsigned int Gap_Width = 30;
//unsigned int angle = arctan(Bar_Height/Gap_Width);


//=============================================================
//Values for Servos
//Still need to test all 0 values
const int Claw_Open = 490;
const int Claw_Closed = 1890;
const int Wrist_Max = 1490;
const int Wrist_Straight = 800;  //Wrist parallel with arm
const int Wrist_Min = 290;
//Leg1: 41
//Leg2: 42
const int Leg1_Max = 2100;  //Legs holding the bot at maximum upright
const int Leg2_Max = 330;
const int Leg1_Min = 330;  //Above the base
const int Leg2_Min = 2100;
const int Leg1_Side = 1290;  //Legs parallel with bot (not 180 to max)
const int Leg2_Side = 1175;
int Leg1_Bar;  //Angled so that the body directs the pentograph towards the bar
int Leg2_Bar;  // based on gap and height




void setup() {
  Serial.begin(9600);

  //For some reason the motors only work with put at the top
  // so that may have something to do with there being a limited
  // # of channels but we'll see
  //Bot_Motors.driveBegin("D1", LEFT_MOTOR_A, LEFT_MOTOR_B, RIGHT_MOTOR_A, RIGHT_MOTOR_B);
  //Bot_Motors.motorBegin("M1", CENTER_MOTOR_A, CENTER_MOTOR_B);
  //Bot_Servos.servoBegin("S1", LEG1);
  //Bot_Servos.servoBegin("S2", LEG2);
  //Bot_Servos.servoBegin("S3", CLAW);
  //Bot_Servos.servoBegin("S4", WRIST);
  
  Bot.motorBegin("M1", CENTER_MOTOR_A, CENTER_MOTOR_B);
  Bot.driveBegin("D1", LEFT_MOTOR_A, LEFT_MOTOR_B, RIGHT_MOTOR_A, RIGHT_MOTOR_B);
  Bot.servoBegin("S1", LEG1);
  Bot.servoBegin("S2", LEG2);
  
  pinMode(MODE_BUTTON, INPUT_PULLUP);
  pinMode(MSWITCH_1, INPUT);
  pinMode(ECHO, INPUT);   //ECHO takes in data
  pinMode(TRIG, OUTPUT);  //TRIG pin outputs data
  //Legs(Leg1_Side, Leg2_Side);
  //add code to have the claw start closed or open
}
void Legs(int Leg1, int Leg2) {
  pos1 = map(Leg1, 0, 4096, Leg1_Min, Leg1_Max);
  pos2 = map(Leg2, 0, 4096, Leg2_Min, Leg2_Max);

  //Bot_Servos.ToPosition("S1", pos1);
  //Bot_Servos.ToPosition("S2", pos2);
  Bot.ToPosition("S1", pos1);    
  Bot.ToPosition("S2", pos2);

}

void loop() {

  current_Millis = millis();
  if ((current_Millis - previous_Millis) >= 2000) {
    previous_Millis = current_Millis;
    Time_Up_1 = true;
  }
  if (Time_Up_1) {
    Time_Up_1 = false;
    /* Order in which the Robot Phases Should Occur:
    0 - Stand up to initial height
    1 - Drive towards edge until microswitch slips off edge
    2 - Angle the Bot so that the arm is angled towards the upper bar
    3 - 
    */
#ifndef TESTING
    switch (Bot_Phase) {
      case 0:
        {
          //Maybe find a good initial value that isn't max
          Legs(Leg1_Max, Leg2_Max);
          Serial.printf("Legs Extended\n");
          Bot_Phase = 1;

          break;
        }
      case 1:
        {
          //Bot_Motors.Forward("D1", Drive_Speed);
          Bot.Forward("D1", Drive_Speed);
          //Test the microswitch to see what values it outputs when pushed
          // for now assuming OFF means not pressed
          if (analogRead(MSWITCH_1) == HIGH) {
            Bot_Phase = 2;
            //Bot_Motors.Stop("D1");
            Bot.Stop("D1");
          }
          break;
        }
      case 2:
        {
          //Change Distances of gap and height to include the bots actual
          // body dimensions
          Legs(Leg1_Bar, Leg2_Bar);
          Serial.println("Bot Angled");
          Bot_Phase = 3;
          break;
        }
      case 3:
        {
          //Straighten the wrist
          //Bot_Servos.ToPosition("S4", Wrist_Straight);
          Serial.println("Wrist Straightened");
          Bot_Phase = 4;
          break;
        }
      case 4:
        {
          //Bot_Motors.Forward("M1", Drive_Speed);
          Bot.Forward("M1", Drive_Speed);
          if (analogRead(MSWITCH_2) == HIGH) {
            Bot_Phase = 6;
            //Bot_Motors.Stop("M1");
            Bot.Stop("M1");
          }
          break;
        }
      case 5:
        {
          //Bot_Servos.ToPosition("S3", Claw_Closed);
          Serial.println("Claw Closed");
          if (analogRead(MSWITCH_1) == HIGH) {
            Bot_Phase = 6;
          }

          break;
        }
      case 6:
        {
          //Bot_Servos.ToPosition("S3", Claw_Open);
          Bot_Phase = 7;
          break;
        }
    }
#else
    //Testing Parts

    //Reset to change case
    while (Serial.available() == 0) {
      Serial.println("Enter case #");
      Bot_Phase = Serial.parseInt();
      Serial.println(Bot_Phase);
    }
    Drive_Speed = map(analogRead(POT), 0, 4096, 150, 255);
    switch (Bot_Phase) {

      /*
      0 - Test Values for Leg 1 using POT
      1 - Test Values for Leg 2
      2 - Claw Values
      3 - Wrist Values
      4 - Pentograph Rod Motor
      5 - Wheels
      6 - Ultrasonic sensor
      7 - Microswitches - maybe
      */
      case 0:
        {
          Serial.printf("Testing Servo #1 (Pin %d): ", LEG1);
          pos = map(analogRead(POT), 0, 4096, 330, 2100);
          //Bot_Servos.ToPosition("S1", pos);
          Bot.ToPosition("S1", pos);
          Serial.println(pos);
          break;
        }
      case 1:
        {
          Serial.printf("Testing Servo #2 (Pin %d): ", LEG2);
          pos = map(analogRead(POT), 0, 4096, Leg2_Min, Leg2_Max);
          //Bot_Servos.ToPosition("S2", pos);
          Bot.ToPosition("S2", pos);
          Serial.println(pos);
          break;
        }
      case 2:
        {
          Serial.printf("Testing Claw (Pin %d): ", CLAW);
          pos = map(analogRead(POT), 0, 4096, Claw_Closed, Claw_Open);
          //Bot_Servos.ToPosition("S3", pos);
          Serial.println(pos);
          break;
        }
      case 3:
        {
          Serial.printf("Testing Wrist (Pin %d): ", WRIST);
          pos = map(analogRead(POT), 0, 4096, Wrist_Min, Wrist_Max);
          //Bot_Servos.ToPosition("S4", pos);
          Serial.println(pos);
          break;
        }
      case 4:
        {
          Serial.printf("Testing Pentograph Motor (Pins %d & %d): ", CENTER_MOTOR_B, CENTER_MOTOR_A);
          //Drive_Speed = map(analogRead(POT), 0, 4096, 150, 255);
          //Bot_Motors.Forward("M1", Drive_Speed);
          Bot.Forward("M1", Drive_Speed);
          Serial.println(millis() / 1000);
          break;
        }
      case 5:
        {
          //Serial.printf("Wheels (Pins %d, %d, %d, %d): ", LEFT_MOTOR_A, LEFT_MOTOR_B, RIGHT_MOTOR_A, RIGHT_MOTOR_B);
          //Drive_Speed = map(analogRead(POT), 0, 4096, 150, 255);
          //Bot.Forward("D1", Drive_Speed);
          //Serial.println(Drive_Speed);
          switch(i)                                                  // Cycle through drive states
          {
            case 0:                                                   // Stop
            {
              Serial.println(i);
              //Bot_Motors.Stop("D1");                                       // Drive ID
              Bot.Stop("D1");
              i = 1;                                                // Next state: drive forward
              break;
            }case 1:                                                // Drive forward
            {
              Serial.println(i);
              //Bot_Motors.Forward("D1", Drive_Speed, Drive_Speed);          // Drive ID, Left speed, Right speed
              Bot.Forward("D1", Drive_Speed, Drive_Speed);
              i = 2;                                                // Next state: drive backward
              break;
            }case 2:                                                // Drive backward
            {
              Serial.println(i);
              //Bot_Motors.Reverse("D1", Drive_Speed);                       // Drive ID, Speed (same for both)
              Bot.Reverse("D1", Drive_Speed); 
              i = 3;                                                // Next state: turn left
              break;
            }case 3:                                                // Turn left (counterclockwise)
            {
              Serial.println(i);
              //Bot_Motors.Left("D1", Drive_Speed);                          // Drive ID, Speed (same for both)
              Bot.Left("D1", Drive_Speed);
              i = 4;                                                // Next state: turn right
              break;
            }case 4:                                                // Turn right (clockwise)
            {
              Serial.println(i);
              //Bot_Motors.Right("D1", Drive_Speed);                         // Drive ID, Speed (same for both)
              Bot.Right("D1", Drive_Speed);
              i = 0;                                                // Next state: stop
              break;
            }
          }
          break;
        }
      case 6:
        {
          switch (i) {
            case 0:
              {
                Serial.printf("Ultrasonic Sensor Distance(cm): ");
                // Clears the trigPin
                digitalWrite(TRIG, LOW);
                i++;
                break;
              }
            case 1:
              {
                // Sets the trigPin on HIGH state for 10 micro seconds
                digitalWrite(TRIG, HIGH);
                i++;
                break;
              }
            case 2:
              {
                digitalWrite(TRIG, LOW);
                // Reads the echoPin, returns the sound wave travel time in microseconds
                duration = pulseIn(ECHO, HIGH);
                // Calculating the distance in cm
                distance = duration * 0.034 / 2;
                // Prints the distance on the Serial Monitor
                Serial.println(distance);
                i = 0;
                break;
              }
          }
          break;
        }
      case 7:
        {
          Serial.printf("Microswitch (Pin %d): ", MSWITCH_1);
          Button_State = digitalRead(MSWITCH_1);
          //Serial.println(Button_State);
          if (Button_State == HIGH) {
            Serial.println("HIGH");
          } else {
            Serial.println("LOW");
          }
        }
    }
#endif
  }
}
