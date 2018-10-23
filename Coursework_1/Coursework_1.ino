#include <Servo.h> // Required library for the servo

Servo myServo;

// Sets up the number of players
int const amountOfPlayers = 2;

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
     attachInterrupt(digitalPinToInterrupt(playerButton[r]), triggered, FALLING);
  }
  for (int a = 0; a < amountOfPlayers; a ++) {
    // Loop to set up the arrays in a better format, if more players are added
    gameOn[a] = false;
    playerPressed[a] = false;
    fouled[a] = false;
    score[a] = 0;
  }

  pinMode(whiteLED, OUTPUT);
  for(int i =0; i< amountOfPlayers; i++){
  pinMode(playerButton[i], INPUT);
  for (int j=0; j<3; j++){
    pinMode(ledPin[i][j], OUTPUT);
  }
  }
}

//run main program loop
void loop() {
  // Runs the play game function for each of the players
  for(int e = 0; e < amountOfPlayers; e++) {
    playGame(e);
  }
}

void playGame(int playerIndex){
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
  if(playerPressed[playerIndex] &&!fouled) {
    score[playerIndex]+=10; // Increases the score if they pressed the button when meant to
    flashPlayer();
  }
  else if (fouled){
    // Player loses 50 points if cheating
    score[playerIndex]-= 50;
    fouled[playerIndex] = false;
  }
  else if(!playerPressed[playerIndex]) score[playerIndex]-=10; // Loses 10 points if idle
  
 delay(1000);
  myServo.write(servoLoose);
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

void triggered() {
  // Checking to see if the player is cheating
  for (int p = 0; p < amountOfPlayers; p++) {
    if(playerButton[p] == LOW){
    if(gameOn[p]) playerPressed[p] = true;
    else fouled[p] = true;
    }
  }

 
}
