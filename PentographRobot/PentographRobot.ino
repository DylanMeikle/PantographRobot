/*
Dylan Meikle 
*/
#include <MSE2202_Lib.h>
#include <stdio.h>
#include<math.h>

#define WRIST 2
#define CLAW 45
#define LEG1 41
#define LEG2 42
#define CENTER_MOTOR_A 40
#define CENTER_MOTOR_B 39

#define LEFT_MOTOR_A 35   // GPIO35 pin 28 (J35) Motor 1 A
#define LEFT_MOTOR_B 36   // GPIO36 pin 29 (J36) Motor 1 B
#define RIGHT_MOTOR_A 37  // GPIO37 pin 30 (J37) Motor 2 A
#define RIGHT_MOTOR_B 38  // GPIO38 pin 31 (J38) Motor 2 B

#define ECHO 10
#define TRIG 9

#define MODE_BUTTON 0
#define MSWITCH_1 4  //The Dragging one
#define POT 1

//Uncomment to test motors and servos
//#define TESTING 1

//=============================================================

//Motion Bot_Motors = Motion();
//Motion Bot_Servos = Motion();
Motion Bot = Motion();

//=============================================================

boolean Time_Up = false;

unsigned int current_Millis;
unsigned int previous_Millis;
unsigned int Bot_Phase = 0;
int pos = 0;
//int pos1 = 0;
//int pos2 = 0;
int i = 0;
int Time = 1000;

unsigned char Drive_Speed;
long duration;
int distance;
int Button_State;


//const double Bar_Height = 45;
//const double Gap_Width = 30;
//unsigned double Hyp = sqrt((Bar_Height * Bar_Height) + (Gap_Width * Gap_Width));
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
//2: new min 1710
//1: new min 720
const int Leg1_Max = 2100;  //Legs holding the bot at maximum upright
const int Leg2_Max = 330;
const int Leg1_Min = 720;  //Above the base 330
const int Leg2_Min = 1710;                //2100
const int Leg1_Side = 1220;  //Legs parallel with bot (not 180 to max)
const int Leg2_Side = 1160;
int Scan_Angle1 = Leg1_Max;
int Scan_Angle2 = Leg2_Max;
int Leg1_Bar;  //Angled so that the body directs the pentograph towards the bar
int Leg2_Bar;  // based on gap and height

//=================================================================================

void Legs(int Leg1, int Leg2) {
  //Bot_Servos.ToPosition("S1", pos1);
  //Bot_Servos.ToPosition("S2", pos2);

  Bot.ToPosition("S1", Leg1);
  Bot.ToPosition("S2", Leg2);
}

//=========================================================================================

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

  //Bot.motorBegin("M1", CENTER_MOTOR_A, CENTER_MOTOR_B);
  Bot.servoBegin("S1", LEG1);
  Bot.servoBegin("S2", LEG2);
  Bot.servoBegin("S3", CLAW);
  Bot.servoBegin("S4", WRIST);
  Bot.driveBegin("D1", LEFT_MOTOR_A, LEFT_MOTOR_B, RIGHT_MOTOR_A, RIGHT_MOTOR_B);
  //Bot.driveBegin("D2", CENTER_MOTOR_A, CENTER_MOTOR_B, 15, 16);

  pinMode(MODE_BUTTON, INPUT_PULLUP);
  pinMode(MSWITCH_1, INPUT);
  pinMode(ECHO, INPUT);   //ECHO takes in data
  pinMode(TRIG, OUTPUT);  //TRIG pin outputs data

  Legs(Leg1_Side, Leg2_Side);
  Bot.ToPosition("S4", Wrist_Straight);
  Bot.ToPosition("S3", Claw_Open);
  //add code to have the claw start closed or open
}

//================================================================

