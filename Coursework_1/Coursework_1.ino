#include <Servo.h>

Servo myServo;

int const amountOfPlayers = 2;

// assign LEDs and button to pins
int ledPin[amountOfPlayers][3] = {{4,5,6},{10,11,12}};

int playerButton[amountOfPlayers] = {2, 3};

boolean gameOn[amountOfPlayers] = {false, false};
boolean playerPressed[amountOfPlayers] = {false, false};

int score[amountOfPlayers] = {0,0};

//int playerOneButton = 2;
int whiteLED = 9;

//int score =0;

int servoPin = 10;
int servoWin = 10;
int servoLoose = 180;
int randNumber;

boolean fouled[amountOfPlayers] = {false,false};

// declare variables
//int delayTime; // time delay between lights on/off
//int randNumber;
//int whiteLEDOn;



//boolean player1Pressed = false;
//boolean gameOn1 = false;

//setup interrupt, button input and LED outputs
void setup() {
  Serial.begin(9600);
  myServo.attach(servoPin);
  for (int r =0; r < amountOfPlayers; r++) {
     attachInterrupt(digitalPinToInterrupt(playerButton[r]), triggered, FALLING);
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

  playGame(0);
  playGame(1);
  
}

void playGame(int playerIndex){
  String msg = "Player "+String(playerIndex)+" Score: "+String(score[playerIndex]);
  Serial.println(msg);
  
  randNumber = random(3); // select a random number
  
  delay(random(100, 400));
  playerPressed[playerIndex] = false;
  gameOn[playerIndex] = true;
  digitalWrite(ledPin[playerIndex][randNumber], HIGH);
  delay(random(200, 400)*2);
  gameOn[playerIndex] = false;
  digitalWrite(ledPin[playerIndex][randNumber], LOW);
  if(playerPressed[playerIndex] &&!fouled) {
   
    score[playerIndex]+=10;
    flashPlayer();
  }
  else if (fouled){
   
    score[playerIndex]-= 50;
    fouled[playerIndex] = false;
  }
  else if(!playerPressed[playerIndex]) score[playerIndex]-=10;
  
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
  Serial.println(digitalRead(2));
  if(gameOn[0]) playerPressed[0] = true;
  else fouled[0] = true;
  
  
  /*change this code so that white LED only switches on when button is pressed 
  at the right time*/
}
