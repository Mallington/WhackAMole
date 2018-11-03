#include <Servo.h> // Required library for the servo

int NORMAL = 0;
int SLAVE = 1;


// IMPORTANT PARAMETER: This states what mode the arduino is in
int MODE = SLAVE;
boolean MASTER = false;

boolean playerWon = false;
boolean debugPrint = false;



// Sets up the number of players
int const amountOfPlayers = 4;
int const amountOfLocalPlayers = 2;
int score[amountOfPlayers];
int difficulty = 1;

Servo servo;
int servoPin = 10;
int servoPos[amountOfPlayers] = {180,120,70,20};

// assign LEDs, buttons and the servo to their pins
int ledPin[amountOfPlayers][3] = {{4,5,6},{11,12,13}};
int playerButton[amountOfPlayers] = {2, 3};

int whiteLED = 8;
int buzzer = 9;
int servoWin = 10;
int servoLose = 180;
int difficultyDial = A0;


boolean gameOn[amountOfPlayers];
boolean playerPressed[amountOfPlayers];




int randNumber;
boolean fouled[amountOfPlayers];


void setup() {
//setup interrupt, button input and LED outputs
  Serial.begin(9600); // Connects to the serial monitor for printing
  servo.attach(servoPin);
  servo.write(0);
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
// Sets up the variables for each of the players
  playerWon = false;
  for (int a = 0; a < amountOfPlayers; a ++) {
    // Loop to set up the arrays in a better format, if more players are added
    gameOn[a] = false;
    playerPressed[a] = false;
    fouled[a] = false;
    score[a] = 0;
  }
}

void setPlayerDial(int playerID){
  if(MASTER){
    sendCmd('(', servoPos[playerID]);
  }
}

int readDifficultyDial(){
// Reads the potentiometer's resistance and maps it to a difficulty level
  int sensorValue = analogRead(difficultyDial);
  return map(sensorValue, 0, 1023, 0, 10);
}

int updateDifficulty(){
// Changes the difficulty of the game depending on the potentiometer
    difficulty = readDifficultyDial();
    if(MASTER) updateClientDifficulty(difficulty);
}

