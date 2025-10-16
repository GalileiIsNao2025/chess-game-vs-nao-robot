#include "SmartKit.h"
#include <Arduino.h>
#include <EEPROM.h>
#include <FastLED.h>

// LED Configuration
#define LED_PIN     7
#define NUM_LEDS    64
#define BRIGHTNESS  50
CRGB leds[NUM_LEDS];

// === GLOBALS AND CONSTANTS ===
// These MUST be at the very top of the file, before any function uses them

float Z0 = -15;              // Board surface (no gripper)
float Z_gripper_zero = -15;  // Gripper tip on board

// Safety limits
#define MIN_Z_HEIGHT -150.0  // Modified to accommodate lower Z values
#define MAX_Z_HEIGHT 250.0
#define MIN_X_COORD 100.0
#define MAX_X_COORD 300.0
#define MIN_Y_COORD -100.0   // Modified to accommodate negative Y values
#define MAX_Y_COORD 300.0

// Maximum number of captured pieces per side
#define MAX_CAPTURED_PIECES 16

// Gripper XY offset relative to calibration tool (to center gripper on square)
float GRIPPER_OFFSET_X = 0.0; // Set after calibration (mm)
float GRIPPER_OFFSET_Y = 0.0; // Set after calibration (mm)
// To calibrate: place a pawn at a known square center, move gripper to calculated center, and measure X/Y offset needed to perfectly center gripper over pawn.

#define PIECE_HEIGHT_PAWN    32.097
#define PIECE_HEIGHT_ROOK    33.833
#define PIECE_HEIGHT_KNIGHT  43.674
#define PIECE_HEIGHT_BISHOP  47.894
#define PIECE_HEIGHT_QUEEN   49.984
#define PIECE_HEIGHT_KING    53.676

// Helper function to get pickup Z for a piece type (relative to gripper zero)

#include "Dobot.h"

// Set to 1 to try to use Dobot_GetPose (may cause linking errors)
// Set to 0 to use predefined coordinates
#define USE_DOBOT_GETPOSE 1

// (Corner calibration removed: now calibrate only edge square centers)

float matrix[8][8][3];  // Will be calculated based on corner positions

#if USE_DOBOT_GETPOSE
// Function to get coordinate by position type
float getCoordinate(int posType) {
    return Dobot_GetPose((Pos)posType); 
}
#endif

// EEPROM addresses for calibration data
#define EEPROM_CALIBRATION_VALID 0    // 1 byte
#define EEPROM_Z0 1                   // 4 bytes
#define EEPROM_Z_GRIPPER_ZERO 5       // 4 bytes
#define EEPROM_MATRIX_START 9         // 8*8*3*4 = 768 bytes

// Higher pickup height for safety
#define SAFE_PICKUP_HEIGHT 35  // Increased height for piece pickup

// Altezze di movimento sicure
#define SAFE_TRAVEL_HEIGHT 37.0    // Altezza di viaggio normale
#define SAFE_TRAVEL_HEIGHT_EDGE 27.0 // Altezza di viaggio ridotta per colonne a e h
#define PIECE_GRAB_OFFSET 5.0      // Offset per la presa dei pezzi

// Velocità di movimento
#define FAST_SPEED 100            // Aumentata da 50 a 100
#define SLOW_SPEED 50             // Aumentata da 20 a 50

// Area di deposito per i pezzi catturati
#define CAPTURED_PIECES_X 200     // Posizione X dell'area pezzi catturati
#define CAPTURED_PIECES_Y 200     // Posizione Y dell'area pezzi catturati
#define CAPTURED_PIECES_Z 0       // Altezza base dell'area pezzi catturati
#define PIECES_SPACING 30         // Spazio tra i pezzi catturati

// Contatori per i pezzi catturati
int capturedWhitePieces = 0;
int capturedBlackPieces = 0;

// Constants for the chess robot
#define CAPTURED_PIECE_AREA_X 200.0  // X coordinate for captured pieces
#define CAPTURED_PIECE_AREA_Y 200.0  // Y coordinate for captured pieces
#define CAPTURED_PIECE_SPACING 30.0   // Spacing between captured pieces

// Game state
bool gameInProgress = false;
int capturedPieceCount = 0;

// Calibration variables
bool isCalibrated = false;
bool isEmergencyStop = false;

// EEPROM addresses
#define EEPROM_CALIBRATED_FLAG 0
#define EEPROM_Z0_ADDR 1
#define EEPROM_MATRIX_ADDR 5

