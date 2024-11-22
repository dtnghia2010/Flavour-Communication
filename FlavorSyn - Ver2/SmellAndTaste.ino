#define PUMP_COUNT 6
#define numRelays 6
#define maxTimePumpWater 10000
//<index> <direction> <time> <speed>
// 1 1 10000 200

//<index> <state> <time>
// 1 1 10

/*
TASTE
very sweet: 1
medium sweet: 2
low sweet:3
//<index> <direction> <time> <speed>
// 1 1 10000 200

SMELL
lemon: 1
strawberry: 2
matcha latte: 3
chanh xa: 5
vanila: 6
//<index> <state> <time>
// 1 1 10
*/

// thoi gian goi ham restore water
long durationRestoreWaterCall[PUMP_COUNT] = {0};
// thoi gian ham restore water chay trong bao lau
long durationRestoreWaterRun[PUMP_COUNT] = {0};
// speed of durationRestoreWater
int speedPump[PUMP_COUNT] = {0};

// Cleaning
// DIRECTION OF PUMP
const int FORWARD = 1;
const int REVERSE = 2;
const int CLEAN = 3;
const int HALT = 0;

// PIN OF PUMP
const int PUMP_IN1[] = {22, 24, 26, 28, 30, 32}; // Green 
const int PUMP_IN2[] = {23, 25, 27, 29, 31, 33}; // Yellow
const int PUMP_PWM[] = {2, 3, 4, 5, 6, 7}; // White

// PIN OF RELAYS
const uint8_t relayPins[] = {8, 9, 10, 11, 12, 13};

// DURATION OF BOTH
unsigned long durations_Pump[PUMP_COUNT] = {0}; // Initialize durations array
unsigned long durations_Relays[numRelays] = {0};

// VARIABLE TO REDUCE TIME
unsigned long currentTime = 0;
unsigned long lastTime = 0;
unsigned long deltaTime = 0;

// SETUP ALL PIN
void setup() {
    Serial.begin(115200);
    setupTimekeeping();
    for (int i = 0; i < PUMP_COUNT; i++) {
        pinMode(PUMP_IN1[i], OUTPUT);
        pinMode(PUMP_IN2[i], OUTPUT);
        pinMode(PUMP_PWM[i], OUTPUT);
    }
    for (int i = 0; i < numRelays; i++) {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], LOW);
    } 
}

void loop() {
    currentTime = millis();
    deltaTime = currentTime - lastTime;

    // Check Duration of Pump and Relays
    reduceDurations(deltaTime);

    // Check input Pump and Relays
    handleInput();

    lastTime = currentTime;
}

void setupTimekeeping() {
    currentTime = millis();
    lastTime = currentTime;
}

// Extract the duration from the string
long extractDuration(const char* str) {
    long duration = 0;
    while (isDigit(*str)) {
        duration = duration * 10 + (*str - '0');
        str++;
    }
    // Serial.println(duration);

    return duration;
}

// Extract the speed from the string
int extractSpeed(const char* str) {
    int speed = atoi(str);
    return speed;
}

void reduceDurations(unsigned long deltaTime) {
    static unsigned long lastUpdateTime = 0;
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - lastUpdateTime;

    if (elapsedTime >= 1000) { // Update every 1 second
        lastUpdateTime = currentTime;

        for (int i = 0; i < PUMP_COUNT; i++) {
            if (durations_Pump[i] > 0) {
                if (durations_Pump[i] <= 1000) {
                    durations_Pump[i] = 0;
                    turn(i, HALT, 0); 
                    Serial.print("Pump ");
                    Serial.print(i);
                    Serial.println(" stopped.");
                } else {
                    durations_Pump[i] -= 1000; // Reduce by 1 second
                }
            }
            Serial.print("Pump ");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(durations_Pump[i] / 1000.0, 1); // Display seconds
        }

        for (int i = 0; i < numRelays; i++) {
            if (durations_Relays[i] > 0) {
                if (durations_Relays[i] <= 1000) {
                    durations_Relays[i] = 0;
                    digitalWrite(relayPins[i], LOW);
                    Serial.print("Relay ");
                    Serial.print(i);
                    Serial.println(" turned off.");
                } else {
                    durations_Relays[i] -= 1000; // Reduce by 1 second
                }
            }
            Serial.print("Relay ");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(durations_Relays[i] / 1000.0, 1); // Display seconds
        }
    }
}




void turn(int pump, int dir, int speed) {
    // Serial.println("Setting speed!");
    turnDirection(pump, dir);

    // Set the speed using PWM (analogWrite)
    if (pump >= 0 && pump < PUMP_COUNT) { // Ensure pump is within bounds
        int pin = PUMP_PWM[pump];
        analogWrite(pin, speed);
        // Serial.print("Pump ");
        // Serial.print(pump+1);
        // Serial.print(" set to ");
        // Serial.println(speed);
    } else {
        Serial.println("Invalid pump index!");
    }
}

