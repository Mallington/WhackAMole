#include <Servo.h> // Required library for the servo

boolean MASTER = false;
//boolean MASTER = true;

int NORMAL =0;
int SLAVE = 1;

boolean playerWon = false;

// IMPORTANT PARAMETER: This states what mode the arduino is in
//int MODE = NORMAL;
int MODE = SLAVE;

Servo myServo;

// Sets up the number of players
int const amountOfPlayers = 2;
int const amountOfLocalPlayers = 2;

// assign LEDs and button to pins
int ledPin[amountOfPlayers][3] = {{4,5,6},{11,12,13}}; // ** Changed to 13 to fix weird light issue **

int playerButton[amountOfPlayers] = {2, 3};

boolean gameOn[amountOfPlayers];
boolean playerPressed[amountOfPlayers];
int score[amountOfPlayers];


//int playerOneButton = 2;
int whiteLED = 9;
//int score =0;
int servoPin = 10;
int servoWin = 10;
int servoLoose = 180;
int randNumber;
boolean fouled[amountOfPlayers];

//setup interrupt, button input and LED outputs
void setup() {
  Serial.begin(9600); // Connects to the serial monitor for printing
  myServo.attach(servoPin);

  for (int r =0; r < amountOfPlayers; r++) {
     attachInterrupt(digitalPinToInterrupt(playerButton[r]), triggered,RISING);
  }
  setVariables();
  pinMode(whiteLED, OUTPUT);
  for(int i =0; i< amountOfPlayers; i++){
  pinMode(playerButton[i], INPUT);
  for (int j=0; j<3; j++){
    pinMode(ledPin[i][j], OUTPUT);
  }
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

//run main program loop
void loop() {
  // Runs the play game function for each of the players

  if(!playerWon){

    switch(MODE){

      case 0:
      int e;
      for(e = 0; e < amountOfLocalPlayers; e++) {
          if(playGame(e)) flashPlayer();
      }
      if(MASTER) {
        while (e < amountOfPlayers) if(executeRemotePlayer(e++)) flashPlayer();
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

boolean playGame(int playerIndex){
  String msg = "Player "+String(playerIndex + 1)+" Score: "+String(score[playerIndex]);
  Serial.println(msg); // Shows the player's score

  randNumber = random(3); // select a random number for the LEDs

  delay(random(400,800));
  playerPressed[playerIndex] = false;
  gameOn[playerIndex] = true;
  digitalWrite(ledPin[playerIndex][randNumber], HIGH);
  delay(random(400,800));
  gameOn[playerIndex] = false;
  digitalWrite(ledPin[playerIndex][randNumber], LOW);
  if(playerPressed[playerIndex] &&!fouled[playerIndex]) {
    score[playerIndex]+=1; // Increases the score if they pressed the button when meant to
    return true;
  }
  else if (fouled[playerIndex]){
    // Player loses 50 points if cheating
    score[playerIndex]-= 50;
    fouled[playerIndex] = false;
    return false;
  }
  else if(!playerPressed[playerIndex]) {
    score[playerIndex]-=1;
    return false;
  }
  // Loses 10 points if idle
}

void flashPlayer(){
  myServo.write(servoWin);
  for(int i =0; i<2; i++){
    digitalWrite(whiteLED, HIGH);
    delay(500);
    digitalWrite(whiteLED, LOW);
    delay(500);
  }
}

void debug(){
  for (int p = 0; p < amountOfPlayers; p++) {
    if(digitalRead (playerButton[p])== HIGH) Serial.println(String(playerButton[p])+"| High: "+String(p));
    else Serial.println(String(playerButton[p])+"| LOW: "+String(p));
}
}


void triggered() {
  Serial.println("Triggered");
  // Checking to see if the player is cheating
  for (int p = 0; p < amountOfPlayers; p++) {
    if(digitalRead (playerButton[p]) == HIGH){
      Serial.println(String("Match: "+String(p)));
      if(gameOn[p]) playerPressed[p] = true;
      else fouled[p] = true;
    }
  }
}

/* Please note Comms protocol as follows

For Master to client playGame:
Command (without qoutes): "$<Integer of player>"
Note that Integer of player is relevant only to the clients indexes therefore player 2 (Index starting at 0: obvs) would be 0
eg. for player 0: "$0"

For client return score back to master after request: "=<Turn won integer>"
where 1 is sucessfull and 0 is unsucessful
eg. For a sucessfull attempt: "=1", unsucessful: "=0"


*/

//Client to server: RTX

void parseIncomingSerial(){ // METHOD UNFINISHED
  String serverCommand = ""; // Holds incoming command;
  boolean successful = false;
  while(Serial.available()>0) serverCommand+=String((char)Serial.read());
  if (serverCommand[0] == '$') successful = playGame(serverCommand[1]);
  if (successful) Serial.write("=1");
  else Serial.write("=0");
}


//Server to client: TX

void requestTurn(int remotePlayerID){ // METHOD UNFINISHED
  byte command[2] = {'$',remotePlayerID};
  Serial.write(command,2);
}

boolean waitForResult(){ // METHOD UNFINISHED
  boolean wackedMole = false;
  String clientCommand = "";
  while(Serial.available()>0) clientCommand+=String((char)Serial.read());
  if (clientCommand[0] == '=') wackedMole = true;
  return wackedMole; // returns whether player was successful or not
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