void loop() {

  current_Millis = millis();
  if ((current_Millis - previous_Millis) >= Time) {
    previous_Millis = current_Millis;
    Time_Up = true;
  }
  if (Time_Up) {
    Time_Up = false;

    /* Order in which the Robot Phases Should Occur:
    0 - Stand up to initial height                                      - Figure out a good starting point (Maybe set using pot)
    1 - Drive towards edge until microswitch slips off edge               
    2 - Angle the Bot so that the arm is angled towards the upper bar   - Figure out geometry for angle of servo calculation in terms of height and base
    3 - Wrist scans for the bar using the ultrasonic sensor             - Test values of time and distance
    4 - Extend the arm until the ultrasonic sensor is less than 2cm     - Test value of distance
    5 - Close the Claw
    6 - Retract the arm for a few seconds                               - Test time to retract(currently 10 seconds)
    7 -
    */
    Bot_Phase = 1;
#ifndef TESTING
    switch (Bot_Phase) {
      case 0:
        {
          
          Legs(Leg1_Max, Leg2_Max);
          Serial.println("Legs Extended");
          Bot_Phase = 1;

          break;
        }
      case 1:
        {
          //Bot_Motors.Forward("D1", Drive_Speed);
          Bot.Forward("D1", Drive_Speed);
          if (analogRead(MSWITCH_1) == LOW) {
            Bot_Phase = 1;
            //Bot_Motors.Stop("D1");
            Bot.Stop("D1");
            Serial.println("Bot in Position");
          }
          break;
        }
      case 2:
        {
          Legs(Leg1_Max, Leg2_Max);
          Serial.println("Bot Angled");
          Bot_Phase = 3;
          break;
        }
      case 3:   //Originally - Wrist goes at different angles, scanning for bar
                //Final - Wrist angles to be perpendicular to pentograph and then the legs angles so that the 
                // ultrasonic sensor can scan for the bar
        {
          Time = 10;
          //Start at Wrist_Min and end at Wrist_Max
          /*                                                        //This code was removed as we're now using the base servos
          if (Scan_Angle >= Wrist_Max || Scan_Angle <= Wrist_Min) { // instead of the wrist servo
            if (Scan_Angle >= Wrist_Max) {
              Scan_Angle = Scan_Angle + 10;
            } else {
              Scan_Angle = Scan_Angle - 10;
            }
          }*/
          //Figure out how to scan using legs
          if(Scan_Angle1 >= Leg1_Min + 10 && Scan_Angle2 <= Leg2_Min - 10){
            Scan_Angle1 = Scan_Angle1 - 10;
            Scan_Angle2 = Scan_Angle2 + 10;
          }
          
          Legs(Scan_Angle_1, Scan_Angle_2);
          //Bot.ToPosition("S4", Scan_Angle);
          switch (i) {
            case 0:
              {
                digitalWrite(TRIG, LOW);  // Clears the trigPin
                i++;
                break;
              }
            case 1:
              {
                digitalWrite(TRIG, HIGH);
                i++;
                break;
              }
            case 2:
              {
                digitalWrite(TRIG, LOW);
                duration = pulseIn(ECHO, HIGH);   // Reads the echoPin, returns the sound wave travel time in microseconds
                distance = duration * 0.034 / 2;  // Calculating the distance in cm
                if (35 < distance < 65) {
                  Bot_Phase = 4;
                }
                i = 0;
                break;
              }
          }
          break;
        }case 4:
        {
          Time = 10;
          Bot.Forward("M1", Drive_Speed);
          /*                                      //MSWITCH_2 was removed as it was unnecessary
          if(analogRead(MSWITCH_2) == HIGH){      //If the pentograph over extends then it will hit mswitch2 and stop operations
            Bot.Stop("M1");                       // then maybe consider having it retract and restart by scanning for bar.
          }*/
          switch (i) {
            case 0:
            {
              digitalWrite(TRIG, LOW);  // Clears the trigPin
              i++;
              break;
            }
            case 1:
            {
              digitalWrite(TRIG, HIGH);
              i++;
              break;
            }
            case 2:
            {
              digitalWrite(TRIG, LOW);
              duration = pulseIn(ECHO, HIGH);   // Reads the echoPin, returns the sound wave travel time in microseconds
              distance = duration * 0.034 / 2;  // Calculating the distance in cm
              if (distance <= 2) {
                Bot_Phase = 5;
                Bot.Stop("M1");
              }
              i = 0;
              break;
            }
          }
          break;
        }case 5:
        {
          Time = 2000;
          Bot.ToPosition("S3", Claw_Closed);
          Bot_Phase = 6;
          break;
        }case 6:
        {
          Time = 10000;//10 seconds of retracting
          Bot.Reverse("M1", Drive_Speed);
          Bot_Phase = 7;  
          break;      
        }case 7:  //Bot Extends until the mswitch1 touches the ground
        {
          Time = 1000;
          Bot.Forward("M1", Drive_Speed);
          if(MSWITCH_1 == HIGH){
            Bot_Phase = 8;
            Bot.Stop("M1");
          }
          break;
        }case 8:  //Opens claw
        {
          Time = 1000;
          Bot.ToPosition("S3", Claw_Open);
          Bot_Phase = 9;
          break;
        }case 9:
        {
          Time = 3000;
          Bot.Reverse("M1", Drive_Speed);
          Bot_Phase = 10;
          break;
        }case 10:
        {
          Time = 10;
          Bot.Stop("M1");
          Bot.Forward("D1", Drive_Speed);
          if(MSWITCH_1 == LOW){
            Bot_Phase = 11;
            Bot.Stop("D1");            
          }    
          break;     
        }case 11:
        {
          Time = M_PI * 6.5 * Drive_Speed;
          Bot.Reverse("D1", Drive_Speed);
          break;
        }case 12: //Lays down the robot to signal finish
        {
          Time = 1000;
          Bot.Stop("D1");
          Legs(Leg1_Side, Leg2_Side);
          Bot_Phase = 13;
          break;          
        }case 13:
        {
          Legs(Leg1_Side, Leg2_Side);
          break;
        }

    }
#else
    /* TESTING CODE SECTION
    This section of code runs when TESTING is defined (found at the bottom of all the #define statements).
    What happens is you have to choose what you want to test by typing it's corresponding number into
    the serial monitor. When you're done testing one compoenent you can press reset on your board and then
    you will be prompted to choose another component to test. List of components and their corresponding numbers below.
    */
    /*
          0 - Test Values for Leg 1 using POT     - Working
          1 - Test Values for Leg 2               - Not Working - Too many channels in use?
          2 - Claw Values                         - Working
          3 - Wrist Values                        - Working
          4 - Pentograph Rod Motor                - Not Working - Why the fuck doesn't M1 work?
          5 - Wheels                              - Working
          6 - Ultrasonic sensor                   - Working 
          7 - Microswitches                       - Working
    */
    //Reset to change case
    if (Bot_Phase == 0) {
      Serial.println("Enter case #");
      while (Serial.available() == 0) {
        Bot_Phase = Serial.parseInt();
      }
      Serial.println(Bot_Phase);
    }

    Drive_Speed = map(analogRead(POT), 0, 4096, 150, 255);
    switch (Bot_Phase) {
      case 0:
        {
          Serial.printf("Testing Servo #1 (Pin %d): ", LEG1);
          pos = map(analogRead(POT), 0, 4096, 330, 2100);
          //Bot_Servos.ToPosition("S1", pos);
          Bot.ToPosition("S1", pos);
          Serial.println(analogRead(POT));
          break;
        }
      case 1:
        {
          Serial.printf("Testing Servo #2 (Pin %d): ", LEG2);
          pos = map(analogRead(POT), 0, 4096, Leg2_Min, Leg2_Max);
          //Bot_Servos.ToPosition("S2", pos);
          Bot.ToPosition("S2", pos);
          Serial.println(analogRead(POT));
          break;
        }
      case 2:
        {
          Serial.printf("Testing Claw (Pin %d): ", CLAW);
          pos = map(analogRead(POT), 0, 4096, Claw_Closed, Claw_Open);
          //Bot_Servos.ToPosition("S3", pos);
          Bot.ToPosition("S3", pos);
          Serial.println(pos);
          break;
        }
      case 3:
        {
          Serial.printf("Testing Wrist (Pin %d): ", WRIST);
          pos = map(analogRead(POT), 0, 4096, Wrist_Min, Wrist_Max);
          //Bot_Servos.ToPosition("S4", pos);
          Bot.ToPosition("S4", pos);
          Serial.println(pos);
          break;
        }
      case 4:
        {
          /*
          Serial.printf("Testing Pentograph Motor (Pins %d & %d): ", CENTER_MOTOR_B, CENTER_MOTOR_A);
          //Drive_Speed = map(analogRead(POT), 0, 4096, 150, 255);
          //Bot_Motors.Forward("M1", Drive_Speed);
          Bot.Forward("M1", Drive_Speed);
          Serial.println(millis() / 1000);*/
          Bot.Forward("D1", Drive_Speed);
          //Bot.Forward("M1", Drive_Speed);
          /*
          while(i != 2){
            Motor_Direction = Serial.read();
            Serial.println(Motor_Direction);
            if(Motor_Direction == 'F')
            {
              Bot.Forward("M1", Drive_Speed);
            }else if (Motor_Direction == 'R'){
              Bot.Reverse("M1", Drive_Speed);
            }else if(Motor_Direction == 'S'){
              break;
            }
          }*/
          break;
        }
      case 5:
        {
          //Serial.printf("Wheels (Pins %d, %d, %d, %d): ", LEFT_MOTOR_A, LEFT_MOTOR_B, RIGHT_MOTOR_A, RIGHT_MOTOR_B);
          //Drive_Speed = map(analogRead(POT), 0, 4096, 150, 255);
          //Bot.Forward("D1", Drive_Speed);
          //Serial.println(Drive_Speed);
          Serial.println(i);
          switch (i)  // Cycle through drive states
          {
            case 0:  // Stop
              {

                //Bot_Motors.Stop("D1");                                       // Drive ID
                Bot.Stop("D1");
                i = 1;  // Next state: drive forward
                break;
              }
            case 1:  // Drive forward
              {

                //Bot_Motors.Forward("D1", Drive_Speed, Drive_Speed);          // Drive ID, Left speed, Right speed
                Bot.Forward("D1", Drive_Speed, Drive_Speed);
                i = 2;  // Next state: drive backward
                break;
              }
            case 2:  // Drive backward
              {

                //Bot_Motors.Reverse("D1", Drive_Speed);                       // Drive ID, Speed (same for both)
                Bot.Reverse("D1", Drive_Speed);
                i = 3;  // Next state: turn left
                break;
              }
            case 3:  // Turn left (counterclockwise)
              {

                //Bot_Motors.Left("D1", Drive_Speed);                          // Drive ID, Speed (same for both)
                Bot.Left("D1", Drive_Speed);
                i = 4;  // Next state: turn right
                break;
              }
            case 4:  // Turn right (clockwise)
              {

                //Bot_Motors.Right("D1", Drive_Speed);                         // Drive ID, Speed (same for both)
                Bot.Right("D1", Drive_Speed);
                i = 0;  // Next state: stop
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
        }case 7:
        {
          Serial.printf("Microswitch (Pin %d): ", MSWITCH_1);
          Button_State = digitalRead(MSWITCH_1);

          //Serial.println(Button_State);
          if (Button_State == HIGH) {
            Serial.println("HIGH");
          } else {
            Serial.println("LOW");
          }
          break;
        }
        
    }
#endif
  }
}
