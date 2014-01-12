/* 
Tiny 3-Axis CNC Sketch to draw shapes using the MakerBot Unicorn Inkscape plugin available here https://github.com/martymcguire/inkscape-unicorn
*/

#include <Servo.h>

#define SERVOPINX 11
#define SERVOPINY 10
#define SERVOPINZ 9
#define LINE_BUFFER_LENGTH 512

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

//  Drawing settings
int StepIncPower = 3;
float StepInc = (1/ pow(2,StepIncPower));
int StepDelay = 0;
int LineDelay = 50;
int penDelay = 50;
//  Rounding the mm2deg functions corrects for floating point errors
float PowerRound = 2;

//  Drawing robot limits, in degrees
float Xdmin = 17;  //  8.31mm
float Xdmax = 171;  //  83.57mm
float Ydmin = 25;  //  Functionally 18 degrees  //  12.22mm
float Ydmax = 146;  //  Functionally 146        //  71.35mm
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
  Serial.print("Step increment is "); 
  Serial.print(StepInc); 
  Serial.println(" degrees");
  Serial.print("X range is from "); 
  Serial.print(Xmin); 
  Serial.print(" to "); 
  Serial.print(Xmax); 
  Serial.println(" mm."); 
  Serial.print("Y range is from "); 
  Serial.print(Ymin); 
  Serial.print(" to "); 
  Serial.print(Ymax); 
  Serial.println(" mm."); 
}

/**********************
 * void loop() - Main loop
 ***********************/
void loop() 
{
  delay(1000);
  char line[ LINE_BUFFER_LENGTH ];
  char c;
  int lineIndex;
  bool lineIsComment, lineSemiColon;

  lineIndex = 0;
  lineSemiColon = false;
  lineIsComment = false;

  while (1) {

    // Serial reception - Mostly from Grbl, added semicolon support
    while ( Serial.available()>0 ) {
      c = Serial.read();
      if (( c == '\n') || (c == '\r') ) {             // End of line reached
        if ( lineIndex > 0 ) {                        // Line is complete. Then execute!
          line[ lineIndex ] = '\0';                   // Terminate string
          if (verbose) { 
            Serial.print( "Received : "); 
            Serial.println( line ); 
          }
          processIncomingLine( line, lineIndex );
          lineIndex = 0;
        } 
        else { 
          // Empty or comment line. Skip block.
        }
        lineIsComment = false;
        lineSemiColon = false;  
      } 
      else {
        if ( (lineIsComment) || (lineSemiColon) ) {   // Throw away all comment characters
          if ( c == ')' )  lineIsComment = false;     // End of comment. Resume line.
        } 
        else {
          if ( c <= ' ' ) {                           // Throw away whitepace and control characters
          } 
          else if ( c == '/' ) {                    // Block delete not supported. Ignore character.
          } 
          else if ( c == '(' ) {                    // Enable comments flag and ignore all characters until ')' or EOL.
            lineIsComment = true;
          } 
          else if ( c == ';' ) {
            lineSemiColon = true;
          } 
          else if ( lineIndex >= LINE_BUFFER_LENGTH-1 ) {
            Serial.println( "ERROR - lineBuffer overflow" );
            lineIsComment = false;
            lineSemiColon = false;
          } 
          else if ( c >= 'a' && c <= 'z' ) {        // Upcase lowercase
            line[ lineIndex++ ] = c-'a'+'A';
          } 
          else {
            line[ lineIndex++ ] = c;
          }
        }
      }
    }
  }
}