// Function declarations
bool validateCoordinates(float x, float y, float z);
void emergencyStop();
bool initializeLEDs();
void calibrateChessboard();
void calculateMatrixPositions();
int charToIndex(char c);
void processMoveCommand(const String& command);
void processMoveData(const String& moveData);
bool parseMoveData(const String& move, int& fromCol, int& fromRow, int& toCol, int& toRow, bool& isCapture);
bool handleCapture(float toX, float toY, float toZ);
float getSafeTravelHeight(int col);
bool executeMove(float fromX, float fromY, float fromZ, float toX, float toY, float toZ);
void printSystemStatus();
void clearEEPROM();
void testLEDs();
int squareToLedIndex(int row, int col);
void lightSquare(int row, int col, CRGB color);
void clearLEDs();
void lightMove(int fromRow, int fromCol, int toRow, int toCol, bool isCapture);
void highlightPossibleMoves(String squares);

// Function to save calibration data to EEPROM
void saveCalibrationToEEPROM() {
    // Write validation byte
    EEPROM.write(EEPROM_CALIBRATION_VALID, 0xAA);
    
    // Write Z values
    EEPROM.put(EEPROM_Z0, Z0);
    EEPROM.put(EEPROM_Z_GRIPPER_ZERO, Z_gripper_zero);
    
    // Write matrix data
    int addr = EEPROM_MATRIX_START;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            for (int coord = 0; coord < 3; coord++) {
                EEPROM.put(addr, matrix[row][col][coord]);
                addr += sizeof(float);
            }
        }
    }
}

// Function to load calibration data from EEPROM
bool loadCalibrationFromEEPROM() {
    // Check if valid calibration exists
    if (EEPROM.read(EEPROM_CALIBRATION_VALID) != 0xAA) {
        return false;
    }
    
    // Read Z values
    EEPROM.get(EEPROM_Z0, Z0);
    EEPROM.get(EEPROM_Z_GRIPPER_ZERO, Z_gripper_zero);
    
    // Read matrix data
    int addr = EEPROM_MATRIX_START;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            for (int coord = 0; coord < 3; coord++) {
                EEPROM.get(addr, matrix[row][col][coord]);
                addr += sizeof(float);
            }
        }
    }
    return true;
}

void setup() {
    Serial.begin(115200);  // Initialize serial communication for debugging
    Serial1.begin(9600);   // Initialize Serial1 for communication with ESP32
    
    // Initialize FastLED
    if (!initializeLEDs()) {
        Serial.println("ERROR: Failed to initialize LED strip!");
        while(1) { delay(100); } // Halt if LED init fails
    }
    
    // Initialize Dobot and home position
    Dobot_Init();
    
    // Try to load calibration from EEPROM
    if (loadCalibrationFromEEPROM()) {
        isCalibrated = true;
        Serial.println("Calibration loaded from EEPROM");
        Serial1.println("CALIB_MSG:Calibration loaded from EEPROM");
    }
    
    Serial.println("\nMega pronto - Chess Robot System Initialized");
}

bool initializeLEDs() {
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
    
    // Basic validation
    if (NUM_LEDS != 64) return false;
    
    // Test first and last LED
    leds[0] = CRGB::Red;
    leds[NUM_LEDS-1] = CRGB::Red;
    FastLED.show();
    delay(100);
    FastLED.clear();
    FastLED.show();
    
    return true;
}

bool validateCoordinates(float x, float y, float z) {
    if (z < MIN_Z_HEIGHT || z > MAX_Z_HEIGHT) {
        Serial.println("ERROR: Z coordinate out of safe range!");
        return false;
    }
    if (x < MIN_X_COORD || x > MAX_X_COORD) {
        Serial.println("ERROR: X coordinate out of safe range!");
        return false;
    }
    if (y < MIN_Y_COORD || y > MAX_Y_COORD) {
        Serial.println("ERROR: Y coordinate out of safe range!");
        return false;
    }
    return true;
}

void emergencyStop() {
    isEmergencyStop = true;
    Serial.println("EMERGENCY STOP ACTIVATED!");
    Serial1.println("CALIB_MSG:EMERGENCY STOP ACTIVATED!");
    
    // Open gripper to release any held piece
    Dobot_SetEndEffectorGripper(true, false);
    delay(1000);
    Dobot_SetEndEffectorGripper(false, false);
}

