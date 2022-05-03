// ---------------------------------------------------------------- //
// Tensile Strength Tester v1.2
// ECE442 Spring 2022
// Team 5 | Team Point Break
// Brianna Lossow, Alexander Karampelas, Omar Elmejjati
// ---------------------------------------------------------------- //

#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>   // Enables use of touch screen
#include <HX711.h>         // Enables use of load cell
#include <SD.h>            // Enables use of SD card

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

//Touch For New ILI9341 TP
#define TS_MINX 120
#define TS_MAXX 900
#define TS_MINY 70
#define TS_MAXY 920

// Pins for LCD screen
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4 // resets LCD when reset button is pressed

// Assign names to some common 16-bit color values:
#define BLACK     0x0000
#define DARKGREY  0x7BEF
#define LIGHTGREY 0xC618
#define BLUE      0x000F
#define RED       0xF800
#define ORANGE    0xFBE0
#define GREEN     0x03E0
#define CYAN      0x03EF
#define MAGENTA   0xF81F
#define PURPLE    0x80F9
#define YELLOW    0xFFE0
#define WHITE     0xFFFF

// UI details
#define BUTTON_X 120
#define BUTTON_Y 40
#define BUTTON_W 200
#define BUTTON_H 60
#define BUTTON_SPACING_X 20
#define BUTTON_SPACING_Y 20
#define BUTTON_TEXTSIZE 2.5

// Touch Screen
#define MINPRESSURE 5
#define MAXPRESSURE 1000

// For printed messages
#define STATUS_X 5
#define STATUS_Y 5

// Ultrasonic HC-SR04
#define trigPin 25
#define echoPin 27

// Load Cell HX711
#define loadcellDOUTPin  29
#define loadcellSCKPin   31

// Buzzer
#define buzzerPin 33

// Resistance between X+ and X- is used for best pressure precision
// For our touchscreen, it's 300 Ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

uint16_t identifier;
Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TSPoint touch;
int touchLocation[2];

/* MENU */
Elegoo_GFX_Button menubuttons[4];
char menubuttonlabels[4][15] = {"Start", "Calibrate", "Export", "Reset"};
uint16_t menubuttoncolors[4] = {GREEN, CYAN, DARKGREY, RED};

/* CALIBRATE */
Elegoo_GFX_Button calibratebuttons[4];
char calibratebuttonlabels[4][10] = {"+", "-", "Done", "Cancel"};
uint16_t calibratebuttoncolors[4] = {GREEN, RED, CYAN, DARKGREY};

// states of the system
enum states {
  MENU,
  RUN,
  CALIBRATE,
  EXPORT,
  RESET,
  FRACTURE,
  WEIGHT_ERROR,
  SD_ERROR,
  FILE_ERROR
};

states screenState;

const float KILOGRAM_NEWTON_CONVERSION = 9.81;      // stored as m/s
const float CALIBRATION_SLOPE = 105;
const float MAVG_WEIGHT = 0.1;

// defines variables
float originalDistance;
float currentDistance;
float nextDistance; // variable for the distance measurement
float calibrationIntercept = 152500;
float prevCalibrationIntercept;

HX711 scale;
float previousWeight = 0;
float weight = previousWeight;

float appliedForce = 0; // weight * gravity
float strain = 0; // (displacement - length) / length

char fileName[] = "TEST000.CSV";
File sdFile;

