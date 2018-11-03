#include <Servo.h> // Required library for the servo

int NORMAL = 0;
int SLAVE = 1;


// IMPORTANT PARAMETER: This states what mode the arduino is in
int MODE = NORMAL;
boolean MASTER = true;

boolean playerWon = false;
boolean debugPrint = false;

Servo myServo;
// Sets up the number of players
int const amountOfPlayers = 4; // *** Is this meant to also be 2 if we're getting a 4 player game working ***?
int const amountOfLocalPlayers = 2;
int score[amountOfPlayers];
int difficulty = 1;

// assign LEDs, buttons and the servo to their pins
int ledPin[amountOfPlayers][3] = {{4,5,6},{11,12,13}};
int playerButton[amountOfPlayers] = {2, 3};

int whiteLED = 8;
int buzzer = 9;
int servoPin = 10;
int servoWin = 10;
int servoLose = 180;
int difficultyDial = A0;


boolean gameOn[amountOfPlayers];
boolean playerPressed[amountOfPlayers];




int randNumber;
boolean fouled[amountOfPlayers];

//setup interrupt, button input and LED outputs
void setup() {
  Serial.begin(9600); // Connects to the serial monitor for printing
  myServo.attach(servoPin);
  for (int r = 0; r < amountOfPlayers; r++) {
     attachInterrupt(digitalPinToInterrupt(playerButton[r]), triggered,RISING);
  }
  setVariables();
  
  pinMode(whiteLED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  for(int i = 0; i< amountOfPlayers; i++){
    pinMode(playerButton[i], INPUT);
    for (int j = 0; j<3; j++){
      pinMode(ledPin[i][j], OUTPUT);
    }
  }

  if(MASTER){
  waitForSlave();
  sendCmd('#', 0); // Initiates Remote LED test
  ledTest(); //Local Led Test
  }
  
}

void setVariables(){
  playerWon = false;

  for (int a = 0; a < amountOfPlayers; a ++) {
    // Loop to set up the arrays in a better format, if more players are added
    gameOn[a] = false;
    playerPressed[a] = false;
    fouled[a] = false;
    score[a] = 0;
  }
}

int readDifficultyDial(){
  int sensorValue = analogRead(difficultyDial);
  return map(sensorValue, 0, 1023, 0, 10);
}

int updateDifficulty(){
    difficulty = readDifficultyDial();
    
    if(MASTER) updateClientDifficulty(difficulty);
}

//run main program loop
void loop() {
  // Runs the play game function for each of the players

  if(!playerWon){

    switch(MODE){

      case 0:
      updateDifficulty();
      int e;
      for(e = 0; e < amountOfLocalPlayers; e++) {
          if(playGame(e)) flashPlayer(score[e], true);
      }
      if(MASTER) {
        while (e < amountOfPlayers) {
          if(executeRemotePlayer(e++)) flashPlayer(score[e],false);
        }
      }

      int winner;

      if((winner=getWinningPlayer())>=0){
        playWinningAnimation(winner);
        playerWon = true;
      }

      break;

      case 1 :
      if(Serial.available()>0) parseIncomingSerial();
      break;
    }
    
  }

  else{
    setVariables(); // Resets variables for new game
  }
//Debugging purposes
//debug();
}
int getWinningPlayer(){
  for (int a = 0; a < amountOfPlayers; a ++) {
    if(score[a]>=10) return a;
  }
  return -1;
}

void setPlayerLEDs(int playerID, int value){
  for (int j = 0; j<3; j++){
      digitalWrite(ledPin[playerID][j],value);
    }
}

void setAllLEDs(int value){
  for(int i = 0; i< amountOfPlayers; i++){
    for (int j = 0; j<3; j++){
      digitalWrite(ledPin[i][j],value);
    }
  }
}

void playWinningAnimation(int playerID){
  boolean remotePlayer = (MASTER && (playerID+1)>amountOfLocalPlayers);
  if(remotePlayer) sendCmd('&', playerID-amountOfLocalPlayers);
  else sendCmd('&', -1);
  for(int i =0; i<10; i++){
    setAllLEDs(HIGH);
    delay(100);
    setAllLEDs(LOW);
    delay(100);
  }

    int amountOfFlashes = 3;
    if(!remotePlayer){
    for(int i= 0; i< amountOfFlashes; i++){
    setPlayerLEDs(playerID,HIGH);
    delay(1000);
    setPlayerLEDs(playerID,LOW);
    delay(1000);
    }
    }
    else delay (2000*amountOfFlashes);
  
}

boolean playGame(int playerIndex){
  String msg = "Player "+String(playerIndex + 1)+" Score: "+String(score[playerIndex]);
  //Serial.println(msg); // Shows the player's score

  randNumber = random(3); // select a random number for the LEDs
  int minWait = (int)(400.0 * ((double)(11.0-difficulty)/10.0));
  int maxWait = minWait*2;
  delay(random(minWait,maxWait));
  playerPressed[playerIndex] = false;
  gameOn[playerIndex] = true;
  digitalWrite(ledPin[playerIndex][randNumber], HIGH);
  delay(random(minWait,maxWait));
  gameOn[playerIndex] = false;
  digitalWrite(ledPin[playerIndex][randNumber], LOW);
  if(playerPressed[playerIndex] &&!fouled[playerIndex]) {
    score[playerIndex]+= 1; // Increases the score if they pressed the button when meant to
    return true;
  }
  else if (fouled[playerIndex]){
    fouled[playerIndex] = false;
    return false;
  }
  else if(!playerPressed[playerIndex]) {
    return false;
  }
  // Loses 10 points if idle
}

void flashPlayer(int score, boolean flashRemote){
  myServo.write(servoWin);

  if(flashRemote && MASTER) sendCmd('*', score); //sends flash request to client, with score as payload
  
  for(int i = 0; i<2; i++){
    digitalWrite(whiteLED, HIGH);
    tone(buzzer,score*100);
    delay(500);
    digitalWrite(whiteLED, LOW);
    noTone(buzzer);
    delay(500);
  }
}

void ledTest(){
   for(int i = 0; i< amountOfPlayers; i++){
    for (int j = 0; j<3; j++){
      delay(100);
      digitalWrite(ledPin[i][j], HIGH);
      delay(100);
      digitalWrite(ledPin[i][j], LOW);
    }
  }
}

void debug(){
  for (int p = 0; p < amountOfPlayers; p++) {
    if(digitalRead (playerButton[p]) == HIGH) printDebug(String(playerButton[p])+"| High: "+String(p));
    else printDebug(String(playerButton[p])+"| LOW: "+String(p));
  }
}

void printDebug(String in) {
  if(debugPrint) printDebug(in);
}


void triggered() {
  // Checking to see if the player is cheating
  for (int p = 0; p < amountOfPlayers; p++) {
    if(digitalRead (playerButton[p]) == HIGH){
      printDebug(String("Match: "+String(p)));
      if(gameOn[p]) playerPressed[p] = true;
      else fouled[p] = true;
    }
  }
}

/* Please note Comms protocol as follows

For Master to client playGame:
Command (without qoutes): "a<Integer of player>"
Note that Integer of player is relevant only to the clients indexes therefore player 2 (Index starting at 0: obvs) would be 0
eg. for player 0: "a0"

For client return score back to master after request: "b<Turn won integer>"
where 1 is sucessfull and 0 is unsucessful
eg. For a sucessfull attempt: "b1", unsucessful: "b0"


*/

// General serial commands
byte* awaitHeader(char* ID, int idAmount, int bufferSize){

  String clientCommand = "";
  byte* ret = new byte[bufferSize];
  
  while (true) {
    while(Serial.available()>0) {
      char command = Serial.read();
      boolean cmdFound = false;
      
      for(int i =0; i< idAmount; i++) if(command == ID[i]) cmdFound = true;
      if (cmdFound){ 
      ret[0] = command;
      for(int i = 1; i<bufferSize; i++){
        while(!(Serial.available()>0));
        ret[i] = Serial.read();
      }
      return ret;
      }
  }
}

}

//Client to server: RTX

byte* sendCmd(char header, int payload){
   byte packet[2] = {header, payload};
   Serial.write(packet, 2);
}

void parseIncomingSerial(){ // METHOD UNFINISHED
    char expectedTypes[6] = {'$', '#','*','!', '^', '&'};
    byte* ret = awaitHeader(expectedTypes,6,2);
  switch (ret[0]){
    case '$':
    if (playGame(ret[1])) {
      sendCmd('=', 1);
      flashPlayer(score[ret[1]], false);
    }
    else {
      sendCmd('=', 0);
    }

    break;

    case '#':
    ledTest();
    break;

    case '*':
      flashPlayer(ret[1], false);
      
    break;

    case '!':
    sendCmd('!',0);
    break;

    case '^':
    difficulty = ret[1];
    break;

    case '&':
    playWinningAnimation(ret[1]);
    setVariables();
    break;
    
  }
    
}
//Server to client: TX

void updateClientDifficulty(int difficulty){
  sendCmd('^', difficulty);
}

void waitForSlave(){
  boolean slaveAlive = false;
  while(!slaveAlive){
    sendCmd('!',0);
    delay(1000);
  if(Serial.available()>0){
    if(Serial.read()== '!') slaveAlive = true;
  }

  }
 
}


void requestTurn(int remotePlayerID){ 
  sendCmd('$',remotePlayerID);
}

boolean waitForResult(){ // METHOD UNFINISHED
  char expectedTypes[1] = {'='};
  byte* ret = awaitHeader(expectedTypes,1,2);

  return (ret[1]== 1);
}

boolean executeRemotePlayer(int localID){ // Finished when UNFINISHED methods are completed
  int remotePlayerID = localID - amountOfLocalPlayers;

  requestTurn(remotePlayerID);
    
  if(waitForResult()) { // Returns boolean: True - means player hit mole, False - means player missed
    score[localID] ++;
    return true;
  }
  else return false;

}
