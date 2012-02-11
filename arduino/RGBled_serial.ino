/*
 * Hello World!
 *
 * This is the Hello World! for Arduino. 
 * It shows how to send data to the computer
 */

// Output
int redPin = 10;   // Red LED,   connected to digital pin 9
int grnPin = 11;  // Green LED, connected to digital pin 10
int bluPin = 9;  // Blue LED,  connected to digital pin 11

int ledPin = 8;

// data
int inByte=0;

// etc.
int wait = 0;
int hold = 0;       // Optional hold when a color is complete, before the next crossFade
int DEBUG = 1;      // DEBUG counter; if set to 1, will write values back via serial

int loopCount = 60; // How often should DEBUG report?
// Set initial color
byte redVal = 0;
byte grnVal = 0; 
byte bluVal = 0;

// Initialize color variables
byte prevR = redVal;
byte prevG = grnVal;
byte prevB = bluVal;

//

boolean blink=false;

void setup()                    // run once, when the sketch starts
{
  Serial.begin(9600);           // set up Serial library at 9600 bps
}

void loop()                       // run over and over again
{
  
  if (Serial.available() > 0) {
    
     // get incoming byte:
    inByte = Serial.read();
    
    if(inByte=='.') {
      Serial.println("READY");  // prints hello with ending line break 
    } else if(inByte=='#') {
      blink=false;
      readColor();
      Serial.println("DONE");
    } else if(inByte=='*') {
      blink=true;
      Serial.flush();
      Serial.println("DONE");
    }
   }
   
    if(blink) {
      setBlink();
    }
}

void setBlink() {
  Serial.print(".*.");
  digitalWrite(redPin, LOW);
  digitalWrite(grnPin, LOW);
  digitalWrite(bluPin, LOW);
  
  analogWrite(ledPin, HIGH);
  delay(100);
  
  digitalWrite(redPin, prevR);   // Write current values to LED pins
  digitalWrite(grnPin, prevG);      
  digitalWrite(bluPin, prevB);
  
  analogWrite(ledPin, LOW);
  delay(500);
}


void readColor() { 
 
 byte color[3]={0,0,0};
 
 for(int i=0; i<3; i++ ){
   Serial.println(".");
   Serial.flush();
   color[i]=Serial.read();
 }
 Serial.flush();
 
 Serial.print("OK - #");
 Serial.print(color[0],HEX);
 Serial.print(" / ");
 Serial.print(color[1],HEX);
 Serial.print(" / ");  
 Serial.println(color[2],HEX);
 
 
 crossFade(color);
}

/* crossFade() converts the percentage colors to a 
*  0-255 range, then loops 1020 times, checking to see if  
*  the value needs to be updated each time, then writing
*  the color values to the correct pins.
*/

void crossFade(byte color[3]) {
  Serial.print("******* crossFade: #");
  Serial.print(color[0],HEX);
  Serial.print(" / ");
  Serial.print(color[1],HEX);
  Serial.print(" / ");  
  Serial.println(color[2],HEX); 
  
  byte stepR = calculateStep(prevR, color[0]);
  byte stepG = calculateStep(prevG, color[1]); 
  byte stepB = calculateStep(prevB, color[2]);

  for (int i = 0; i <= 1020; i++) {
    redVal = calculateVal(stepR, redVal, i);
    grnVal = calculateVal(stepG, grnVal, i);
    bluVal = calculateVal(stepB, bluVal, i);

    analogWrite(redPin, redVal);   // Write current values to LED pins
    analogWrite(grnPin, grnVal);      
    analogWrite(bluPin, bluVal); 
    
    delay(wait); // Pause for 'wait' milliseconds before resuming the loop

    if (DEBUG) { // If we want serial output, print it at the 
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

byte calculateStep(byte prevValue, byte endValue) {
  byte step = endValue - prevValue; // What's the overall gap?
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

byte calculateVal(byte step, byte val, byte i) {
  if ((step) && i % step == 0) { // If step is non-zero and its time to change a value,
    if (step > 0) {              //   increment the value if step is positive...
      val += 1;           
    } 
    else if (step < 0) {         //   ...or decrement it if step is negative
      val -= 1;
    } 
  }
  // Defensive driving: make sure val stays in the range 0-255
  if (val > 254) {
    val = 254;
  } 
  else if (val < 0) {
    val = 0;
  }
  return val;
}
