/* 
Tiny 3-Axis CNC Sketch to draw rectangles, concentric rectangles, circles, concentric circles
*/

#include <Servo.h>

#define SERVOPINX 11
#define SERVOPINY 10
#define SERVOPINZ 9
#define LINE_BUFFER_LENGTH 512

/* Structures, global variables    */
struct point { float x; float y; float z; };

/* Naming servos    */
Servo servoX;
Servo servoY;
Servo servoZ;
struct point actuatorPos;

//  Gear settings
float gearD = 56;
float ZgearD = 13.3239;

//  Drawing settings
int StepIncPower = 3;
float StepInc = (1/ pow(2,StepIncPower));
int StepDelay = 0;
int LineDelay = 500;
int penDelay = 500;
//  Rounding the mm2deg functions corrects for floating point errors
float PowerRound = 2;

//  Drawing robot limits, in degrees
float Xdmin = 17;
float Xdmax = 171;
float Ydmin = 25;  //  Functionally 18
float Ydmax = 146;  //  Functionally 146
float Zdmin = 18;
float Zdmax = 50;

//  Drawing robot limits, in mm
float Xmin = deg2mm(Xdmin);
float Xmax = deg2mm(Xdmax);
float Ymin = deg2mm(Ydmin);
float Ymax = deg2mm(Ydmax);
float Zmin = Zdmin;
float Zmax = Zdmax;

float Xpos = Xdmin;
float Ypos = Ydmin;
float Zpos = Zmax; 

boolean verbose = false;

//  Needs to interpret 
//  G1 for moving
//  G4 P300 (wait 150ms)
//  M300 S30 (pen down)
//  M300 S50 (pen up)
//  Discard anything with a (
//  Discard any other command!

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
  Serial.print("Step increment is "); Serial.print(StepInc); Serial.println(" degrees");
  Serial.print("X range is from "); Serial.print(Xmin); Serial.print(" to "); Serial.print(Xmax); Serial.println(" mm."); 
  Serial.print("Y range is from "); Serial.print(Ymin); Serial.print(" to "); Serial.print(Ymax); Serial.println(" mm."); 
  }

/**********************
* void loop() - Main loop
***********************/
void loop() 
  {
  delay(5000);
//  drawCircle(50, 30, 10);
//  drawCircleSpiral(40, 30, 10, 1);
//  drawRect(15,15,80,35);
  drawRectSpiral(15,15,80,35,0.3, true);
  while(1) {}
  }

//  Draw a circle spiral!
void drawCircleSpiral(float x0, float y0, float rad, float dec)
  {
  int i = 0;
  while (rad-dec*i > StepInc) { drawCircle(x0, y0, rad-dec*i); i++; }
  }

//  Draw a circle!
void drawCircle(float x0, float y0, float rad)
  {
  //  Lift the pen, go to leftmost point on circle, set the pen down
  penUp(); drawLine(x0-rad, y0); penDown();
  float x1, y1, dist;
  //  This will draw the top half of the circle!
  for (int i=0; i*StepInc < rad*2; i++ )
    { 
    x1 = x0+StepInc*i-rad;
    y1 = y0 + returnY(x0+StepInc*i-rad, x0, rad);
    dist = lineDist(deg2mm(Xpos), deg2mm(Ypos), x1, y1);
    while (dist < StepInc)
      {
      i++;
      x1 = x0+StepInc*i-rad;
      y1 = y0 + returnY(x0+StepInc*i-rad, x0, rad);
      dist = lineDist(deg2mm(Xpos), deg2mm(Ypos), x1, y1);
      }
    drawLine(x1, y1); 
    }
  //  This should draw the bottom half of the circle!
  for (int i=0; i*StepInc < rad*2; i++ )
    { 
    x1 = x0+rad - StepInc*i;
    y1 = y0 -returnY(x0-StepInc*i+rad, x0, rad);
    dist = lineDist(deg2mm(Xpos), deg2mm(Ypos), x1, y1);
    while (dist < StepInc)
      {
      i++;
      x1 = x0-StepInc*i+rad;
      y1 = y0 - returnY(x0-StepInc*i+rad, x0, rad);
      dist = lineDist(deg2mm(Xpos), deg2mm(Ypos), x1, y1);
      }
    drawLine(x1, y1); 
    }
  //  Complete the circle, pen up!
  drawLine(x0-rad, y0); 
  }