void calibrateChessboard() {
    bool confirmed = false;
    Serial.println("\n=== STARTING CALIBRATION ===");
    Serial1.println("CALIB_MSG:Starting chessboard calibration...");
    
    // Home position
    Serial.println("Moving to home position...");
    Dobot_SetHOMECmd();
    delay(2000);

    // Clear all LEDs at start
    clearLEDs();

    // 1. Calibrate Z0 (board surface without gripper)
    Serial.println("\nSTEP 1: Calibrating board surface height (Z0)");
    Serial.println("Place the calibration tool on the board surface. Press ENTER when ready.");
    Serial1.println("CALIB_MSG:STEP 1: Place the calibration tool on the board surface. Press confirm when ready.");
    
    // Light up center squares during Z calibration
    lightSquare(3, 3, CRGB::Blue);
    lightSquare(3, 4, CRGB::Blue);
    lightSquare(4, 3, CRGB::Blue);
    lightSquare(4, 4, CRGB::Blue);
    FastLED.show();
    
    confirmed = false;
    while (!confirmed) {
        // Check Serial Monitor (USB) for Enter key
        if (Serial.available()) {
            String conf = Serial.readStringUntil('\n');
            conf.trim();
            if (conf.length() == 0 || conf.equalsIgnoreCase("OK") || conf.equalsIgnoreCase("CONFIRM")) { 
                confirmed = true; 
                Serial.println("Confirmed via Serial Monitor");
                break; 
            }
        }
        // Check Serial1 (ESP32) for confirmation
        if (Serial1.available()) {
            String conf = Serial1.readStringUntil('\n');
            conf.trim();
            if (conf.equalsIgnoreCase("CALIB_CONFIRM")) { 
                confirmed = true; 
                Serial.println("Confirmed via ESP32");
                break; 
            }
        }
        delay(10);
    }
    Z0 = getCoordinate(Z);
    Serial.print("Z0 (board surface) saved as: "); Serial.println(Z0);
    Serial1.print("CALIB_MSG:Z0 saved as: "); Serial1.println(Z0);

    // 2. Calibrate Z_gripper_zero (with gripper)
    Serial.println("\nSTEP 2: Calibrating gripper height (Z_gripper_zero)");
    Serial.println("Attach gripper and place it on board surface. Press ENTER when ready.");
    Serial1.println("CALIB_MSG:STEP 2: Attach gripper and place it on board surface. Press confirm when ready.");
    
    confirmed = false;
    while (!confirmed) {
        // Check Serial Monitor (USB) for Enter key
        if (Serial.available()) {
            String conf = Serial.readStringUntil('\n');
            conf.trim();
            if (conf.length() == 0 || conf.equalsIgnoreCase("OK") || conf.equalsIgnoreCase("CONFIRM")) { 
                confirmed = true; 
                Serial.println("Confirmed via Serial Monitor");
                break; 
            }
        }
        // Check Serial1 (ESP32) for confirmation
        if (Serial1.available()) {
            String conf = Serial1.readStringUntil('\n');
            conf.trim();
            if (conf.equalsIgnoreCase("CALIB_CONFIRM")) { 
                confirmed = true; 
                Serial.println("Confirmed via ESP32");
                break; 
            }
        }
        delay(10);
    }
    Z_gripper_zero = getCoordinate(Z);
    Serial.print("Z_gripper_zero saved as: "); Serial.println(Z_gripper_zero);
    Serial1.print("CALIB_MSG:Z_gripper_zero saved as: "); Serial1.println(Z_gripper_zero);

    clearLEDs();

    // 3. Calibrate corner positions
    Serial.println("\nSTEP 3: Calibrating corner positions");
    float corners[4][2];  // Store X,Y coordinates of corners
    String cornerNames[4] = {"a8", "h8", "a1", "h1"};
    int cornerRows[4] = {0, 0, 7, 7};
    int cornerCols[4] = {0, 7, 0, 7};
    
    for (int i = 0; i < 4; i++) {
        Serial.print("\nCalibrating corner "); Serial.println(cornerNames[i]);
        Serial.println("Move to " + cornerNames[i] + " corner and press ENTER when ready");
        Serial1.println("CALIB_MSG:Move to " + cornerNames[i] + " corner and press confirm when ready");
        
        // Light up current corner being calibrated
        clearLEDs();
        lightSquare(cornerRows[i], cornerCols[i], CRGB::Green);
        FastLED.show();
        
        confirmed = false;
        while (!confirmed) {
            // Check Serial Monitor (USB) for Enter key
            if (Serial.available()) {
                String conf = Serial.readStringUntil('\n');
                conf.trim();
                if (conf.length() == 0 || conf.equalsIgnoreCase("OK") || conf.equalsIgnoreCase("CONFIRM")) { 
                    confirmed = true; 
                    Serial.println("Confirmed via Serial Monitor");
                    break; 
                }
            }
            // Check Serial1 (ESP32) for confirmation
            if (Serial1.available()) {
                String conf = Serial1.readStringUntil('\n');
                conf.trim();
                if (conf.equalsIgnoreCase("CALIB_CONFIRM")) { 
                    confirmed = true; 
                    Serial.println("Confirmed via ESP32");
                    break; 
                }
            }
            delay(10);
        }
        corners[i][0] = getCoordinate(X);
        corners[i][1] = getCoordinate(Y);
        Serial.print(cornerNames[i] + " position: X=");
        Serial.print(corners[i][0]); Serial.print(" Y=");
        Serial.println(corners[i][1]);
        Serial1.print("CALIB_MSG:" + cornerNames[i] + " saved as X=");
        Serial1.print(corners[i][0]); Serial1.print(" Y=");
        Serial1.println(corners[i][1]);
    }

    // Calculate all square positions
    Serial.println("\nCalculating square positions...");
    float xStep = (corners[1][0] - corners[0][0]) / 7.0;
    float yStep = (corners[2][1] - corners[0][1]) / 7.0;
    
    // Show calibration progress pattern
    clearLEDs();
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            matrix[row][col][0] = corners[0][0] + col * xStep;  // X coordinate
            matrix[row][col][1] = corners[0][1] + row * yStep;  // Y coordinate
            matrix[row][col][2] = Z_gripper_zero;  // Z coordinate
            
            // Show progress with LEDs
            lightSquare(row, col, CRGB::Green);
            FastLED.show();
            delay(50);
        }
    }
    
    // Save calibration to EEPROM
    Serial.println("\nSaving calibration to EEPROM...");
    saveCalibrationToEEPROM();
    
    // Verify EEPROM save
    Serial.println("Verifying EEPROM data...");
    float testZ0, testZ_gripper;
    EEPROM.get(EEPROM_Z0, testZ0);
    EEPROM.get(EEPROM_Z_GRIPPER_ZERO, testZ_gripper);
    
    if (testZ0 != Z0 || testZ_gripper != Z_gripper_zero) {
        Serial.println("ERROR: EEPROM verification failed!");
        Serial1.println("CALIB_MSG:ERROR: EEPROM verification failed!");
        isCalibrated = false;
        return;
    }
    
    // Final verification
    Serial.println("\nCalibration values:");
    Serial.print("Z0 (board surface): "); Serial.println(Z0);
    Serial.print("Z_gripper_zero: "); Serial.println(Z_gripper_zero);
    Serial.print("Gripper offset: "); Serial.println(Z_gripper_zero - Z0);
    
    delay(1000);
    clearLEDs();
    
    isCalibrated = true;
    Serial.println("\n=== CALIBRATION COMPLETE ===");
    Serial1.println("CALIB_MSG:Calibration complete and verified!");
}

