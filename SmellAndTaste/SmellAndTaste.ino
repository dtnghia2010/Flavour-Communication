#define PUMP_COUNT 6
#define numRelays 6

//<index> <direction> <time> <speed>
// 1 F 10000 200

//<index> <state> <time>
// 1 1 1000

// DIRECTION OF PUMP
const int FORWARD = 1;
const int REVERSE = 2;
const int HALT = 0;

// PIN OF PUMP
const int PUMP_IN1[] = {22, 24, 26, 28, 30, 32}; // Green 
const int PUMP_IN2[] = {23, 25, 27, 2 9, 31, 33}; // Yellow
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

    // check Duration of Pump and Relays
    reduceDurations(deltaTime);

    // check input Pump and Relays
    handleInput();

    lastTime = currentTime;
}
void setupTimekeeping() {
    currentTime = millis();
    lastTime = currentTime;
}

// Extract the duration from the string
int extractDuration(const char* str) {
    int duration = 0;
    for (int i = 0; i < 5 && isDigit(str[i]); i++) {
        duration = duration * 10 + (str[i] - '0');
    }
    return duration;
}

// Extract the speed from the string
int extractSpeed(const char* str) {
    int speed = atoi(str);
    return speed;
}

// Stop the pump when duration ends
void reduceDurations(unsigned long deltaTime) {
    for (int i = 0; i < PUMP_COUNT; i++) {
        if (durations_Pump[i] != 0) {
            durations_Pump[i] = max(0, durations_Pump[i] - deltaTime);
            if (durations_Pump[i] == 0) {
                turn(i, HALT, 0); 
            }
        }
    }
    for (int i = 0; i < numRelays; i++) {
        if (durations_Relays[i] != 0) {
            durations_Relays[i] = max(0, durations_Relays[i] - deltaTime);
            if (durations_Relays[i] == 0) {
                digitalWrite(relayPins[i], LOW);
            }
        }
    }
}

void turn(int pump, int dir, int speed) {
    Serial.println("Setting speed!");
    turnDirection(pump, dir);

    // Set the speed using PWM (analogWrite)
    if (pump >= 0 && pump < PUMP_COUNT) { // Ensure pump is within bounds
        int pin = PUMP_PWM[pump];
        analogWrite(pin, speed);
        Serial.print("Pump ");
        Serial.print(pump);
        Serial.print(" set to ");
        Serial.println(speed);
    } else {
        Serial.println("Invalid pump index!");
    }
}

//Case for pump direction and stop
void turnDirection(int pump, int dir) {
    int out1;
    int out2;
    switch (dir) {
        case HALT:
            out1 = LOW;
            out2 = LOW;
            break;
        case FORWARD:
            out1 = HIGH;
            out2 = LOW;
            break;
        case REVERSE:
            out1 = LOW;
            out2 = HIGH;
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

        char *token;
        const char *delimiter = " ";
        token = strtok(commandBuffer, delimiter);

        while (token != NULL && count < 4) { // Ensure array index doesn't exceed 4
            array[count++] = token; // Store token in the array and increment count
            token = strtok(NULL, delimiter);
        }
       if (count == 4){
          //Asign which part of the input command to be read
          String pump = array[0];
          String dirString = array[1];
          String durationString = array[2];
          String speedString = array[3];

          // Convert String to char array
          int pumpIndex = pump.charAt(0) -'1';
          // char dirChar = dir.charAt(0);

          // Convert duration and speed from String to int
          int dir = extractDuration(dirString.c_str()); 
          int duration = extractDuration(durationString.c_str());
          int speed = extractSpeed(speedString.c_str());

          // Start or update duration only if it's greater than 0
          if (speed >= 0 && speed <= 255) {
              turn(pumpIndex, dir, speed);                
              if (duration > 0) {
                durations_Pump[pumpIndex] = duration;
              } else {
                  Serial.println("Error: Speed out of range (0 - 255)");
              }
          }
          //Print input double check
          //LCD Crystal add later
          Serial.println(pump);
          Serial.println(dir);
          Serial.println(duration);
          Serial.println(speed);
      }
      else if (count == 3) {
            int relayIndex = atoi(array[0]) - 1;
            char relayState = array[1][0];
            String durationString = array[2];
            // convert char to string to extract duration
            int duration = extractDuration(durationString.c_str());


            if (relayIndex >= 0 && relayIndex < numRelays) {
                if (relayState == '1') {
                    digitalWrite(relayPins[relayIndex], HIGH);
                    if (duration > 0) {
                      durations_Relays[relayIndex] = duration;
                    } 
                } else if (relayState == '0') {
                    digitalWrite(relayPins[relayIndex], LOW);
                }
            }
        }
    }
}

