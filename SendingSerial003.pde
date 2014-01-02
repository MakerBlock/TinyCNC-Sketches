/**
 * Simple Write. 
 * Check if the mouse is over a rectangle and writes the status to the serial port. 
 * This example works with the Wiring / Arduino program that follows below.
 */

import processing.serial.*;
import java.io.*;
//  import java.io.FileReader;


Serial myPort;  // Create object from Serial class
int val;        // Data received from the serial port
String command;

void setup() 
  { 
  String portName = Serial.list()[0];
  myPort = new Serial(this, portName, 115200);
  
  readData("C:/Users/Jay/Desktop/MakerBot/processing-2.0.3/Sketchbook/Sketches/SendingSerial003/file.gcode");
  }

void readData(String myGcodeFile)
  {
  File file=new File(myGcodeFile);
 BufferedReader br=null;
 try{
 br=new BufferedReader(new FileReader(file));
 String text=null;
 
 /* keep reading each line until you get to the end of the file */
 while((text=br.readLine())!=null){
 /* Spilt each line up into bits and pieces using a comma as a separator */
 print(text + '\n');
 myPort.write(text);
 myPort.write('\n');
 delay(1000);
 }
 }catch(FileNotFoundException e){
 e.printStackTrace();
 }catch(IOException e){
 e.printStackTrace();
 }finally{
 try {
 if (br != null){
 br.close();
 }
 } catch (IOException e) {
 e.printStackTrace();
 }
 }
}

void importTextFile(){   
  String[] strLines = loadStrings("file.gcode"); // the name and extension of the file to import!
  for(int i = 0; i < strLines.length; ++i){
    String[] arrTokens = split(strLines[i], ',');       // use the split array with character to isolate each component
    float xx = float(arrTokens[0]);                     // cast string value to a float values!
    float yy = float(arrTokens[1]);                     // cast string value to a float values!
    float zz = float(arrTokens[2]);                     // cast string value to a float values!
   // pointList.add( new PVector(xx,yy,zz) );             // add values to a new array slot
  }
}