// Calculate the matrix positions based on the corner positions
void calculateMatrixPositions() {
    // Bilinear interpolation from the four corners
    float x_a1 = matrix[7][0][0]; float y_a1 = matrix[7][0][1];
    float x_h1 = matrix[7][7][0]; float y_h1 = matrix[7][7][1];
    float x_a8 = matrix[0][0][0]; float y_a8 = matrix[0][0][1];
    float x_h8 = matrix[0][7][0]; float y_h8 = matrix[0][7][1];

    // Validate corner positions
    if (!validateCoordinates(x_a1, y_a1, Z0) ||
        !validateCoordinates(x_h1, y_h1, Z0) ||
        !validateCoordinates(x_a8, y_a8, Z0) ||
        !validateCoordinates(x_h8, y_h8, Z0)) {
        Serial.println("ERROR: Invalid corner coordinates!");
        Serial1.println("CALIB_MSG:ERROR: Invalid corner coordinates!");
        isCalibrated = false;
        return;
    }

    for (int row = 0; row < 8; row++) {
        float tRow = row / 7.0;
        for (int col = 0; col < 8; col++) {
            float tCol = col / 7.0;

            // Corrected bilinear interpolation
            // X coordinate
            matrix[row][col][0] =
                (1 - tRow) * (1 - tCol) * x_a1 +
                (1 - tRow) * tCol       * x_h1 +
                tRow       * (1 - tCol) * x_a8 +
                tRow       * tCol       * x_h8;

            // Y coordinate (corrected order)
            matrix[row][col][1] =
                (1 - tRow) * (1 - tCol) * y_a1 +
                (1 - tRow) * tCol       * y_h1 +
                tRow       * (1 - tCol) * y_a8 +
                tRow       * tCol       * y_h8;

            // Z coordinate with validation
            matrix[row][col][2] = Z_gripper_zero;
            
            // Validate calculated position
            if (!validateCoordinates(matrix[row][col][0], 
                                   matrix[row][col][1], 
                                   matrix[row][col][2])) {
                Serial.println("ERROR: Invalid calculated position!");
                Serial1.println("CALIB_MSG:ERROR: Invalid calculated position!");
                isCalibrated = false;
                return;
            }
        }
    }

    // Print calculated positions for verification
    Serial.println("\n--- CALCULATED MATRIX ---");
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Serial.print((char)('a' + col));
            Serial.print(8 - row);
            Serial.print(": X=");
            Serial.print(matrix[row][col][0], 2);
            Serial.print(" Y=");
            Serial.print(matrix[row][col][1], 2);
            Serial.print(" Z=");
            Serial.println(matrix[row][col][2], 2);
        }
    }
    Serial.println("-------------------------");
    
    isCalibrated = true;
}

