/**
Processing code to send Gcode from Inkscape MakerBot Unicorn Plugin to Tiny CNC Drawing Robot
**/

import processing.serial.*;
import java.io.*;

Serial myPort;  // Create object from Serial class
int val;        // Data received from the serial port
String command;

void setup() 
  { 
  String portName = Serial.list()[0];
  myPort = new Serial(this, portName, 115200);
  
  readData("FILELOCATION/FILENAME.GCODE");
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