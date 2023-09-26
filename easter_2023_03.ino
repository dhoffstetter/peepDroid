
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>

// initialize the library with the numbers of the interface pins
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// you can change the overall brightness by range 0 -> 255
int brightness = 100;

const int nPins = 5;
const int nSolutions = 3;
const int helpCountMax = 10;
int helpCount = 0;
const int outputPins[] = {2, 3, 4, 5, 6};
const int inputPins[] = {7, 8, 9, 10, 11};
int outputConInput[] = {-1, -1, -1, -1, -1};
int const outputConSolution[nSolutions][nPins] = {
    { 3,  4,  0,  2,  1},
    { 2,  4,  3,  0,  1},
    { 0,  1,  2,  3,  4} }
    ;
bool solutionSolved[nSolutions] = {false, false, false};

const int audioIntro = 12;
const int audioOutFail = A0;
const int audioOutSuccess[] = {A1, A2, A3};

bool debugSolCount = false;
bool debugShowConnection = false;
bool debugPrintLcd = false;


void pollConnections(){
  for (int outPinIndex = 0; outPinIndex < nPins; outPinIndex++){
    outputConInput[outPinIndex] = -1;
    
    for (int inPinIndex = 0; inPinIndex < nPins; inPinIndex++){
      digitalWrite(outputPins[outPinIndex], HIGH);
      if (digitalRead(inputPins[inPinIndex]) == HIGH) {
        digitalWrite(outputPins[outPinIndex], LOW);
        if (digitalRead(inputPins[inPinIndex]) == LOW){
          outputConInput[outPinIndex] = inPinIndex;
          break;
        }
      }
    }
    digitalWrite(outputPins[outPinIndex], HIGH);
  }
}

int connectionStatus(){
  // >= 0 = index of correct connection
  // Message will only play if all previous connections have been found
  // A correct connection out of order will play the error sound
  // -1 = wrong
  // -2 = incomplete
  
  int numberConnected = 0;
  int numberCorrect[nSolutions] = {0, 0, 0}; // Track the number of pins that are correct for each solution

  for (int pinIndex = 0; pinIndex < nPins; pinIndex++){
    if (outputConInput[pinIndex] != -1){
      numberConnected++;
      for (int solIndex = 0; solIndex < nSolutions; solIndex++){
        if (outputConInput[pinIndex] == outputConSolution[solIndex][pinIndex]){
          numberCorrect[solIndex]++;
        }
      }
    }
  }
  clearLcd();
  for (int solIndex = 0; solIndex < nSolutions; solIndex++){
    if (debugSolCount){
      lcd.setCursor(solIndex*3,0);
      lcd.print(numberCorrect[solIndex]);
      delay(1000);
    }
    if (numberCorrect[solIndex] == nPins){
      return solIndex;
    }
  }
  if (numberConnected == nPins){ // all connected but wrong
    return -1;
  }
  else { // at least one disconnected
    return -2;
  }
}

void clearLcd(){
    lcd.setCursor(0,0);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
}

void showConnectionsDebug(int del){
  clearLcd();
  lcd.setCursor(0,0);
  lcd.print("                ");
  for (int index = 0; index < nPins; index++){
    lcd.setCursor(2 * index, 0);
    if (outputConInput[index] == -1){
      lcd.print(".");
    }
    else {
      lcd.print(outputConInput[index]);
    }
  }
  delay(del);
}

void setup()  
{
    // set up the LCD's number of rows and columns: 
  lcd.begin(16, 2);
  lcd.setBacklight(0x7);
  // Print a message to the LCD.
  brightness = 100;
  if (debugPrintLcd){
    clearLcd();
    lcd.setCursor(0,0);
    lcd.print("Help me ObiWan! ");
    lcd.setCursor(0,1);
    lcd.print("CONNECT WIRES");}

  // Initialize audio output pins
  pinMode(audioOutFail, OUTPUT);
  digitalWrite(audioOutFail, HIGH);
  pinMode(audioIntro, OUTPUT);
  digitalWrite(audioIntro, HIGH);

  for (int index = 0; index < 3; index++){
    pinMode(audioOutSuccess[index], OUTPUT);
    digitalWrite(audioOutSuccess[index], HIGH);
  }

  // Initialize IO pins
  for (int index = 0; index < nPins; index++){
    pinMode(outputPins[index], OUTPUT);
    digitalWrite(outputPins[index], HIGH);
    pinMode(inputPins[index], INPUT_PULLUP);
  }

  // Play the intro message
  digitalWrite(audioIntro, LOW);
  delay(100);
  digitalWrite(audioIntro, HIGH);
  delay(2000);
}


void loop()                     // run over and over again
{
  pollConnections();
  int status = connectionStatus();
  clearLcd();

  if (status >= 0){ // One of the solutions is correct
    if (debugPrintLcd){
      lcd.setCursor(0,0);
      lcd.print("CORRECT         ");
      lcd.setCursor(0,1);
      lcd.print(status);
    }      
    for (int solIndex = 0; solIndex < nSolutions; solIndex++){
      if (solIndex != status){
        digitalWrite(audioOutSuccess[solIndex], HIGH);
      }
    }
    digitalWrite(audioOutFail, HIGH);
    delay(100);
    if (status == 0 || solutionSolved[status - 1] == true){
      solutionSolved[status] = true;
      digitalWrite(audioOutSuccess[status], LOW);      
    }
    else{
      digitalWrite(audioOutFail, LOW);
      helpCount++;
    }
  }
  else if (status == -1) {
    if (debugPrintLcd){
      lcd.setCursor(0,0);
      lcd.print("WRONG           ");
    }
    for (int solIndex = 0; solIndex < nSolutions; solIndex++){
      digitalWrite(audioOutSuccess[solIndex], HIGH);
    }
    helpCount++;
    // lcd.setCursor(0,0);
    // lcd.print(helpCount);
    if (helpCount >= helpCountMax){
      helpCount = 0;
      digitalWrite(audioOutFail, HIGH);
      delay(2000);
      digitalWrite(audioIntro, LOW);
      delay(100);
      digitalWrite(audioIntro, HIGH);
      delay(100);
    }
    delay(100);
    digitalWrite(audioOutFail, LOW);
  }
  else if (status == -2) {
    if (debugPrintLcd){
      lcd.setCursor(0,0);
      lcd.print("INCOMPLETE      ");      
    }

    for (int solIndex = 0; solIndex < nSolutions; solIndex++){
      digitalWrite(audioOutSuccess[solIndex], HIGH);
    }
    digitalWrite(audioOutFail, HIGH);
  }
  delay(100);
  if (debugShowConnection){
    showConnectionsDebug(500);
  }
        
}