int charToIndex(char c) {
    return c - 'a';
}

void loop() {
    // Check for emergency stop condition
    if (isEmergencyStop) {
        return;
    }

    // Check for commands from Serial Monitor (USB)
    if (Serial.available()) {
        String data = Serial.readStringUntil('\n');
        data.trim();
        Serial.println("Serial Monitor command: " + data);
        
        // Process serial monitor commands
        if (data.equalsIgnoreCase("CALIBRATE")) {
            calibrateChessboard();
        } else if (data.equalsIgnoreCase("STATUS")) {
            printSystemStatus();
        } else if (data.equalsIgnoreCase("CLEAR_EEPROM")) {
            clearEEPROM();
        } else if (data.equalsIgnoreCase("TEST_LEDS")) {
            testLEDs();
        } else {
            Serial.println("Unknown command. Available commands:");
            Serial.println("CALIBRATE - Start calibration");
            Serial.println("STATUS - Show system status");
            Serial.println("CLEAR_EEPROM - Clear calibration data");
            Serial.println("TEST_LEDS - Test LED strip");
        }
    }

    // Check if data is available on Serial1 (ESP32 communication)
    if (Serial1.available()) {
        String data = Serial1.readStringUntil('\n');
        data.trim();
        Serial.println("Received from ESP32: " + data);
        
        // Process commands from ESP32
        if (data.equalsIgnoreCase("STARTGAME")) {
            gameInProgress = true;
            capturedPieceCount = 0;
            capturedWhitePieces = 0;
            capturedBlackPieces = 0;
            Serial.println("Game started");
        } else if (data.equalsIgnoreCase("ENDGAME")) {
            gameInProgress = false;
            Serial.println("Game ended");
        } else if (data.startsWith("#") && data.endsWith(";")) {
            // Handle move commands in format: #utente:b2b3;
            processMoveCommand(data);
        } else if (gameInProgress) {
            if (!isCalibrated) {
                Serial.println("ERROR: Cannot make moves without calibration!");
                return;
            }
            processMoveData(data);
        } else {
            Serial.println("Game not in progress. Start game first.");
        }
    }
}

// Function to process move commands in format: #utente:b2b3;
void processMoveCommand(const String& command) {
    Serial.println("\n=== PROCESSING MOVE COMMAND ===");
    Serial.print("Raw command: ");
    Serial.println(command);
    
    // Check if system is calibrated
    if (!isCalibrated) {
        Serial.println("ERROR: Cannot make moves without calibration!");
        Serial1.println("ERROR: System not calibrated!");
        return;
    }
    
    // Extract move from command format: #utente:b2b3;
    int colonIndex = command.indexOf(':');
    int semicolonIndex = command.indexOf(';');
    
    if (colonIndex == -1 || semicolonIndex == -1 || semicolonIndex <= colonIndex) {
        Serial.println("ERROR: Invalid command format!");
        Serial1.println("ERROR: Invalid command format!");
        return;
    }
    
    String move = command.substring(colonIndex + 1, semicolonIndex);
    move.trim();
    
    Serial.print("Extracted move: ");
    Serial.println(move);
    
    // Process the move
    processMoveData(move);
}