void processIncomingLine( char* line, int charNB ) {
  int currentIndex = 0;
  char buffer[ 64 ];                                 // Hope that 64 is enough for 1 parameter
  struct point newPos;

  newPos.x = 0.0;
  newPos.y = 0.0;

  //  Needs to interpret 
  //  G1 for moving
  //  G4 P300 (wait 150ms)
  //  G1 X60 Y30
  //  G1 X30 Y50
  //  M300 S30 (pen down)
  //  M300 S50 (pen up)
  //  Discard anything with a (
  //  Discard any other command!

  while( currentIndex < charNB ) {
    switch ( line[ currentIndex++ ] ) {              // Select command, if any
    case 'U':
      penUp(); 
      break;
    case 'D':
      penDown(); 
      break;
    case 'G':
      buffer[0] = line[ currentIndex++ ];          // /!\ Dirty - Only works with 2 digit commands
//      buffer[1] = line[ currentIndex++ ];
//      buffer[2] = '\0';
      buffer[1] = '\0';

      switch ( atoi( buffer ) ){                   // Select G command
      case 0:                                   // G00 & G01 - Movement or fast movement. Same here
      case 1:
        // /!\ Dirty - Suppose that X is before Y
        char* indexX = strchr( line+currentIndex, 'X' );  // Get X/Y position in the string (if any)
        char* indexY = strchr( line+currentIndex, 'Y' );
        if ( indexY <= 0 ) {
          newPos.x = atof( indexX + 1); 
          newPos.y = actuatorPos.y;
        } 
        else if ( indexX <= 0 ) {
          newPos.y = atof( indexY + 1);
          newPos.x = actuatorPos.x;
        } 
        else {
          newPos.y = atof( indexY + 1);
          indexY = '\0';
          newPos.x = atof( indexX + 1);
        }
        Serial.println("OK");
        drawLine((int) newPos.x, (int) newPos.y );
        actuatorPos.x = newPos.x;
        actuatorPos.y = newPos.y;
        break;
      }
      break;
    case 'M':
      buffer[0] = line[ currentIndex++ ];        // /!\ Dirty - Only works with 3 digit commands
      buffer[1] = line[ currentIndex++ ];
      buffer[2] = line[ currentIndex++ ];
      buffer[3] = '\0';
      switch ( atoi( buffer ) ){
      case 300:
        {
        char* indexS = strchr( line+currentIndex, 'S' );
        float Spos = atof( indexS + 1);
        Serial.println("OK");
        if (Spos == 30) { penDown(); }
        if (Spos == 50) { penUp(); }
        break;
        }
      case 114:                                // M114 - Repport position
        Serial.print( "Absolute position : X = " );
        Serial.print( actuatorPos.x );
        Serial.print( "  -  Y = " );
        Serial.println( actuatorPos.y );
        break;
      default:
        Serial.print( "Command not recognized : M");
        Serial.println( buffer );
      }
    }
  }
}



//  Draw a circle spiral!
void drawCircleSpiral(float x0, float y0, float rad, float dec)
{
  int i = 0;
  while (rad-dec*i > StepInc) { 
    drawCircle(x0, y0, rad-dec*i); 
    i++; 
  }
}

//  Draw a circle!
void drawCircle(float x0, float y0, float rad)
{
  //  Lift the pen, go to leftmost point on circle, set the pen down
  penUp(); 
  drawLine(x0-rad, y0); 
  penDown();
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
  { 
    drawRect(x0+dec*i,y0+dec*i,x1-dec*i,y1-dec*i, toggle); 
  }
}

//  Draw a rectangle using just two points
void drawRect(float x0, float y0, float x1, float y1, boolean toggle)
{
  penUp();
  drawLine(x0,y0);
  if (toggle) { 
    penUp(); 
    delay(penDelay); 
    penDown(); 
  }
  else { 
    penDown(); 
  }
  drawLine(x1,y0);
  if (toggle) { 
    penUp(); 
    delay(penDelay); 
    penDown(); 
  }
  drawLine(x1,y1);
  if (toggle) { 
    penUp(); 
    delay(penDelay); 
    penDown(); 
  }
  drawLine(x0,y1);
  if (toggle) { 
    penUp(); 
    delay(penDelay); 
    penDown(); 
  }
  drawLine(x0,y0);
  if (toggle) { 
    penUp(); 
    delay(penDelay); 
    penDown(); 
  }
}

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
  if (x1 >= Xdmax) { 
    x1 = Xdmax; 
  }
  if (x1 <= Xdmin) { 
    x1 = Xdmin; 
  }
  if (y1 >= Ydmax) { 
    y1 = Ydmax; 
  }
  if (y1 <= Ydmin) { 
    y1 = Ydmin; 
  }

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
//  Raises pen
void penUp() { 
  servoZ.write(Zdmax); 
  delay(LineDelay); 
  Zpos=Zdmax; 
  if (verbose) { 
    Serial.println("Pen up!"); 
  } 
}
//  Lowers pen
void penDown() { 
  servoZ.write(Zdmin); 
  delay(LineDelay); 
  Zpos=Zdmin; 
  if (verbose) { 
    Serial.println("Pen down."); 
  } 
}

//////////////////////////////////////////
//  Functions for Calculating Useful Stuff
//////////////////////////////////////////
//  Converts mm to degrees for the servos
float mm2deg(float mm) { 
  return mm/PI/gearD*360; 
}
//  Converts mm to degrees for the servos
float deg2mm(float deg) { 
  return PI*gearD*deg/360; 
}
//  Function for getting Y to draw a circle
float returnY(float x, float x0, float rad)
{ 
  return ((int) ((pow( pow(rad, 2) - pow((x-x0), 2) , 0.5))*pow(10,PowerRound)))/pow(10,PowerRound); 
}
//  Function for determining distance between two points
float lineDist(float x0, float y0, float x1, float y1) 
{ 
  //  return ((int) (pow(pow(x1-x0, 2) + pow(y1-y0, 2), 0.5) * pow(10,PowerRound)))/pow(10,PowerRound); 
  float temp = pow(pow(x1-x0, 2) + pow(y1-y0, 2), 0.5); 
  //    Serial.println(temp);
  temp = (int) temp * pow(10,PowerRound);
  //    Serial.println(temp);
  temp = temp/pow(10,PowerRound);
  //    Serial.println(temp);
  return temp;
}