//  Draw a rectangular spiral using just two points
void drawRectSpiral(float x0, float y0, float x1, float y1, float dec, boolean toggle)
  {
  drawLine(x0,y0);
  for (int i=0; ((x1-x0)/2 > dec*i) && ((y1-y0)/2 > dec*i);i++)
    { drawRect(x0+dec*i,y0+dec*i,x1-dec*i,y1-dec*i, toggle); }
  }

//  Draw a rectangle using just two points
void drawRect(float x0, float y0, float x1, float y1, boolean toggle)
  {
  penUp();
  drawLine(x0,y0);
  if (toggle) { penUp(); delay(penDelay); penDown(); }
  else { penDown(); }
  drawLine(x1,y0);
  if (toggle) { penUp(); delay(penDelay); penDown(); }
  drawLine(x1,y1);
  if (toggle) { penUp(); delay(penDelay); penDown(); }
  drawLine(x0,y1);
  if (toggle) { penUp(); delay(penDelay); penDown(); }
  drawLine(x0,y0);
  if (toggle) { penUp(); delay(penDelay); penDown(); }
  }

//  Raises pen
void penUp() { servoZ.write(Zdmax); delay(LineDelay); Zpos=Zdmax; if (verbose) { Serial.println("Pen up!"); } }
//  Lowers pen
void penDown() { servoZ.write(Zdmin); delay(LineDelay); Zpos=Zdmin; if (verbose) { Serial.println("Pen down."); } }

/*********************************
* Draw a line from (x0;y0) to (x1;y1). Bresenham algorythm from http://rosettacode.org/wiki/Bitmap/Bresenham's_line_algorithm
* int (x1;y1) : Starting coordinates
* int (x2;y2) : Ending coordinates
* Modified only require destination, new step min distance, max/min travel
**********************************/
void drawLine(float x1, float y1) {

  //  Convert coordinatesz to degrees
  x1 = ((int) mm2deg(x1)*pow(10,PowerRound))/pow(10,PowerRound);
  y1 = ((int) mm2deg(y1)*pow(10,PowerRound))/pow(10,PowerRound);
  float x0 = Xpos;
  float y0 = Ypos;

  //  Bring instructions within limits
  if (x1 >= Xdmax) { x1 = Xdmax; }
  if (x1 <= Xdmin) { x1 = Xdmin; }
  if (y1 >= Ydmax) { y1 = Ydmax; }
  if (y1 <= Ydmin) { y1 = Ydmin; }

  //  Let's find out the change for the coordinates
  float dx = abs(x1-x0), sx = x0<x1 ? StepInc : -StepInc;
  float dy = abs(y1-y0), sy = y0<y1 ? StepInc : -StepInc;
  float err = (dx>dy ? dx : -dy)/2, e2;

  //  Loops servo instructions until destination reached
  for(;;){    
    delay(1);  //  Without this the program sometimes hangs!
    servoX.write( x0 );
    servoY.write( y0 );
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dy) {
      err += dx;
      y0 += sy;
    }
    delay(StepDelay);           //delay for settling
  }
  if (verbose)
    {
    Serial.print("Going to (");
    Serial.print(deg2mm(x0));
    Serial.print(",");
    Serial.print(deg2mm(y0));
    Serial.println(")");
    }

  //  Delay before any next lines are submitted
  delay(LineDelay);
  //  Update the positions
  Xpos = x1;
  Ypos = y1;
}

//////////////////////////////////////////
//  Functions for Calculating Useful Stuff
//////////////////////////////////////////
//  Converts mm to degrees for the servos
float mm2deg(float mm) { return mm/PI/gearD*360; }
//  Converts mm to degrees for the servos
float deg2mm(float deg) { return PI*gearD*deg/360; }
//  Function for getting Y to draw a circle
float returnY(float x, float x0, float rad)
  { return ((int) ((pow( pow(rad, 2) - pow((x-x0), 2) , 0.5))*pow(10,PowerRound)))/pow(10,PowerRound); }
//  Function for determining distance between two points
float lineDist(float x0, float y0, float x1, float y1) 
  { 
  float temp = pow(pow(x1-x0, 2) + pow(y1-y0, 2), 0.5); 
//    Serial.println(temp);
  temp = (int) temp * pow(10,PowerRound);
//    Serial.println(temp);
  temp = temp/pow(10,PowerRound);
//    Serial.println(temp);
  return temp;
  }