// Function to process move data
void processMoveData(const String& moveData) {
    if (isEmergencyStop) {
        Serial.println("ERROR: Emergency stop is active!");
        return;
    }

    String move = moveData;
    move.trim();
    Serial.println("\n=== PROCESSING MOVE ===");
    Serial.print("Raw move data: '");
    Serial.print(move);
    Serial.println("'");

    int fromCol, fromRow, toCol, toRow;
    bool isCapture = false;

    // Parse and validate move format
    if (!parseMoveData(move, fromCol, fromRow, toCol, toRow, isCapture)) {
        Serial.println("ERROR: Invalid move format!");
        return;
    }

    Serial.println("Move parsed successfully:");
    Serial.print("From: row="); Serial.print(fromRow); Serial.print(" col="); Serial.println(fromCol);
    Serial.print("To: row="); Serial.print(toRow); Serial.print(" col="); Serial.println(toCol);
    Serial.print("Is capture: "); Serial.println(isCapture ? "yes" : "no");

    // Get coordinates
    float fromX = matrix[fromRow][fromCol][0];
    float fromY = matrix[fromRow][fromCol][1];
    float fromZ = matrix[fromRow][fromCol][2];
    float toX = matrix[toRow][toCol][0];
    float toY = matrix[toRow][toCol][1];
    float toZ = matrix[toRow][toCol][2];

    Serial.println("Coordinates calculated:");
    Serial.print("From: X="); Serial.print(fromX); Serial.print(" Y="); Serial.print(fromY); Serial.print(" Z="); Serial.println(fromZ);
    Serial.print("To: X="); Serial.print(toX); Serial.print(" Y="); Serial.print(toY); Serial.print(" Z="); Serial.println(toZ);

    // Validate all coordinates
    if (!validateCoordinates(fromX, fromY, fromZ) ||
        !validateCoordinates(toX, toY, toZ)) {
        Serial.println("ERROR: Invalid move coordinates!");
        return;
    }

    // If it's a capture, first remove the captured piece
    if (isCapture) {
        if (!handleCapture(toX, toY, toZ)) {
            Serial.println("ERROR: Failed to handle capture!");
            return;
        }
    }

    // Check if calibration is valid before executing move
    if (!isCalibrated) {
        Serial.println("ERROR: System not calibrated!");
        return;
    }

    // Execute the move
    Serial.println("Executing move...");
    if (!executeMove(fromX, fromY, fromZ, toX, toY, toZ)) {
        Serial.println("ERROR: Move execution failed!");
        return;
    }

    Serial.println("=== MOVE COMPLETE ===\n");
}

bool parseMoveData(const String& move, int& fromCol, int& fromRow, int& toCol, int& toRow, bool& isCapture) {
    // Check if it's a capture move (format: e4xd5)
    if (move.indexOf('x') != -1) {
        if (move.length() != 5) return false;
        isCapture = true;
        fromCol = move.charAt(0) - 'a';
        fromRow = 8 - (move.charAt(1) - '0');
        toCol = move.charAt(3) - 'a';
        toRow = 8 - (move.charAt(4) - '0');
    } else if (move.length() == 4) {
        isCapture = false;
        fromCol = move.charAt(0) - 'a';
        fromRow = 8 - (move.charAt(1) - '0');
        toCol = move.charAt(2) - 'a';
        toRow = 8 - (move.charAt(3) - '0');
    } else {
        return false;
    }

    // Validate indices
    if (fromCol < 0 || fromCol > 7 || toCol < 0 || toCol > 7 ||
        fromRow < 0 || fromRow > 7 || toRow < 0 || toRow > 7) {
        return false;
    }

    return true;
}

bool handleCapture(float toX, float toY, float toZ) {
    Serial.println("\n=== CAPTURING PIECE ===");
    
    // Check if we have space for more captured pieces
    bool isWhitePiece = toY < (MIN_Y_COORD + MAX_Y_COORD) / 2;
    if ((isWhitePiece && capturedWhitePieces >= MAX_CAPTURED_PIECES) ||
        (!isWhitePiece && capturedBlackPieces >= MAX_CAPTURED_PIECES)) {
        Serial.println("ERROR: No more space for captured pieces!");
        return false;
    }

    float depositX, depositY;
    
    // Calculate deposit position
    if (!isWhitePiece) {
        depositX = CAPTURED_PIECES_X;
        depositY = CAPTURED_PIECES_Y + (PIECES_SPACING * capturedBlackPieces);
    } else {
        depositX = CAPTURED_PIECES_X;
        depositY = CAPTURED_PIECES_Y - (PIECES_SPACING * capturedWhitePieces);
    }

    // Validate deposit coordinates
    if (!validateCoordinates(depositX, depositY, CAPTURED_PIECES_Z)) {
        Serial.println("ERROR: Invalid deposit coordinates!");
        return false;
    }

    // Execute capture movement sequence
    if (!executeMove(toX, toY, toZ, depositX, depositY, CAPTURED_PIECES_Z)) {
        return false;
    }

    // Update counters only after successful capture
    if (isWhitePiece) {
        capturedWhitePieces++;
    } else {
        capturedBlackPieces++;
    }

    return true;
}

// Funzione per determinare l'altezza di viaggio sicura in base alla colonna
float getSafeTravelHeight(int col) {
    // Se la colonna è 'a' (0) o 'h' (7), usa l'altezza ridotta
    if (col == 0 || col == 7) {
        return SAFE_TRAVEL_HEIGHT_EDGE;
    }
    return SAFE_TRAVEL_HEIGHT;
}

