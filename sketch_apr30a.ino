#include <DueTimer.h>
#include <RTCDue.h>
#include <LiquidCrystal.h>

// Pin definitions
#define ALARM_BUZZER 11        // Pin connected to the buzzer
#define RESET_BUTTON 2       // Reset button 
#define SNOOZE_BUTTON 3      // Snooze button 
#define START_BUTTON 44      // start stopwatch
#define STOP_BUTTON 42       // stop stopwatch
#define SWITCH_MODE 52     //  switch between clock and stopwatch modes
#define STOPWATCH_BUTTON 50
#define LED1 10               
#define LED2 8                 

// snooze period
const unsigned long SNOOZE_DURATION = 30000;  // 30,000 milliseconds (30 seconds)

// LCD (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(12, 13, 7, 6, 5, 4);

// Initialise RTC
RTCDue rtc(XTAL);

// Alarm variables
int alarm_hour = -1;
int alarm_minute = -1;

// Melody and duration arrays
int melody[] = {262, 294, 330, 349, 392, 440, 494};
int noteDuration = 200;

// Day names 
const char* dayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

// Alarm state variables
bool alarmRinging = false;
bool snoozeActive = false;
unsigned long snoozeStartTime;

// Stopwatch variables
bool stopwatchMode = false;
bool stopwatchRunning = false;
unsigned long stopwatchStart = 0;
unsigned long stopwatchElapsed = 0;

// Clock mode
bool clockMode = true;
void setup() {
    Serial.begin(9600);

    // Initialize pins
    pinMode(ALARM_BUZZER, OUTPUT);
    pinMode(RESET_BUTTON, INPUT_PULLUP);  
    pinMode(SNOOZE_BUTTON, INPUT_PULLUP); 
    pinMode(START_BUTTON, INPUT_PULLUP);  
    pinMode(STOP_BUTTON, INPUT_PULLUP);  
    pinMode(SWITCH_MODE, INPUT_PULLUP); 
    pinMode(STOPWATCH_BUTTON, INPUT_PULLUP);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);

    // Initialize RTC
    rtc.begin();
    rtc.setHours(11);
    rtc.setMinutes(30);
    rtc.setSeconds(0);
    rtc.setDay(14);
    rtc.setMonth(3);
    rtc.setYear(2025);

    // Initialize LCD
    lcd.begin(16, 2); // 16x2
}

void loop() {
    if (clockMode) {
        printDateAndTime();

        // Check if the current time matches the alarm time
        if (rtc.getHours() == alarm_hour && rtc.getMinutes() == alarm_minute && rtc.getSeconds() == 0 && !alarmRinging) {
            alarmRinging = true;
            displayWakeUpMessage(); // Display wake-up message on the LCD
        }

       
        handleSerialInput();

        // reset 
        if (digitalRead(RESET_BUTTON) == LOW) {
            alarmRinging = false; // Stop the alarm
            displayStopMessage(); // Display stop message on the LCD
            delay(1000);  
        }

        // snooze
        if (digitalRead(SNOOZE_BUTTON) == LOW && alarmRinging) {
            snoozeActive = true;
            snoozeStartTime = millis(); // Record snooze start time
            lcd.clear(); // Clear the LCD display
            lcd.setCursor(0, 0);
            lcd.print("Snooze for 30s"); // Display snooze message
        }

        // If snooze is active and snooze period has elapsed, reset snooze state
        if (snoozeActive && (millis() - snoozeStartTime >= SNOOZE_DURATION)) {
            snoozeActive = false;
        }

        // If alarm is ringing and snooze is not active, play the alarm melody
        if (alarmRinging && !snoozeActive) {
            playAlarmRinger();
        }

    } else if (stopwatchMode) {
        // Stopwatch mode operations
        handleStopwatch();

        // start button press to start the stopwatch
        if (digitalRead(START_BUTTON) == LOW) {
            if (!stopwatchRunning) {
                stopwatchRunning = true;
                stopwatchStart = millis(); 
            }
            delay(200); // Debounce delay
        }

        // stop button press to stop the stopwatch
        if (digitalRead(STOP_BUTTON) == LOW) {
            if (stopwatchRunning) {
                stopwatchRunning = false;
                stopwatchElapsed += millis() - stopwatchStart;
            }
            delay(200); 
        }

        // reset button press to reset the stopwatch
        if (digitalRead(RESET_BUTTON) == LOW) {
            stopwatchRunning = false;
            stopwatchElapsed = 0; // Reset the elapsed time
            lcd.clear();
            lcd.setCursor(0, 0);
    lcd.print("Stopwatch reset");
            delay(200); // Debounce delay
        }
    }

    // switch button press to toggle between clock mode and stopwatch mode
    if (digitalRead(SWITCH_MODE) == LOW) {
        if (clockMode) {
            clockMode = false;
            stopwatchMode = true;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Stopwatch mode");
        } else {
            clockMode = true;
            stopwatchMode = false;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Clock mode");
        }
        delay(200); 
    }

    delay(1000);
}

