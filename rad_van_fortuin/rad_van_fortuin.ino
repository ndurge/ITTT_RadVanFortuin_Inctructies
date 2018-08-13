#include <SimpleTimer.h>
SimpleTimer timer;



// Hoeland het duurt voordat het volgende ledje aan gaat (in miliseconde)
int nextLedDelay = 120;

// Hoeveel de delay verlaagt moet worden (in miliseconde)
const int nextLedDelayDecrease = 25;

// Hoelang het duurt voor dat het spel begint nadat de knop is gedrukt (in miliseconde)
const int startingDelay = 500;

// Hoelang het duurt voordat de fortune wheel weer verder gaat nadat de speler op de knop heeft gedrukt (in miliseconde)
const int nextTurnDelay = 800;

// Hoe hoog de toon is van het geluid
const int toneFrequency = 600;

// Hoe snel de leds knipperen (gebeurt voordat de game begint)
const int ledBlinkDelay = 500;

// Hoeveel ledjes er gestopt kunnen worden voordat het spel eindigt
const int winningLedsAmount = 3;

// Een lijst met het aantal ledjes en welke pin ze gebruiken
const int ledPins[12] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, A5};
const int ledPinsAmount = 12;

// De pin van de speaker/buzzer
const int speakerPin = A0;
// De pin van de button
const int buttonPin = A1;
// De pin van het ledje dan in de button zit
const int buttonLedPin = A2;



bool gameStarted = false;
bool gameFinished = false;

int winningLeds[winningLedsAmount];
int stoppedLeds = 0;
int currentLed;

int wheelTimerID;
int blinkTimerID;

bool blinkEvenNumbers = true;