bool executeMove(float fromX, float fromY, float fromZ, float toX, float toY, float toZ) {
    Serial.println("\n=== EXECUTING MOVE ===");
    
    // Determina le colonne di partenza e arrivo (0-7, dove 0='a' e 7='h')
    int fromCol = round((fromX - matrix[0][0][0]) / ((matrix[0][7][0] - matrix[0][0][0]) / 7.0));
    int toCol = round((toX - matrix[0][0][0]) / ((matrix[0][7][0] - matrix[0][0][0]) / 7.0));
    
    // Determina l'altezza di viaggio in base alle colonne
    float travelHeight = min(getSafeTravelHeight(fromCol), getSafeTravelHeight(toCol));
    
    Serial.print("Move from column: ");
    Serial.print((char)('a' + fromCol));
    Serial.print(" to column: ");
    Serial.print((char)('a' + toCol));
    Serial.print(" - Using travel height: ");
    Serial.println(travelHeight);
    
    // Array of movement steps with dynamic height
    struct {
        const char* description;
        float x, y, z;
        float speed;
    } steps[] = {
        {"Moving to safe height above source", fromX, fromY, travelHeight - 15.0, FAST_SPEED},
        {"Moving down to pickup position", fromX, fromY, fromZ + PIECE_GRAB_OFFSET, SLOW_SPEED},
        {"Moving to safe height with piece", fromX, fromY, travelHeight - 15.0, FAST_SPEED},
        {"Moving above destination", toX, toY, travelHeight - 15.0, FAST_SPEED},
        {"Moving down to place position", toX, toY, toZ + PIECE_GRAB_OFFSET, SLOW_SPEED},
        {"Moving to final safe height", toX, toY, travelHeight - 15.0, FAST_SPEED}
    };

    // Execute each movement step
    for (int i = 0; i < 6; i++) {
        Serial.print(i + 1);
        Serial.print(". ");
        Serial.print(steps[i].description);
        Serial.print(" (Z=");
        Serial.print(steps[i].z);
        Serial.println(")");

        if (!validateCoordinates(steps[i].x, steps[i].y, steps[i].z)) {
            Serial.println("ERROR: Invalid coordinates in movement sequence!");
            return false;
        }

        // Execute movement with speed control
        Dobot_SetPTPCmd(MOVJ_XYZ, steps[i].x, steps[i].y, steps[i].z, steps[i].speed);
        
        // Minimal delay for movement completion
        if (i == 1 || i == 4) { // Solo per pickup e place
            delay(500);
        }

        // Handle gripper operations
        if (i == 1) {
            Dobot_SetEndEffectorGripper(true, true);  // Close gripper
            delay(300);
        } else if (i == 4) {
            Dobot_SetEndEffectorGripper(true, false);  // Open gripper
            delay(300);
            Dobot_SetEndEffectorGripper(false, false); // Deactivate gripper
        }
    }

    return true;
}

// Serial Monitor Control Functions
void printSystemStatus() {
    Serial.println("\n=== SYSTEM STATUS ===");
    Serial.print("Calibrated: "); Serial.println(isCalibrated ? "YES" : "NO");
    Serial.print("Game in progress: "); Serial.println(gameInProgress ? "YES" : "NO");
    Serial.print("Emergency stop: "); Serial.println(isEmergencyStop ? "ACTIVE" : "INACTIVE");
    Serial.print("Captured white pieces: "); Serial.println(capturedWhitePieces);
    Serial.print("Captured black pieces: "); Serial.println(capturedBlackPieces);
    
    if (isCalibrated) {
        Serial.print("Z0 (board surface): "); Serial.println(Z0);
        Serial.print("Z_gripper_zero: "); Serial.println(Z_gripper_zero);
        Serial.print("Gripper offset: "); Serial.println(Z_gripper_zero - Z0);
    }
    Serial.println("===================\n");
}

void clearEEPROM() {
    Serial.println("Clearing EEPROM calibration data...");
    EEPROM.write(EEPROM_CALIBRATION_VALID, 0x00);
    isCalibrated = false;
    Serial.println("EEPROM cleared. System needs recalibration.");
}

void testLEDs() {
    Serial.println("Testing LED strip...");
    
    // Test all LEDs with different colors
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Red;
    }
    FastLED.show();
    delay(500);
    
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Green;
    }
    FastLED.show();
    delay(500);
    
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Blue;
    }
    FastLED.show();
    delay(500);
    
    // Clear LEDs
    FastLED.clear();
    FastLED.show();
    Serial.println("LED test complete.");
}

