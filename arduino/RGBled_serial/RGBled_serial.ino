/*
 * RGBled_serial
 *
 * Permits change the color of the RGB led with commands sent by the usb
 * 
 * The "protocol" to change the color is:
 * 
 * usb     arduino
 * ---     -------
 * .    -> READY
 * #RGB -> DONE     
 *
 * To blink the led
 *
 * usb     arduino
 * ---     -------
 * .    -> READY
 * *    -> DONE 
 * 
 * 
 * Based on arduino color cross-fading sample
 * http://www.arduino.cc/en/Tutorial/ColorCrossfader
 */

// Output
int redPin = 10;   // Red LED
int grnPin = 11;  // Green LED
int bluPin = 9;  // Blue LED

int ledPin = 8;  // Heart LED (red led)


// etc.
int wait = 1;
int hold = 0;       // Optional hold when a color is complete, before the next crossFade
int DEBUG = 0;      // DEBUG counter; if set to 1, will write values back via serial

int loopCount = 60; // How often should DEBUG report?

boolean waitCmd = false; // we are wating for a command?

const boolean OFF = HIGH;

// Set initial color
int redVal = 0;
int grnVal = 0; 
int bluVal = 0;

// Initialize color variables
int prevR = redVal;
int prevG = grnVal;
int prevB = bluVal;

String lastCmd = "-1";

boolean blink=false;

void setup()                    // run once, when the sketch starts
{
  Serial.begin(9600);           // set up Serial library at 9600 bps
}

void loop()                       // run over and over again
{
  if (Serial.available() > 0) {
    Serial.println("READY");
    String c = getCommand();
    
    Serial.println("Last: " + lastCmd + " cmd: "+c);
    
    if(lastCmd!=c) {
      doCommand(c); 
      
    } else {
      Serial.println("Command "+ c +" skipped");
    }
    
    
    
  }
   
    if(blink) {
      setBlink();
    }
    
    delay(1000);
}

void doCommand(String cmd) {
  if(cmd.length()==0 || cmd=="") {
    return;
  }
  
  char cmdType = cmd.charAt(0);   
     
      if(cmdType=='#' && cmd.length()==4) {
        if(DEBUG) { Serial.println("Color command received "+cmd); }
        blink=false;
        crossFade(cmd[1], cmd[2], cmd[3]);
      } else if(cmd=="blink") {
        if(DEBUG) { Serial.println("blink command received "); }
        blink=true;
      } else if(cmd=="off") {
	if(DEBUG) { Serial.println("off command received "); }

	digitalWrite(redPin, OFF);
	digitalWrite(grnPin, OFF);
	digitalWrite(bluPin, OFF);

      } else {
        Serial.println("Unknown command: "+cmd);
      }
  lastCmd=cmd;
  Serial.println("DONE");
}

String getCommand() {
  String cmd = "";
  
  char inByte;
  
  for(int x=0; x<15; x++) {
    inByte=Serial.read();
    Serial.flush();
    if(inByte == ';') {
      return cmd;
    } else {
      cmd.concat(char(inByte)) ;
    } 
  }
}


void setBlink() {
  Serial.print("blink OK");
  
  digitalWrite(redPin, LOW);
  digitalWrite(grnPin, LOW);
  digitalWrite(bluPin, LOW);
  
  analogWrite(ledPin, HIGH);
  delay(100);
  
  digitalWrite(redPin, prevR);   // Write current values to LED pins
  digitalWrite(grnPin, prevG);      
  digitalWrite(bluPin, prevB);
  
  analogWrite(ledPin, LOW);
  delay(100);
}


/* crossFade() 
*  loops 1020 times, checking to see if  
*  the value needs to be updated each time, then writing
*  the color values to the correct pins.
*/