// Sourced from example files included with the board - tftpaint.ino
uint16_t identifytft() {
  uint16_t identifier = tft.readID();
  if (identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if (identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if (identifier == 0x4535) {
    Serial.println(F("Found LGDP4535 LCD driver"));
  } else if (identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if (identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if (identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else if (identifier == 0x0101) {
    identifier = 0x9341;
    Serial.println(F("Found 0x9341 LCD driver"));
  } else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Elegoo 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Elegoo_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    Serial.println(F("Also if using the breakout, double-check that all wiring"));
    Serial.println(F("matches the tutorial."));
    identifier = 0x9341;
  }
  return identifier;
}

/*
 * Function: resetFunc
 * ----------------------------
 *   Resets the Arduino program, similarly to pressing the reset button
 */
void(* resetFunc) (void) = 0; //declare reset function at address 0

/*
 * Function: refreshScreen
 * ----------------------------
 *   Redraws the screen for each state change in the system
 */
void refreshScreen() {
  tft.reset(); // clears the screen
  tft.begin(identifier);
  tft.setRotation(0);   // sets vertical layout for screen
  tft.fillScreen(BLACK);  // sets the screen to black to preserve the users' eyes
}

/*
 * Function: status
 * ----------------------------
 *   Displays a message to the touch screen
 *
 *   msg: the message to display to the screen
 */
void status(char *msg) {
  tft.setCursor(STATUS_X, STATUS_Y);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print(msg);
}

/*
 * Function: detectTouch
 * ----------------------------
 *   Determines the function to call depending on the current state of the system
 *   and where the touch is detected
 */
void detectTouch() {
  // determines touch location
  touch.x = map(touch.x, TS_MINX, TS_MAXX, tft.width(), 0);
  touch.y = (tft.height() - map(touch.y, TS_MINY, TS_MAXY, tft.height(), 0));
  touchLocation[0] = touch.x;
  touchLocation[1] = touch.y;
  Serial.print("("); Serial.print(touchLocation[0]); Serial.print(",");
  Serial.print(touchLocation[1]); Serial.println(")");

  switch (screenState) {
    /* MAIN MENU */
    case MENU:
      if (touchLocation > 10 && touchLocation < 70) {
        screenState = RESET;
      }
      else if (touchLocation[1] > 80 && touchLocation[1] < 150) {
        screenState = EXPORT;
      }
      else if (touchLocation[1] > 165 && touchLocation[1] < 230) {
        refreshScreen();
        prevCalibrationIntercept = calibrationIntercept;
        status("Place known weight in the top clamp, \nthen increase or \ndecrease the \ncalibration until \nweight is accurate.");
        delay(2000);
        refreshScreen();
        screenState = CALIBRATE;
      }
      else if (touchLocation[1] > 240 && touchLocation[1] < 310) {
        screenState = RUN;
      }
      break;
    /* RUNNING */
    case RUN:

      break;
    /* CALIBRATE */
    case CALIBRATE:
      if (touchLocation[1] > 130 && touchLocation[1] < 190) {
        if (touchLocation[0] > 10 && touchLocation[0] < 110) {
          //increase calibration factor
          calibrationIntercept += 100;
        }
        else if (touchLocation[0] > 120 && touchLocation[0] < 230) {
          //decrease calibration factor
          calibrationIntercept -= 100;
        }
      }
      else if (touchLocation[1] > 60 && touchLocation[1] < 115) {
        screenState = MENU;
      }
      else if (touchLocation[1] > 0 && touchLocation[1] < 55) {
        calibrationIntercept = prevCalibrationIntercept;
        screenState = MENU;
      }
      break;
    /* EXPORT */
    case EXPORT:
      screenState = MENU;
      break;
    /* FRACTURE */
    case FRACTURE:
      screenState = MENU;
      break;
    /* WEIGHT ERROR */
    case WEIGHT_ERROR:
      screenState = RESET;
      break;
    /* SD CARD ERROR */
    case SD_ERROR:
      screenState = RESET;
      break;
    /* FILE ERROR */
    case FILE_ERROR:
      screenState = RESET;
      break;
  }
  screenStatus();
}

/*
 * Function: screenStatus
 * ----------------------------
 *   Draws the current screen depending on the current state of the system
 */
void screenStatus() {
  refreshScreen();
  switch (screenState) {
    /* MAIN MENU */
    case MENU:   // main menu
      // create buttons
      for (int row = 0; row < 4; row++) {
        // "start", "calibrate", "export", "reset"
        menubuttons[row].initButton(&tft, (BUTTON_X),
                                    (BUTTON_Y + (row * (BUTTON_H + BUTTON_SPACING_Y))), BUTTON_W, BUTTON_H,
                                    BLACK, menubuttoncolors[row], WHITE, menubuttonlabels[row], BUTTON_TEXTSIZE);
        menubuttons[row].drawButton();
      }
      break;
    /* RUNNING */
    case RUN:
      tft.setCursor(0, 0);
      tft.setTextSize(2);
      tft.print("Initializing...");
      assignFileName();

      delay(800);
      tft.setCursor(0, 20);
      tft.print("Done.");
      tft.setCursor(0, 70);
      tft.print("Data will be saved\nto ");
      tft.setCursor(40, 90);
      tft.print(fileName);

      delay(500);
      tft.setCursor(0, 180);
      tft.print("Begin pulling sample");
      break;
    /* CALIBRATE */
    case CALIBRATE:
      // displays weight from load cell to screen with each refresh
      status("Current Weight:");
      tft.setCursor(35, 50);
      tft.setTextSize(4);
      tft.print(getWeight(), 3);
      tft.print("kg");
      // create buttons
      for (int row = 0; row < 2; row++) {
        // + and - buttons
        calibratebuttons[row].initButton(&tft, 8 + (BUTTON_X / 2) + row * ((BUTTON_W / 2) + 5),
                                         60 + BUTTON_Y + (BUTTON_H + (BUTTON_SPACING_Y / 5)), BUTTON_W / 2,
                                         BUTTON_H, BLACK, calibratebuttoncolors[row], WHITE,
                                         calibratebuttonlabels[row], BUTTON_TEXTSIZE);
        calibratebuttons[row].drawButton();
      }
      for (int row = 2; row < 4; row++) {
        // "done" & "cancel" buttons
        calibratebuttons[row].initButton(&tft, (BUTTON_X),
                                         (60 + BUTTON_Y + (row * (BUTTON_H + BUTTON_SPACING_Y / 5))), BUTTON_W, BUTTON_H,
                                         BLACK, calibratebuttoncolors[row], WHITE, calibratebuttonlabels[row],
                                         BUTTON_TEXTSIZE);
        calibratebuttons[row].drawButton();
      }
      break;
    /* EXPORT */
    case EXPORT:
      tft.setCursor(0, 0);
      tft.setTextSize(2);
      exportData();
      tft.print("Exported\nsuccessfully!\n\nPlease remove the\nSD card.");
      tft.setCursor(0, 90);
      delay(1500);
      tft.print("Touch anywhere to terminate program...");
      break;
    /* RESET */
    case RESET:
      resetFunc();
      break;
    /* FRACTURE */
    case FRACTURE:
      tft.setCursor(0, 0);
      tft.setTextSize(2);
      tft.print("Fracture detected!\nSaving data...");
      delay(500);
      exportData();
      tft.setCursor(0, 70);
      tft.print("Done.");
      tft.setCursor(0, 90);
      tft.print("Touch anywhere to\nreturn to main menu.");
      break;
    /* WEIGHT ERROR */
    case WEIGHT_ERROR:
      tft.setCursor(50, 5);
      tft.setTextSize(4);
      tft.print("ERROR!");
      tft.setCursor(0, 45);
      tft.setTextSize(3);
      tft.print("Weight\ncapacity\nexceeded.\n\nTouch anywhere to return to main menu...");
      break;
    /* SD CARD ERROR */
    case SD_ERROR:
      tft.setCursor(50, 5);
      tft.setTextSize(4);
      tft.print("ERROR!");
      tft.setCursor(0, 45);
      tft.setTextSize(2);
      tft.print("SD couldn't be\naccessed. Please\nmake sure the SD\ncard is inserted\nand has available\nstorage space.");
      tft.setCursor(0, 200);
      tft.print("Touch anywhere to\nreturn to main\nmenu...");
      break;
    /* FILE ERROR */
    case FILE_ERROR:
      tft.setCursor(50, 5);
      tft.setTextSize(4);
      tft.print("ERROR!");
      tft.setCursor(0, 45);
      tft.setTextSize(2);
      tft.print(fileName);
      tft.setCursor(0, 70);
      tft.print(" couldn't be\naccessed. Please\nmake sure the SD\ncard is inserted\nand has available\nstorage space.");
      tft.setCursor(0, 200);
      tft.print("Touch anywhere to\nreturn to main\nmenu...");
      break;
  }
}

/*
 * Function: getDistance
 * ----------------------------
 *   Returns the distance in millimeters from the HR-SR04 Ultrasonic Range Sensor
 *
 *   returns: the distance in millimeters
 */
float getDistance() {
  float dist = 0;

  for (int i = 0; i < 4; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    long duration = pulseIn(echoPin, HIGH);

    dist += (duration * 0.034 / 2.0) * 10; // distance in millimeters
  }
  dist = dist / 4;

  return dist;
}

/*
 * Function: getWeight
 * ----------------------------
 *   Returns the weight in kilograms from the HX711 load cell
 *
 *   returns: the weight in kilograms
 */
float getWeight() {
  float kg = 0;

  kg = (scale.read_average(2) - calibrationIntercept);
  kg = kg / CALIBRATION_SLOPE;
  kg = kg / 1000.0;

  return kg;
}

/*
 * Function: measureTensile
 * ----------------------------
 *   Measures the tensile strength of an object
 *   Uses getDistance & getWeight
 */
void measureTensile() {

  sdFile = SD.open(fileName, FILE_WRITE);
  originalDistance = getDistance();
  currentDistance = originalDistance;

  if (sdFile) {
    Serial.print(fileName);
    Serial.println(" found!");
    sdFile.println("Strain (mm/mm),Applied Force (N)");
  }
  else {
    Serial.print("Error opening ");
    Serial.print(fileName);
    Serial.println(". Exiting...");
    screenState = FILE_ERROR;
    screenStatus();
    return;
  }

  for (int i = 0; i < 70; i++) {
    nextDistance = getDistance();
    // applies moving average to smooth data curve
    currentDistance = (MAVG_WEIGHT * nextDistance) + ((1 - MAVG_WEIGHT) * currentDistance);

    previousWeight = weight;
    weight = getWeight();
    appliedForce = weight * KILOGRAM_NEWTON_CONVERSION;
    strain = abs(currentDistance - originalDistance) / originalDistance;

    if (weight > 18) { // warning for max weight
      Serial.println("WARNING! Approaching weight capacity.");
      tone(buzzerPin, 440);
      delay(500);
    }
    noTone(buzzerPin);

    Serial.print(strain, 2);
    Serial.print(", ");
    Serial.print(appliedForce, 3);
    Serial.println();

    sdFile.print(strain, 2);
    sdFile.print(",");
    sdFile.print(appliedForce, 3);
    sdFile.println();
    
  }
  
  screenState = FRACTURE;
  screenStatus();
}

/*
 * Function: addOne
 * ----------------------------
 *   Adds 1 to the file name, allowing for creation of multiple files without removing the SD card all the time
 *
 *   bgn: beginning of number in file name
 *   end: end of number in file name
 *
 *   returns: true if the addition was successful, false if failed 
 */
bool addOne(char* bgn, char* end) {
  while (1) {
    if (*end != '9') {
      *end += 1;
      return true;
    }
    *end = '0';
    if (end-- == bgn) {
      return false;
    }
  }
}

/*
 * Function: assignFileName
 * ----------------------------
 *   Continuously increases until a file can be created in an empty slot
 */
void assignFileName() {
  Serial.println(fileName);
  while (SD.exists(fileName)) {
    if (!addOne(&fileName[4], &fileName[6])) {  // if TEST000 - TEST999 are all used up
      Serial.println("ERROR: Out of file space!");
      while (1);
    }
  }
}

/*
 * Function: exportData
 * ----------------------------
 *   Closes the current open SD card file
 */
void exportData() {
  sdFile.close();
}

/*
 * Function: setup
 * ----------------------------
 *   Runs once at startup, allowing for system initialization
 */
void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  Serial.begin(9600);
  Serial.println("Tensile Strength Tester v1.2");

  identifier = identifytft();

  screenState = MENU;

  Serial.println("Initializing SD Card...");
  pinMode(SS, OUTPUT);
  if (!SD.begin(10, 11, 12, 13)) {    // force software SPI due to touch screen shield & arduino mega incompatibility
    Serial.println("SD initialization failed!");
    screenState = SD_ERROR;
  }
  else {
    Serial.println("SD initialization done.");
  }

  scale.begin(loadcellDOUTPin, loadcellSCKPin);
  scale.tare();  //Reset the scale to 0

  screenStatus();
}

/*
 * Function: loop
 * ----------------------------
 *   Detects input on the touch screen
 *   If screenState is RUN, calls measureTensile
 */
void loop() {
  digitalWrite(13, HIGH);
  touch = ts.getPoint();
  digitalWrite(13, LOW);

  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (touch.z > MINPRESSURE && touch.z < MAXPRESSURE) { // if touch is detected; pressure defined by resistance
    detectTouch();

    if (screenState == RUN) {
      measureTensile();
    }
  }
  delay(10);
}