// Case for pump direction and stop
void turnDirection(int pump, int dir) {
    int out1;
    int out2;
    switch (dir) {
        case HALT:
            out1 = LOW;
            out2 = LOW;
            Serial.println("Halt selected");
            break;
        case FORWARD:
            out1 = HIGH;
            out2 = LOW;
            Serial.println("Forward selected");
            break;
        case REVERSE:
            out1 = LOW;
            out2 = HIGH;
            Serial.println("Reverse selected");
            break;
        default:
            return;
    }

    if (pump >= 0 && pump < PUMP_COUNT) { // Ensure pump is within bounds
        digitalWrite(PUMP_IN1[pump], out1);
        digitalWrite(PUMP_IN2[pump], out2);
    } 
}

void handleInput() {
    char *array[4]; // Assuming there are at most 4 tokens
    int count = 0; // Counter for the number of tokens

    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        char commandBuffer[command.length() + 1]; // Create a char array buffer for strtok
        command.toCharArray(commandBuffer, sizeof(commandBuffer)); // Convert String to char array
        
        Serial.println("Received command: " + command); // Print the received command

        char *token;
        const char *delimiter = " ";
        token = strtok(commandBuffer, delimiter);

        while (token != NULL && count < 4) { // Ensure array index doesn't exceed 4
            array[count++] = token; // Store token in the array and increment count
            token = strtok(NULL, delimiter);
        }
        if (count == 4) {
            // Assign which part of the input command to be read
            String pump = array[0];
            String dirString = array[1];
            String durationString = array[2];
            String speedString = array[3];

            // Convert String to char array
            int pumpIndex = pump.charAt(0) - '1';
            // char dirChar = dir.charAt(0);

            // Convert duration and speed from String to int
            int dir = extractDuration(dirString.c_str());
            long duration = extractDuration(durationString.c_str());
            int speed = extractSpeed(speedString.c_str());
            duration += 1000;

            speedPump[pumpIndex] = speed;
            if (duration < maxTimePumpWater){
              durationRestoreWaterRun[pumpIndex] = duration*2 + 10000;
            }
            else {
              durationRestoreWaterRun[pumpIndex] = duration + 10000 + maxTimePumpWater;
            }

            // Start or update duration only if it's greater than 0
            if (speed >= 0 && speed <= 255) {
                turn(pumpIndex, dir, speed);
                if (duration > 0) {
                    durations_Pump[pumpIndex] = duration; 
                    
                    durationRestoreWaterCall[pumpIndex] = duration + 10000;
                    
                } else {
                    Serial.println("Error: Speed out of range (0 - 255)");
                }
            }
        } else if (count == 3) {
          int relayIndex = atoi(array[0]) - 1; // Convert index
          char relayState = array[1][0];      // Get state ('1' or '0')
          long duration = extractDuration(array[2]); // Extract duration
          duration += 1000;
          Serial.print("Relay Command: Index = ");
          Serial.print(relayIndex);
          Serial.print(", State = ");
          Serial.print(relayState);
          Serial.print(", Duration = ");
          Serial.println(duration);

          if (relayIndex >= 0 && relayIndex < numRelays) {
              if (relayState == '1') {
                  digitalWrite(relayPins[relayIndex], HIGH);
                  durations_Relays[relayIndex] = duration;
                  Serial.print("Set relay ");
                  Serial.print(relayIndex + 1);
                  Serial.print(" ON with duration: ");
                  Serial.println(durations_Relays[relayIndex]);
              } else if (relayState == '0') {
                  digitalWrite(relayPins[relayIndex], LOW);
                  durations_Relays[relayIndex] = 0;
                  Serial.print("Set relay ");
                  Serial.print(relayIndex + 1);
                  Serial.println(" OFF.");
              }
          } else {
              Serial.println("Invalid relay index.");
          }
        } else if (count == 1) {
            String command = array[0];
            if (command == "CLEAN") {
                cleanPumps();
            }
        }
    }
}

// Function to clean all water pumps by running them for 30 seconds
void cleanPumps() {
    const int cleanDuration = 999999999; // 30 seconds in milliseconds
    const int cleanSpeed = 255; // Full speed

    Serial.println("Starting water pump cleaning process...");

    // Set all pumps to run forward at full speed
    for (int i = 0; i < PUMP_COUNT; i++) {
        turn(i, FORWARD, cleanSpeed);
    }

    // Wait for 30 seconds
    delay(cleanDuration);

    // Stop all pumps
    for (int i = 0; i < PUMP_COUNT; i++) {
        turn(i, HALT, 0);
    }

    Serial.println("Water pump cleaning process completed.");
}

// Function to clean all water pumps by running them for 30 seconds
void RestoreWater(int index) {
    const int cleanSpeed = speedPump[index]; // Full speed
    speedPump[index] = 0;

    // Serial.println("Starting restore pump water");

    turn(index, 1, cleanSpeed);
    
    // Serial.println("Restore water pump process completed.");
}