// Convert chess coordinates to LED index (0-63)
int squareToLedIndex(int row, int col) {
    if (row < 0 || row >= 8 || col < 0 || col >= 8) {
        Serial.println("ERROR: Invalid chess coordinates!");
        return -1;
    }
    
    // LED strip starts from a8 (top left) and goes right
    // Chess coordinates: row 0 = rank 8, row 7 = rank 1
    // We need to invert the row to match LED strip orientation
    row = 7 - row;  // Invert row to match chess coordinates
    
    if (row % 2 == 0) {
        // Even rows (0,2,4,6) go left to right
        return (row * 8) + col;
    } else {
        // Odd rows (1,3,5,7) go right to left
        return (row * 8) + (7 - col);
    }
}

// Light up a specific square with error checking
void lightSquare(int row, int col, CRGB color) {
    int ledIndex = squareToLedIndex(row, col);
    if (ledIndex < 0 || ledIndex >= NUM_LEDS) {
        Serial.println("ERROR: Invalid LED index!");
        return;
    }
    leds[ledIndex] = color;
    FastLED.show();
}

// Clear all LEDs safely
void clearLEDs() {
    FastLED.clear();
    FastLED.show();
    delay(50); // Small delay to ensure update is complete
}

// Light up a move path with validation
void lightMove(int fromRow, int fromCol, int toRow, int toCol, bool isCapture) {
    // Validate all coordinates
    if (fromRow < 0 || fromRow >= 8 || fromCol < 0 || fromCol >= 8 ||
        toRow < 0 || toRow >= 8 || toCol < 0 || toCol >= 8) {
        Serial.println("ERROR: Invalid move coordinates for LED display!");
        return;
    }
    
    clearLEDs();
    
    // Source square in blue
    lightSquare(fromRow, fromCol, CRGB::Blue);
    
    // Destination square in green for normal moves, red for captures
    lightSquare(toRow, toCol, isCapture ? CRGB::Red : CRGB::Green);
}

// Function to highlight possible moves with validation
void highlightPossibleMoves(String squares) {
    Serial.println("\n=== HIGHLIGHTING MOVES ===");
    Serial.print("Input squares string: '");
    Serial.print(squares);
    Serial.println("'");
    
    clearLEDs();
    
    // Se la stringa contiene solo due quadrati, illumina il percorso tra loro
    if (squares.indexOf(',') == -1 && squares.length() >= 4) {
        char startFile = squares.charAt(0);
        char startRank = squares.charAt(1);
        char endFile = squares.charAt(2);
        char endRank = squares.charAt(3);
        
        if (startFile >= 'a' && startFile <= 'h' && 
            endFile >= 'a' && endFile <= 'h' &&
            startRank >= '1' && startRank <= '8' &&
            endRank >= '1' && endRank <= '8') {
            
            int startCol = startFile - 'a';
            int startRow = 8 - (startRank - '0');
            int endCol = endFile - 'a';
            int endRow = 8 - (endRank - '0');
            
            // Illumina tutti i quadrati nel percorso
            int minRow = min(startRow, endRow);
            int maxRow = max(startRow, endRow);
            int minCol = min(startCol, endCol);
            int maxCol = max(startCol, endCol);
            
            for (int row = minRow; row <= maxRow; row++) {
                for (int col = minCol; col <= maxCol; col++) {
                    if (row == startRow && col == startCol) {
                        lightSquare(row, col, CRGB::Blue);  // Quadrato di partenza in blu
                    } else if (row == endRow && col == endCol) {
                        lightSquare(row, col, CRGB::Green); // Quadrato di arrivo in verde
                    } else {
                        lightSquare(row, col, CRGB::Yellow); // Quadrati intermedi in giallo
                    }
                }
            }
            FastLED.show();
            return;
        }
    }
    
    // Altrimenti, procedi con il comportamento normale per lista di quadrati
    String tempSquares = squares;
    tempSquares.trim();
    
    while (tempSquares.length() > 0) {
        String square;
        int commaIndex = tempSquares.indexOf(',');
        
        if (commaIndex == -1) {
            square = tempSquares;
            tempSquares = "";
        } else {
            square = tempSquares.substring(0, commaIndex);
            tempSquares = tempSquares.substring(commaIndex + 1);
        }
        
        square.trim();
        if (square.length() == 2) {
            char file = square.charAt(0);
            char rank = square.charAt(1);
            
            if (file >= 'a' && file <= 'h' && rank >= '1' && rank <= '8') {
                int col = file - 'a';
                int row = 8 - (rank - '0');
                lightSquare(row, col, CRGB::Yellow);
            }
        }
    }
    
    FastLED.show();
}