void crossFade(byte colorR, byte colorG, byte colorB) {
  int stepR = calculateStep(prevR, colorR);
  int stepG = calculateStep(prevG, colorG); 
  int stepB = calculateStep(prevB, colorB);
  
  if(stepR==0 && stepG==0 && stepB==0) {
    return;
  }
  
  if (DEBUG>1) {
    Serial.print("******* crossFade FROM: #");
    Serial.print(prevR,HEX);
    Serial.print(" / ");
    Serial.print(prevG,HEX);
    Serial.print(" / ");  
    Serial.println(prevB,HEX); 
    Serial.print("******* crossFade TO: #");
    Serial.print(colorR,HEX);
    Serial.print(" / ");
    Serial.print(colorG,HEX);
    Serial.print(" / ");  
    Serial.println(colorB,HEX);
    Serial.print("******* Step: #");
    Serial.print(stepR);
    Serial.print(" / ");
    Serial.print(stepG);
    Serial.print(" / ");  
    Serial.println(stepB);  
  }

  for (int i = 0; i <= 1020; i++) {
    redVal = calculateVal(stepR, redVal, i);
    grnVal = calculateVal(stepG, grnVal, i);
    bluVal = calculateVal(stepB, bluVal, i);

    analogWrite(redPin, redVal);   // Write current values to LED pins
    analogWrite(grnPin, grnVal);      
    analogWrite(bluPin, bluVal); 
    
    delay(wait); // Pause for 'wait' milliseconds before resuming the loop

    if (DEBUG > 10) { // If we want serial output, print it at the 
      if (i == 0 or i % loopCount == 0) { // beginning, and every loopCount times
        Serial.print("Loop/RGB: #");
        Serial.print(i);
        Serial.print(" | ");
        Serial.print(redVal,HEX);
        Serial.print(" / ");
        Serial.print(grnVal,HEX);
        Serial.print(" / ");  
        Serial.println(bluVal,HEX); 
      } 
      DEBUG += 1;
    }
  }
  // Update current values for next loop
  prevR = redVal; 
  prevG = grnVal; 
  prevB = bluVal;
}

void establishContact() {
  while (Serial.available() <= 0) {
    Serial.print('.');   // send a capital A
    delay(300);
  }
}



/* BELOW THIS LINE IS THE MATH -- YOU SHOULDN'T NEED TO CHANGE THIS FOR THE BASICS
* 
* The program rks like this:
* Imagine a crossfade that moves the red LED from 0-10, 
*   the green from 0-5, and the blue from 10 to 7, in
*   ten steps.
*   We'd want to count the 10 steps and increase or 
*   decrease color values in evenly stepped increments.
*   Imagine a + indicates raising a value by 1, and a -
*   equals lowering it. Our 10 step fade would look like:
* 
*   1 2 3 4 5 6 7 8 9 10
* R + + + + + + + + + +
* G   +   +   +   +   +
* B     -     -     -
* 
* The red rises from 0 to 10 in ten steps, the green from 
* 0-5 in 5 steps, and the blue falls from 10 to 7 in three steps.
* 
* In the real program, the color percentages are converted to 
* 0-255 values, and there are 1020 steps (255*4).
* 
* To figure out how big a step there should be between one up- or
* down-tick of one of the LED values, we call calculateStep(), 
* which calculates the absolute gap between the start and end values, 
* and then divides that gap by 1020 to determine the size of the step  
* between adjustments in the value.
*/

int calculateStep(int prevValue, int endValue) {
  int step = endValue - prevValue; // What's the overall gap?
  if (step) {                      // If its non-zero, 
    step = 1020/step;              //   divide by 1020
  } 
  return step;
}

/* The next function is calculateVal. When the loop value, i,
*  reaches the step size appropriate for one of the
*  colors, it increases or decreases the value of that color by 1. 
*  (R, G, and B are each calculated separately.)
*/

int calculateVal(int step, int val, int i) {
  if ((step) && i % step == 0) { // If step is non-zero and its time to change a value,
    if (step > 0) {              //   increment the value if step is positive...
      val += 1;           
    } else if (step < 0) {         //   ...or decrement it if step is negative
      val -= 1;
    } 
  }
  // Defensive driving: make sure val stays in the range 0-255
  if (val > 254) {
    val = 254;
  } else if (val < 0) {
    val = 0;
  }
  return val;
}
