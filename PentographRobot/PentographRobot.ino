/*
Dylan Meikle 
*/
#include <MSE2202_Lib.h>
#include <stdio.h>

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

#define MSWITCH_1 4  
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
int i = 0;
int Time = 1000;
int D = 50;   //Distance to travel back from second edge

unsigned char Drive_Speed = 100;
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
const int Claw_Closed = 830;
const int Wrist_Max = 1490;
const int Wrist_Straight = 800;  //Wrist parallel with arm
const int Wrist_Min = 290;
//Leg1: 41
//Leg2: 42
const int Leg1_Max = 2100;  //Legs holding the bot at maximum upright
const int Leg2_Max = 330;
const int Leg1_Min = 720;  //Above the base 
const int Leg2_Min = 1710;                
const int Leg1_Side = 1220;  //Legs parallel with bot 
const int Leg2_Side = 1160;
int Scan_Angle1 = Leg1_Max;
int Scan_Angle2 = Leg2_Max;
//=================================================================================

void Legs(int Leg1, int Leg2) {
  //Bot_Servos.ToPosition("S1", pos1);
  //Bot_Servos.ToPosition("S2", pos2);

  Bot.ToPosition("S1", Leg1);
  Bot.ToPosition("S2", Leg2);
}
int Sonic_Sensor(){
  switch (i) {
    case 0:
    {
      digitalWrite(TRIG, LOW);  // Clears the trigPin
      i = 1;
      break;
    }case 1:
    {
      digitalWrite(TRIG, HIGH);
      i = 2;
      break;
    }case 2:
    {
      digitalWrite(TRIG, LOW);
      duration = pulseIn(ECHO, HIGH);   // Reads the echoPin, returns the sound wave travel time in microseconds
      distance = duration * 0.034 / 2;  // Calculating the distance in cm
      return distance;
      i = 0;
      break;
    }
  }
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

  Bot.motorBegin("M1", CENTER_MOTOR_A, CENTER_MOTOR_B);
  Bot.driveBegin("D1", LEFT_MOTOR_A, LEFT_MOTOR_B, RIGHT_MOTOR_A, RIGHT_MOTOR_B);
  Bot.servoBegin("S1", LEG1);
  Bot.servoBegin("S2", LEG2);
  Bot.servoBegin("S3", CLAW);
  Bot.servoBegin("S4", WRIST);
  

  
  pinMode(MSWITCH_1, INPUT);
  pinMode(ECHO, INPUT);   //ECHO takes in data
  pinMode(TRIG, OUTPUT);  //TRIG pin outputs data

  Legs(Leg1_Side, Leg2_Side);             //Sets legs so that they start parallel to the bot
  Bot.ToPosition("S4", Wrist_Min);        //Sets the wrist so that it's at it's lowest position
  Bot.ToPosition("S3", Claw_Closed);      //Sets the claw to start closed
  
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
    0 - Stand up until the microswitch is depressed                     - Maybe have servo go a bit further just to make sure it is fully depressed (like me)    
    1 - Drive towards edge until microswitch slips off edge               
    2 - Angle the bot untilt the ultrasonic sensor detects the bar
    3 - Extend the arm until the ultrasonic sensor is less than 2cm     - Test value of distance
    4 - Close the Claw
    5 - Retract the arm for a few seconds                               - Test time to retract(currently 10 seconds)
    6 - Extend the arm until the microswitch is depressed
    7 - Open the Claw
    8 - Retract the arm
    9 - Drive towards the second edge until the microswitch slips off
    10 - Drive backwards to a speficied distance from edge              - Test the timing
    11 - Bot lays down to signal end and prints the time of operations to serial monitor
    */
    
#ifndef TESTING
    switch (Bot_Phase) {
      case 0://Bot extends the legs until the microswitch is depressed (Which should be Leg#_Max)
        {
          Time = 10;
          if(Scan_Angle1 >= Leg1_Min + 10 && Scan_Angle2 <= Leg2_Min - 10){
            Scan_Angle1 = Scan_Angle1 - 10;
            Scan_Angle2 = Scan_Angle2 + 10;
          }
          Legs(Scan_Angle1, Scan_Angle2);
          if(analogRead(MSWITCH_1) == HIGH){  //Once the bot is angled far enough to depress the microswitch
            Serial.println("Legs Extended");
            Bot_Phase = 1;
          }
          break;
        }case 1://Bot Drives forwards until it detects the edge via the microswitch opening upon passing the edge
        {
          Time = 10;
          //Bot_Motors.Forward("D1", Drive_Speed);
          Bot.Forward("D1", Drive_Speed);
          if (analogRead(MSWITCH_1) == LOW) { //If the microswitch passes the edge of the table
            Bot_Phase = 2;
            //Bot_Motors.Stop("D1");
            Bot.Stop("D1");
            Serial.println("Bot in Position");
          }
          break;
        }case 2:  //Originally - Wrist goes at different angles, scanning for bar
                  //Final - Wrist angles to be perpendicular to pentograph and then the legs angles so that the 
                  // ultrasonic sensor can scan for the bar - maybe add code so that if it makes one pass without seeing
                  // it, it will go back and forth until the bar is detected or even reposition itself.
        {
          Time = 10;
          Bot.ToPosition("S4", Wrist_Straight);
          if(Scan_Angle1 >= Leg1_Min + 10 && Scan_Angle2 <= Leg2_Min - 10){
            Scan_Angle1 = Scan_Angle1 - 10;
            Scan_Angle2 = Scan_Angle2 + 10;
          }
          Legs(Scan_Angle1, Scan_Angle2);
          distance = Sonic_Sensor();
          if (35 < distance < 65) {
            Bot_Phase = 4;
          }
          break;
        }case 3:
        {
          Time = 10;
          Bot.Forward("M1", Drive_Speed);
          distance = Sonic_Sensor();
          if (distance <= 2) {
            Bot_Phase = 5;
            Bot.Stop("M1");
          }
          break;
        }case 4:
        {
          Time = 2000;
          Bot.ToPosition("S3", Claw_Closed);
          Bot_Phase = 5;
          break;
        }case 5:
        {
          Time = 10000;//10 seconds of retracting
          Bot.Reverse("M1", Drive_Speed);
          Bot_Phase = 6;  
          break;      
        }case 6:  //Bot Extends until the mswitch1 touches the ground
        {
          Time = 1000;
          Bot.Forward("M1", Drive_Speed);
          if(MSWITCH_1 == HIGH){
            Bot_Phase = 7;
            Bot.Stop("M1");
          }
          break;
        }case 7:  //Opens claw
        {
          Time = 1000;
          Bot.ToPosition("S3", Claw_Open);
          Bot_Phase = 8;
          break;
        }case 8:  //Retract the arm
        {
          Time = 3000;
          Bot.Reverse("M1", Drive_Speed);
          Bot_Phase = 9;
          break;
        }case 9:  //Drive to the next edge (until the microswitch passes the edge)
        {
          Time = 10;
          Bot.Stop("M1");
          Bot.Forward("D1", Drive_Speed);
          if(MSWITCH_1 == LOW){
            Bot_Phase = 10;
            Bot.Stop("D1");            
          }    
          break;     
        }case 10: //Drive backwards for *Time* seconds (until it reaches the distance specified by prof)
        {
          Time = D /(M_PI * 6.5 * Drive_Speed);  //Calculates the time required for the bot to travel the distance specified by prof.
          Bot.Reverse("D1", Drive_Speed);
          break;
        }case 11: //Lays down the robot to signal finish
        {
          Time = 1000;
          Bot.Stop("D1");
          Legs(Leg1_Side, Leg2_Side);
          Serial.print("Time for completion of operations (In seconds): ");
          Serial.println(millis()/1000);
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
          4 - Pentograph Rod Motor                - Not Working - Why the doesn't M1 work? Wired wrong maybe
          5 - Wheels                              - Working
          6 - Ultrasonic sensor                   - Working 
          7 - Microswitches                       - Working
    */
    if (Bot_Phase == 0) {             //This part takes user input to choose which testing environment
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
          Serial.println(Drive_Speed);
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
                delay(3000);
                i = 2;  // Next state: drive backward
                break;
              }
            case 2:  // Drive backward
              {

                //Bot_Motors.Reverse("D1", Drive_Speed);                       // Drive ID, Speed (same for both)
                Bot.Reverse("D1", Drive_Speed);
                delay(3000);
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
