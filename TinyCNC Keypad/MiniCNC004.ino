/* Based on MiniCNC Arduino sketch by Oliv4945 
It kinda works, mostly spazzy
*/

#include <Servo.h>
//  Library from here - http://playground.arduino.cc/Code/Keypad#Download
#include <Keypad.h>

//  Keypad Registration Stuff, cribbed from https://www.adafruit.com/products/419
//  Wiring diagram here - http://www.adafruit.com/images/large/membranekeypad34arduino_LRG.jpg
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {8, 7, 6, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 3, 2}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


//  Servo Control Stuff
#define SERVOPINX 11
#define SERVOPINY 10
#define SERVOPINZ 9
#define LINE_BUFFER_LENGTH 512

boolean invertmotors = true;

/* Structures, global variables    */
struct point { 
  float x; 
  float y; 
  float z; 
};

/* Naming servos    */
Servo servoX;
Servo servoY;
Servo servoZ;
struct point actuatorPos;

//  Gear settings
float gearD = 56;
float ZgearD = 13.3239;

//  Drawing robot limits, in degrees
float lineInc = mm2steps(5);

float Xmin = 0;     //  8.31mm
float Xmax = 255;   //  83.57mm
float Ymin = 10;    //  Functionally 18 degrees  //  12.22mm
float Ymax = 255-100;  //  Functionally 146        //  71.35mm
float Zmin = 0;  
float Zmax = 255;

float Xpos = Xmin;
float Ypos = Ymax;
float Zpos = Zmin; 

boolean verbose = true;

/**********************
 * void setup() - Initialisations
 ***********************/
void setup() {
  //  Setup
  Serial.begin( 115200 );
  //  Attach servos
  servoX.attach( SERVOPINX );
  servoY.attach( SERVOPINY );
  servoZ.attach( SERVOPINZ );
  //  Set & move to initial default position
  servoX.write(Xpos);
  servoY.write(Ypos);
  servoZ.write(Zpos);
  //  Notifications!!!
  Serial.println("Robot Team Go!!!");
}

/**********************
 * void loop() - Main loop
 ***********************/
void loop() 
{
char key = keypad.getKey();
if (key != NO_KEY)
  { 
  delay(1);
  //  Serial.println(key); 
  if (key == '2')  //  Back
    { 
    Ypos = Ypos+lineInc; 
    if (Ypos > Ymax) { Ypos = Ymax; }
    servoY.write(Ypos);
    }
  if (key == '8')  //  Forward
    { 
    Ypos = Ypos-lineInc; 
    if (Ypos < Ymin) { Ypos = Ymin; }
    servoY.write(Ypos);
    }
  if (key == '4')  //  Left
    { 
    Xpos = Xpos+lineInc; 
    if (Xpos > Xmax) { Xpos = Xmax; }
    servoX.write(Xpos);
    }
  if (key == '6')  //  Right
    { 
    Xpos = Xpos-lineInc; 
    if (Xpos < Xmin) { Xpos = Xmin; }
    servoX.write(Xpos);
    }
  if (key == '5')  //  Down
    { 
    Zpos = Zpos-lineInc*100; 
    if (Zpos < Zmin) { Zpos = Zmin; }
    servoZ.write(Zpos);
    }
  if (key == '0')  //  Up
    { 
    Zpos = Zpos+lineInc*100; 
    if (Zpos > Zmax) { Zpos = Zmax; }
    servoZ.write(Zpos);
    }
  //  Do stuff!
  //  Serial.println(key); 
  if (key == '1')  //  Back
    { 
    servoY.write(Ymax);
    servoX.write(Xmax);
    }
  //  Serial.println(key); 
  if (key == '3')  //  Back
    { 
    servoY.write(Ymax);
    servoX.write(Xmin);
    }
  //  Serial.println(key); 
  if (key == '7')  //  Back
    { 
    servoY.write(Ymin);
    servoX.write(Xmax);
    }
  //  Serial.println(key); 
  if (key == '9')  //  Back
    { 
    servoY.write(Ymin);
    servoX.write(Xmin);
    }


  //  Serial.println(Xpos); Serial.println(Ypos); Serial.println(Zpos); 
  }
}

//////////////////////////////////////////
//  Functions for Calculating Useful Stuff
//////////////////////////////////////////

//  Converts mm to degrees
float mm2deg(float mm) 
  { return mm/PI/gearD*360; }

//  Converts degrees to mm
float deg2mm(float deg) 
  { return PI*gearD*deg/360; }

//  Converts degrees to steps
float deg2steps(float deg)
  { return 256/180 * deg; }

//  Converts steps to deg
float steps2deg(float steps)
  { return 180/256 * steps; }

//  Converts steps to mm
float steps2mm(float steps)
  { return deg2mm(steps2deg(steps)); }

//  Converts mm to steps
float mm2steps(float mm) 
  { return deg2steps(mm2deg(mm)); }