void printDateAndTime() {
    // serial monitor display
    Serial.print(dayNames[rtc.getDayofWeek()]);
    Serial.print(" ");
    Serial.print(rtc.getDay());
    Serial.print("/");
    Serial.print(rtc.getMonth());
    Serial.print("/");
    Serial.print(rtc.getYear());
    Serial.print("\t");
    Serial.print(rtc.getHours());
    Serial.print(":");
    Serial.print(rtc.getMinutes());
    Serial.print(":");
    Serial.println(rtc.getSeconds());

    // date and time LCD display
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(dayNames[rtc.getDayofWeek()]);
    lcd.print(" ");
    lcd.print(rtc.getDay());
    lcd.print("/");
    lcd.print(rtc.getMonth());
    lcd.print("/");
    lcd.print(rtc.getYear());
    lcd.setCursor(0, 1);
    lcd.print(WithZeros(rtc.getHours(), 2));
    lcd.print(":");
    lcd.print(WithZeros(rtc.getMinutes(), 2));
    lcd.print(":");
    lcd.print(WithZeros(rtc.getSeconds(), 2));
}

void handleStopwatch() {
    // current elapsed time
    unsigned long currentTime = stopwatchRunning ? millis() - stopwatchStart + stopwatchElapsed : stopwatchElapsed;

    // Calculate hours, minutes, seconds, and milliseconds
    unsigned long hh = currentTime / 3600000; // Calculate hours
    unsigned long mm = (currentTime % 3600000) / 60000; // Calculate minutes
    unsigned long ss = (currentTime % 60000) / 1000; // Calculate seconds
    unsigned long ms = currentTime % 1000; // Calculate milliseconds

    // stopwatch time LCD display
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Stopwatch: ");
    lcd.setCursor(0, 1);
    // hh:mm:ss format 
    lcd.print((hh / 10) % 10);
    lcd.print(hh % 10);
    lcd.print(":");
    lcd.print((mm / 10) % 10);
    lcd.print(mm % 10);
    lcd.print(":");
    lcd.print((ss / 10) % 10);
    lcd.print(ss % 10);
   }

void displayWakeUpMessage() {
    // Display "Wake up princess" 
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wake up princess");
}

void displayStopMessage() {
    // Display "Get up girl" 
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Get up girl");
}

void playAlarmRinger() {
    // Play the alarm melody
    for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
        int frequency = melody[i];

        // Flash LED
        digitalWrite(LED1, HIGH);
        digitalWrite(LED2, HIGH);
        delay(100); // Adjust delay to control LED flashing speed
        digitalWrite(LED1, LOW);
        digitalWrite(LED2, LOW);
        for (int j = 0; j < noteDuration * frequency / 1000; j++) {
            digitalWrite(ALARM_BUZZER, HIGH);
            delayMicroseconds(500000 / frequency);
            digitalWrite(ALARM_BUZZER, LOW);
            delayMicroseconds(500000 / frequency);
        }
        delay(50);
    }
}

void handleSerialInput() {
    // alarm setting
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        int index = input.indexOf(':');
        if (index != -1) {
            int serial_hour = input.substring(0, index).toInt();
            int serial_minute = input.substring(index + 1).toInt();
            if (serial_hour >= 0 && serial_hour <= 23 && serial_minute >= 0 && serial_minute <= 59) {
                alarm_hour = serial_hour;
                alarm_minute = serial_minute;
                Serial.print("Alarm set for ");
                Serial.print(alarm_hour);
                Serial.print(":");
                Serial.println(alarm_minute);
            }
        }
    }
}


String WithZeros(int number, int length) {
    String result = String(number);
    while (result.length() < length) {
        result = "0" + result;
    }
    return result;
}
