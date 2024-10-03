#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal.h>

struct package {
  int joystick;
  int button1;
  int button2;
};

package data;

int ackData[2] = {0, 0};

RF24 radio(9, 8); // CE, CSN pins
const byte address[6] = "ADDRESS01";

int joystickYPin = A1; // Connect joystick Y-axis to analog pin A1
const int buttonPin1 = A3;  // the number of the pushbutton pin
const int buttonPin2 = A4;  // the number of the pushbutton pin

int power = 0;

int buttonStateStart = 0;  // variable for reading the pushbutton status
int buttonStateShoot = 0;  // variable for reading the pushbutton status

long shotDelay = 0;
int ballsRemaining = 10;

const int rs = 10, en = 7, d4 = 6, d5 = 5, d6 = 4, d7 = 3;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setChannel(100);
  radio.setDataRate(RF24_250KBPS);
  radio.setRetries(5, 5); // retry if ack not received
  radio.openWritingPipe(address);
  // Initialize the LCD
  lcd.begin(16, 2);
}

void loop() {
  int yVal = analogRead(joystickYPin);
  buttonStateStart = analogRead(buttonPin1);
  buttonStateShoot = analogRead(buttonPin2);

  if (buttonStateStart > 1000 && !power) {
    power = 1;
    ballsRemaining = 10;
    data.button1 = 1;
  }

  if (buttonStateShoot > 1000 && millis() > shotDelay && ballsRemaining > 0) {
    shotDelay = millis() + 1250;
    data.button2 = 1;
    ballsRemaining--;
  }

  // Map the Y-axis joystick value to a range suitable for transmission (0-180 for servo control)
  byte yMapped = map(yVal, 0, 1023, 0, 180);

  lcd.clear();

  // Display the remaining balls on the LCD
  if (power) {
    lcd.setCursor(0, 0);
    lcd.print("Balls Remaining:");
    lcd.setCursor(0, 1);
    lcd.print(ballsRemaining);
  }

  Serial.print("Joystick Y: ");
  Serial.print(yVal);
  Serial.print("  Mapped Y: ");
  Serial.println(yMapped);
  Serial.print(". Button 1: ");
  Serial.print(buttonStateStart);
  Serial.print(". Button 2: ");
  Serial.print(buttonStateShoot);

  // Create a data buffer to hold the joystick Y-axis value
  data.joystick = yMapped;

  // Send the data
  radio.write(&data, sizeof(package));
  data.button1 = 0;
  data.button2 = 0;

  if (ballsRemaining == 0 && power) {
    data.button1 = 2;
    power = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Game Over");
    lcd.setCursor(0, 1);
    lcd.print("Turning Off...");
    delay(2000);
    radio.write(&data, sizeof(package));
    lcd.clear();
  }
  delay(50); // Adjust delay as needed
}

void gameFunction() {
  int sensorOne = analogRead(A0); // Example sensor pin
  int sensorTwo = analogRead(A1);
  int sensorThree = analogRead(A2);

  if (sensorOne > 400) {
    Serial.print("Sensor one: ");
    Serial.println(sensorOne);
    digitalWrite(2, HIGH);
    delay(1000);
    digitalWrite(2, LOW);
  } else if (sensorTwo > 400) {
    Serial.print("Sensor two: ");
    Serial.println(sensorTwo);
    digitalWrite(3, HIGH);
    delay(1000);
    digitalWrite(3, LOW);
  } else if (sensorThree > 400) {
    Serial.print("Sensor three: ");
    Serial.println(sensorThree);
    digitalWrite(4, HIGH);
    delay(1000);
    digitalWrite(4, LOW);
  }
}

void setupGame() {
  Serial.begin(9600); // Initialize serial communication
  pinMode(2, OUTPUT); // Set up LED digital pins
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  radio.begin(); // Set up nRF24L01 channel to match controller, and set to receive data only
  radio.openReadingPipe(1, address);
  radio.setChannel(100);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loopGame() {
  if (radio.available()) {
    radio.read(&data, sizeof(package)); // Read the start button value
    int start = data.button1; // Store the start button value

    if (start == 1) {
      power = 1;
      digitalWrite(2, HIGH);
      digitalWrite(3, HIGH);
      digitalWrite(4, HIGH);
      delay(500);
      digitalWrite(2, LOW);
      digitalWrite(3, LOW);
      digitalWrite(4, LOW);
    } else if (start == 2) {
      power = 0;
      digitalWrite(2, HIGH);
      digitalWrite(3, HIGH);
      digitalWrite(4, HIGH);
      delay(500);
      digitalWrite(2, LOW);
      digitalWrite(3, LOW);
      digitalWrite(4, LOW);
    }

    if (power) {
      gameFunction();
    }
  }
}
