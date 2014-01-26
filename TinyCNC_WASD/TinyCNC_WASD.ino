//  Based on work by Oliv4945, https://github.com/Oliv4945/GcodeMiniCNC
#include <Servo.h>

#define PIN_SERVO_X 11
#define PIN_SERVO_Y 10
#define PIN_SERVO_Z 9
#define LINE_BUFFER_LENGTH 512

  float movementIncrement = 2;
  int movementDelay = 50*movementIncrement;

/* Structures, global variables    */
struct point {
  float x;
  float y;
  float z;
};

Servo servoX;
Servo servoY;
Servo servoZ;
struct point actuatorPos;

/**********************
* void setup() - Initialisations
***********************/
void setup() {
  Serial.begin( 115200 );
  servoX.attach( PIN_SERVO_X );
  servoY.attach( PIN_SERVO_Y );
  servoZ.attach( PIN_SERVO_Z );

// 
  actuatorPos.x = 80;
  actuatorPos.y = 20;
  actuatorPos.z = 140;

  // Set default positions
  servoX.write( actuatorPos.x );
  servoY.write( actuatorPos.y );
  servoZ.write( actuatorPos.z );
  
  Serial.println( "MiniCNC - GO!!!" );
}

/**********************
* void loop() - Main loop
***********************/
void loop() {
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
          Serial.print( "Received : "); Serial.println( line );
          processIncomingLine( line, lineIndex );
          lineIndex = 0;
        } else { 
          // Empty or comment line. Skip block.
        }
        lineIsComment = false;
        lineSemiColon = false;  
      } else {
        if ( (lineIsComment) || (lineSemiColon) ) {   // Throw away all comment characters
          if ( c == ')' )  lineIsComment = false;     // End of comment. Resume line.
        } else {
          if ( c <= ' ' ) {                           // Throw away whitepace and control characters
          } else if ( c == '/' ) {                    // Block delete not supported. Ignore character.
          } else if ( c == '(' ) {                    // Enable comments flag and ignore all characters until ')' or EOL.
            lineIsComment = true;
          } else if ( c == ';' ) {
            lineSemiColon = true;
          } else if ( lineIndex >= LINE_BUFFER_LENGTH-1 ) {
            Serial.println( "ERROR - lineBuffer overflow" );
            lineIsComment = false;
            lineSemiColon = false;
          } else if ( c >= 'a' && c <= 'z' ) {        // Upcase lowercase
            line[ lineIndex++ ] = c-'a'+'A';
          } else {
            line[ lineIndex++ ] = c;
          }
        }
      }
    }
  }
}



/**********************
* void processIncomingLline( char* line, int charNB ) - Main loop
* char* line : Line to process
* int charNB : Number of characters
***********************/
void processIncomingLine( char* line, int charNB ) {
  int currentIndex = 0;
  char buffer[ 64 ];                                 // Hope that 64 is enough for 1 parameter
  
  while( currentIndex < charNB ) {
    delay(movementDelay);
    switch ( line[ currentIndex++ ] ) {
      // Select command, if any
      case 'O': actuatorPos.z=actuatorPos.z+movementIncrement; servoZ.write( actuatorPos.z ); 
      Serial.println('0' + actuatorPos.z); break;
      case 'L': actuatorPos.z=actuatorPos.z-movementIncrement; servoZ.write( actuatorPos.z ); 
      Serial.println('L' + actuatorPos.z); break;
      case 'D': actuatorPos.x=actuatorPos.x+movementIncrement; servoX.write( actuatorPos.x ); 
      Serial.println('D' + actuatorPos.x); break;
      case 'A': actuatorPos.x=actuatorPos.x-movementIncrement; servoX.write( actuatorPos.x ); 
      Serial.println('A' + actuatorPos.x); break;
      case 'W': actuatorPos.y=actuatorPos.y+movementIncrement; servoY.write( actuatorPos.y ); 
      Serial.println('W' + actuatorPos.y); break;
      case 'S': actuatorPos.y=actuatorPos.y-movementIncrement; servoY.write( actuatorPos.y ); 
      Serial.println('S' + actuatorPos.y); break;
    }
  }
servoZ.write( actuatorPos.z );
servoY.write( actuatorPos.y );
servoX.write( actuatorPos.x );
}