void setup() 
{
  // Maak alles in winning leds -1, anders worden ze automatisch 0 en denkt het spel dat ledje 0 overgeslagen moet worden
  for (int i = 0; i < winningLedsAmount; i++) {
    winningLeds[i] = -1;
  }
  
  // Geef aan welke pins je wilt gebruiken voor de ledjes
  for (int i = 0; i < ledPinsAmount; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  // De pin voor de speaker
  pinMode(speakerPin, OUTPUT);

  // De pin voor de button
  pinMode(buttonPin, INPUT_PULLUP);
  // De pin van het ledje in de button
  pinMode(buttonLedPin, OUTPUT);

  // Setup a random seed
  randomSeed(analogRead(A4));

  // Ledjes knipperen in het begin
  blinkTimerID = timer.setInterval(ledBlinkDelay, blinkLeds);

  // Zet het ledje in de knop aan
  digitalWrite(buttonLedPin, HIGH);
}

void startGame()
{
  gameStarted = true;

  // Stop met knipperen knipperen
  timer.deleteTimer(blinkTimerID);

  // Doe alle ledjes uit
  for (int i = 0; i < ledPinsAmount; i++)
    digitalWrite(ledPins[i], LOW);

  // Doe het ledje in de knop uit
  digitalWrite(buttonLedPin, LOW);
  
  // Wacht even een paar miliseconde voor dat het spel start
  delay(startingDelay);

  // Doe het ledje in de knop weer aan
  digitalWrite(buttonLedPin, HIGH);

  // Begin bij een random ledje
  currentLed = random(0, ledPinsAmount+1);
  digitalWrite(ledPins[currentLed], HIGH);

  // Update het wiel om de zoveel miliseconde
  wheelTimerID = timer.setInterval(nextLedDelay, nextLed);
}

void loop() 
{
  // Als de wheel nog niet gestart is, start het wanneer de knop wordt ingedrukt
  if (!gameStarted && !gameFinished)
  {
    // Update de timer zodat het knipperen geupdate wordt
    timer.run();

    // Wanneer de knop wordt ingedrukt, begint het spel
    if (buttonPressed()) 
      startGame();
  }
  // Update het wiel als zolang de game nog niet is afgelopen
  else if (gameStarted && !gameFinished)
    gameLoop();
}

// Doet de lichjes die aanstaan uit, en die uit staan aan
// Wisselt tussen even en oneven pins
void blinkLeds()
{
  blinkEvenNumbers = !blinkEvenNumbers;
  
  for (int i = 0; i < ledPinsAmount; i++)
  {
    // Doe alle even getallen aan, doe alle oneven getallen uit
    if (blinkEvenNumbers && i % 2 == 0)
      digitalWrite(ledPins[i], HIGH);
    else if (!blinkEvenNumbers && i % 2 == 0)
      digitalWrite(ledPins[i], LOW);

    // Doe alle even getallen uit, doe alle oneven getallen aan
    if (!blinkEvenNumbers && i % 2 != 0)
      digitalWrite(ledPins[i], HIGH);
    else if (blinkEvenNumbers && i % 2 != 0)
      digitalWrite(ledPins[i], LOW);
  }
}

// De update functie van het spel
void gameLoop()
{
  // Update de timer zodat de ledjes geupdate worden
  timer.run();

  // Eindig de beurt als de knop wordt ingedrukt
  if (buttonPressed())
    endCurrentTurn();
}

// Checkt of de knop wel of niet wordt ingedrukt
bool buttonPressed()
{
  return digitalRead(buttonPin) == LOW;
}

// Eindigt de huidige beurt
void endCurrentTurn()
{
  // Voeg het huidige ledje toe aan de lijst met winnende ledjes
  winningLeds[stoppedLeds] = currentLed;

  // Houd bij hoeveel ledjes er al gestopt zijn
  stoppedLeds++;

  // Eindig het spel wanneer er genoeg winnende ledjes zijn
  if (stoppedLeds == winningLedsAmount)
    endGame();
  // Als er nog niet genoeg winnende ledjes zijn, ga dan naar de volgende beurt
  else
    startNextTurn();
}

// Eindigt het spel
void endGame()
{
  gameStarted = false;
  gameFinished = true;

  // Doe het ledje in de knop uit als het spel over is
  digitalWrite(buttonLedPin, LOW);
}

// Start de volgende beurt
void startNextTurn()
{
  // Doe het ledje in de knop uit
  digitalWrite(buttonLedPin, LOW);

  // Wacht even voordat de game weer verder gaat
  delay(nextTurnDelay);

  // Doe het ledje in de knop aan
  digitalWrite(buttonLedPin, HIGH);

  // Fortune wheel moet sneller naar elke ronde
  nextLedDelay -= nextLedDelayDecrease;
  if (nextLedDelay < 25)
      nextLedDelay = 25;

  // Reset de timer
  timer.deleteTimer(wheelTimerID);
  wheelTimerID = timer.setInterval(nextLedDelay, nextLed);

  // Ga verder vanaf een random ledje
  currentLed = random(0, ledPinsAmount);

  // Als het huidige ledje al gekozen is, probeer het dan nog eens
  if (!ledIsRemaining(currentLed))
    currentLed = random(0, ledPinsAmount);

  // Als het huidige ledje nog steeds al gekozen is, ga dan verder met die ernaast
  while(!ledIsRemaining(currentLed))
  {
    currentLed++;
    if (currentLed >= ledPinsAmount) 
      currentLed = 0;
  }
  digitalWrite(ledPins[currentLed], HIGH);

  // Doe het gekozen ledje aan
  digitalWrite(ledPins[currentLed], HIGH);
}

// Gaat naar het volgende ledje
void nextLed()
{
  // Doe het huidige ledje uit
  digitalWrite(ledPins[currentLed], LOW);

  // Bereken wat het volgende ledje moet zijn
  currentLed++;
  if (currentLed >= ledPinsAmount)
    currentLed = 0;

  // Sla de ledjes over die al gestopt zijn
  while(!ledIsRemaining(currentLed))
  {
    currentLed++;
    if (currentLed >= ledPinsAmount) 
      currentLed = 0;
  }

  // Doe het volgende ledje aan
  digitalWrite(ledPins[currentLed], HIGH);

  // Speel een geluid af elke keer dat het volgende ledje aan gaat
  playSound();
}

// Checkt of een bepaald ledje nog over is
bool ledIsRemaining(int led)
{
  for (int i = 0; i < winningLedsAmount; i++)
  {
    if (currentLed == winningLeds[i])
    {
      return false;
    }
  }
  return true;
}

// Speelt een geluid af
void playSound()
{
  tone(speakerPin, toneFrequency, nextLedDelay / 2);
}
