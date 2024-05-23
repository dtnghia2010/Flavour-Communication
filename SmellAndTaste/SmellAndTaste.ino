#define PUMP_COUNT 6

const char PUMP[PUMP_COUNT] = {'1', '2', '3', '4', '5', '6'};
const int PUMP_IN1[] = {22, 24, 26, 28, 30, 32}; // Green 
const int PUMP_IN2[] = {23, 25, 27, 29, 31, 33}; // Yellow
const int PUMP_PWM[] = {2, 3, 4, 5, 6, 7}; // White

const uint8_t relayPins[] = {8, 9, 10, 11, 12, 13};
const int numRelays = sizeof(relayPins) / sizeof(relayPins[0]);

struct Relay {
    uint8_t pin;
    bool state;
    unsigned long endTime;
};
Relay relays[numRelays];

unsigned long currentTime = 0;
unsigned long lastTime = 0;

const int FORWARD = 'F';
const int REVERSE = 'R';
const int HALT = 'H';

unsigned long durations[PUMP_COUNT] = {0};

void setup() {
    setupTimekeeping();

    for (int i = 0; i < PUMP_COUNT; i++) {
        pinMode(PUMP_IN1[i], OUTPUT);
        pinMode(PUMP_IN2[i], OUTPUT);
        pinMode(PUMP_PWM[i], OUTPUT);
    }

    Serial.begin(115200);
    for (int i = 0; i < numRelays; i++) {
        pinMode(relayPins[i], OUTPUT);
        // digitalWrite(relayPins[i], LOW);
        relays[i] = {relayPins[i], false, 0};
    }
}

void setupTimekeeping() {
    currentTime = millis();
    lastTime = currentTime;
}

void loop() {
    currentTime = millis();
    unsigned long deltaTime = currentTime - lastTime;

    reduceDurations(deltaTime);
    manageTaste();
    manageSmell();

    lastTime = currentTime;
}

void manageTaste() {
    if (Serial.available() > 0) {
      int num = 4;
        // String input = Serial.readStringUntil('\n');
        // char commandBuffer[input.length() + 1];
        // input.toCharArray(commandBuffer, sizeof(commandBuffer));

        // char *token = strtok(commandBuffer, " ");
        char *token = readCommand(num);
        char *array[num];
        int count = 0;

        while (token != NULL && count < num) {
            array[count++] = token;
            token = strtok(NULL, " ");
        }
        if (count == num) {
            int pumpIndex = array[0][0] - '1';
            char dirChar = array[1][0];
            int duration = atoi(array[2]);
            int speed = atoi(array[3]);

            if (pumpIndex >= 0 && pumpIndex < PUMP_COUNT) {
                if (speed >= 0 && speed <= 255) {
                    turn(PUMP[pumpIndex], dirChar, speed);
                    if (duration > 0) {
                        startDuration(PUMP[pumpIndex], duration);
                    }
                } else {
                    Serial.println("Error: Speed out of range (0 - 255)");
                }
            }
        }
    }
}

void manageSmell() {
    for (int i = 0; i < numRelays; i++) {
        if (relays[i].state && currentTime >= relays[i].endTime) {
            digitalWrite(relayPins[i], LOW);
            relays[i].state = false;
        }
    }
    if (Serial.available() > 0) {
      int num = 3;
      char* token = readCommand(num);
        // String input = Serial.readStringUntil('\n');
        // char commandBuffer[input.length() + 1];
        // input.toCharArray(commandBuffer, sizeof(commandBuffer));

        // char *token = strtok(commandBuffer, " ");
        char *array[num];
        int count = 0;

        while (token != NULL && count < num) {
            array[count++] = token;
            token = strtok(NULL, " ");
        }

        if (count == num) {
            int pumpIndex = atoi(array[0]) - 1;
            char pumpState = array[1][0];
            unsigned long duration = atol(array[2]);
            unsigned long endTime = currentTime + duration;

            if (pumpIndex >= 0 && pumpIndex < numRelays) {
                if (pumpState == '1') {
                    digitalWrite(relayPins[pumpIndex], HIGH);
                    relays[pumpIndex].state = true;
                    relays[pumpIndex].endTime = endTime;
                } else if (pumpState == '0') {
                    digitalWrite(relayPins[pumpIndex], LOW);
                    relays[pumpIndex].state = false;
                }
            }
        }
    }
}

char* readCommand(int num) {
  String input = Serial.readStringUntil('\n');
  char commandBuffer[input.length() + 1];
  input.toCharArray(commandBuffer, sizeof(commandBuffer));
        
  char *token = strtok(commandBuffer, " ");
  // char *array[num];
  // int count = 0;

  // while (token != NULL && count < num) {
  //   array[count++] = token;
  //   token = strtok(NULL, " ");
  // }
  return token;
}

int extractDuration(const char* str) {
    return atoi(str);
}

int extractSpeed(const char* str) {
    return atoi(str);
}

void startDuration(char pump, int duration) {
    int pumpIndex = pump - '1';
    durations[pumpIndex] = duration;
}

void reduceDurations(unsigned long deltaTime) {
    for (int i = 0; i < PUMP_COUNT; i++) {
        if (durations[i] != 0) {
            durations[i] = max(0, durations[i] - deltaTime);
            if (durations[i] == 0) {
                turn(PUMP[i], HALT, 0);
            }
        }
    }
}

void turn(char pump, char dir, int speed) {
    int index = pump - '1';
    if (index >= 0 && index < PUMP_COUNT) {
        turnDirection(pump, dir);
        analogWrite(PUMP_PWM[index], speed);
    }
}

void turnDirection(char pump, char dir) {
    int index = pump - '1';
    if (index >= 0 && index < PUMP_COUNT) {
        int out1 = LOW, out2 = LOW;
        switch (dir) {
            case FORWARD: out1 = HIGH; break;
            case REVERSE: out2 = HIGH; break;
        }
        digitalWrite(PUMP_IN1[index], out1);
        digitalWrite(PUMP_IN2[index], out2);
    }
}