void loop() {
// Main loop for the program. Runs play game for the local player, as well as handling the comms between the master and slave
  if(!playerWon){
    switch(MODE){
      case 0:
      updateDifficulty();
      int e;
      for(e = 0; e < amountOfLocalPlayers; e++) {
        setPlayerDial(e);
          if(playGame(e)) flashPlayer(score[e], true);
      }
      if(MASTER) {
        while (e < amountOfPlayers) {
          setPlayerDial(e);
          if(executeRemotePlayer(e++)) {
            flashPlayer(score[e],false);
          }
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

}
int getWinningPlayer(){
// Returns the player number of the winner
  for (int a = 0; a < amountOfPlayers; a ++) {
    if(score[a]>=10) return a;
  }
  return -1; // Captures if there is an error
}

void setPlayerLEDs(int playerID, int value){
// Sets up all the LEDs for a specific player
  for (int j = 0; j<3; j++){
      digitalWrite(ledPin[playerID][j],value);
    }
}

void setAllLEDs(int value){
// Sets up all the LEDs for all of the players
  for(int i = 0; i< amountOfPlayers; i++){
    for (int j = 0; j<3; j++){
      digitalWrite(ledPin[i][j],value);
    }
  }
}

void playWinningAnimation(int playerID){
// Lights up all of the LEDs to show game has ended
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
  // Play game function for a specific player. Returns true if point is won
  randNumber = random(3); // select a random number for the LEDs
  int minWait = (int)(400.0 * ((double)(11.0-difficulty)/10.0));
  int maxWait = minWait*2; // Varies the time the LED is on based on the difficulty
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
// Checks to see if player is spamming the button/pressed it at the wrong time
    fouled[playerIndex] = false;
    return false;
  }
  else if(!playerPressed[playerIndex]) {
// Returns false if the player didn't press the button
    return false;
  }
}

void flashPlayer(int score, boolean flashRemote){
// Flashes the white LEDs, moves the servos and plays a noise on all of the boards
  
  if(flashRemote && MASTER) sendCmd('*', score); //sends flash request to client, with score as payload
  for(int i = 0; i<2; i++){
    digitalWrite(whiteLED, HIGH);
    tone(buzzer,score*100); // Plays a tone based on the player's score
    delay(500);
    digitalWrite(whiteLED, LOW);
    noTone(buzzer);
    delay(500);
  }
}

void ledTest(){
// Lights up all of the LEDs in order to show all is working
   for(int i=0; i<=180; i++){
    servo.write(180);
    delay(10);
   }
   servo.write(0);

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
// For debugging purposes, while developing
  for (int p = 0; p < amountOfPlayers; p++) {
    if(digitalRead (playerButton[p]) == HIGH) printDebug(String(playerButton[p])+"| High: "+String(p));
    else printDebug(String(playerButton[p])+"| LOW: "+String(p));
  }
}

void printDebug(String in) {
// For debugging purposes, to avoid serial issues
  if(debugPrint) printDebug(in);
}


void triggered() {
  // Main point of entry when a button is pressed
  for (int p = 0; p < amountOfPlayers; p++) {
    if(digitalRead (playerButton[p]) == HIGH){ // Finds which button is pressed
      printDebug(String("Match: "+String(p)));
      if(gameOn[p]) playerPressed[p] = true; // Checks if player mispressed
      else fouled[p] = true;
    }
  }
}

/* Please note Comms protocol as follows

For Master to client playGame:
Command (without quotes): "$<Integer of player>"
Note that Integer of player is relevant only to the clients indexes therefore player 2 (Index starting at 0: obvs) would be 0
eg. for player 0: "$0"

For client return score back to master after request: "=<Turn won integer>"
where 1 is sucessfull and 0 is unsucessful
eg. For a sucessfull attempt: "=1", unsucessful: "=0"

There are additional command functions for variable maintenance, to initiate animations on the clave board, check requests to see if board is alive etc. See below:

Play LED test:
#<null>

Play Flash Player Animation
*<playerID>

Check if slave is alive
!<null>
Response:  !<null>

Set difficulty level:
^<difficultyInteger>

Play winning animation
&<winningPlayerID>

*/

// General serial commands
byte* awaitHeader(char* ID, int idAmount, int bufferSize){
// Checking to see if the incoming serial command is of the correct format (e.g. an “=” for the result command)
  String clientCommand = "";
  byte* ret = new byte[bufferSize]; // Incoming bytes
  
  while (true) {
    while(Serial.available()>0) {
      char command = Serial.read();
      boolean cmdFound = false;
      for(int i =0; i< idAmount; i++) if(command == ID[i]) cmdFound = true; // Checks to see if desired a header exists
      if (cmdFound){ // If a header is found it enters
        ret[0] = command; // header
        for(int i = 1; i<bufferSize; i++){ // loops until desired buffer is filled
          while(!(Serial.available()>0)); // Waits for next incoming byte, before filling buffer
          ret[i] = Serial.read();
          }
        return ret;
      }
    }
  }
}

//Client to server: RTX

byte* sendCmd(char header, int payload){
// Sends the result of play game to the server
   byte packet[2] = {header, payload}; 
   Serial.write(packet, 2);
}

void parseIncomingSerial(){
// Checks all incoming serial commands, then parses only the needed ones
    char expectedTypes[7] = {'$', '#','*','!', '^', '&', '('}; // Expected header types
    byte* ret = awaitHeader(expectedTypes,7,2); // Waits for one of the headers
  switch (ret[0]){ //Based on header, the slave executes accordingly
    case '$': //Play game command
    if (playGame(ret[1])) {
      sendCmd('=', 1);
      flashPlayer(score[ret[1]], false);
    }
    else {
      sendCmd('=', 0);
    }
    break;

    case '#': // Start up LED test command
    ledTest();
    break;

    case '*': // Flash player command, if player is successful
      flashPlayer(ret[1], false);
    break;

    case '!': // Is dobby alive?: Checks to see if slave is alive
// Note: If master does not receive, Dobby has his socks
    sendCmd('!',0);
    break;

    case '^': //Set difficulty command
    difficulty = ret[1];
    break;

    case '&': // Play winning animation command
    playWinningAnimation(ret[1]);
    setVariables();
    break;

    case '(': // Sets servo position
    servo.write(ret[1]);
    break;
  }
}
//Server to client: TX

void updateClientDifficulty(int difficulty){
// Sends the change in difficulty to the client
  sendCmd('^', difficulty);
}

void waitForSlave(){
// Waits for the client’s response before continuing
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
// Sends the command to trigger the turn on the client’s side
  sendCmd('$',remotePlayerID);
}

boolean waitForResult(){ 
// Waits for the “=” sign to check to see if the client’s player succeeded or not
  char expectedTypes[1] = {'='};
  byte* ret = awaitHeader(expectedTypes,1,2);
  return (ret[1]== 1);
}

boolean executeRemotePlayer(int localID){
// Triggers the request for the client’s game, then increases their score if they were successful
  int remotePlayerID = localID - amountOfLocalPlayers;
  requestTurn(remotePlayerID);  
  if(waitForResult()) { // Returns boolean: True - means player hit mole, False - means player missed
    score[localID] ++;
    return true;
  }
  else return false;
}
