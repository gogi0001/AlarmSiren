#include <Arduino.h>
#include <UIPEthernet.h>
#include <PubSubClient.h>
#include "Ticker.h"
// #include <Bounce2.h>

#define PIN_LIGHT   5
#define PIN_SIREN   4
#define PIN_MUTE    2

void blinkOn();
void blinkOff();
void alarmOn();
void alarmOn3();
void alarmOff();

const uint32_t resetDelay = 43200000; //((60 * 60) * 12) * 1000

void(* resetFunc) (void) = 0;//declare reset function at address 0

Ticker timerBlink1(blinkOn, 300, 1, MILLIS);
Ticker timerBlink2(blinkOn, 300, 2, MILLIS);
Ticker timerBlink3(blinkOn, 300, 3, MILLIS);
Ticker timerBlinkOff(blinkOff, 150, 1, MILLIS);

Ticker timerAlarmOn(alarmOn, 180, 3, MILLIS);
Ticker timerAlarmOn3(alarmOn3, 3000, 0, MILLIS);
Ticker timerAlarmOff(alarmOff, 50, 1, MILLIS);

Ticker timerReset(resetFunc, resetDelay, 1, MILLIS);



EthernetClient ethClient;
PubSubClient client(ethClient);
IPAddress server(172, 17, 2, 172);
//IPAddress server(192, 168, 0, 204);
// Bounce mute = Bounce();

// IPAddress ip(192, 168, 0, 66);
// IPAddress mask(255, 255, 255, 0);
// IPAddress gw(192, 168, 0, 1);
// IPAddress dnsserver(192, 168, 0, 1);

void blinkOn();
void blinkOff();

Ticker timerBlink1(blinkOn, 1, 200, MILLIS);
Ticker timerBlinkOff(blinkOff, 1, 100, MILLIS);

bool alarmEnabled = false;
bool alarmActive = false;
uint32_t alarmStarted = 0;
uint32_t alarmPaused = 0;
uint32_t alarmActiveDelay = 30000;
uint32_t alarmPausedDelay = 10000;
int alarmMaxAttempts = 25;
int alarmAttempt = 0;
String sTopic;


void blinkOn() {
    // Serial.println(F("[ BLINK ] Lets blink some leds!"));
    digitalWrite(PIN_LIGHT, HIGH);
    timerBlinkOff.start();
}

void blinkOff() {
    // Serial.println(F("[ BLINK ] Blink light is off."));
    digitalWrite(PIN_LIGHT, LOW);
}

void alarmOn3() {
    timerAlarmOn.start();
    // Serial.println("SIREN3!");
}

void blinkOn() {

}

void blinkOff() {
    
}

void alarmOn() {
    digitalWrite(PIN_LIGHT, HIGH);
    digitalWrite(PIN_SIREN, HIGH);
    timerAlarmOff.start();
    // client.publish("/alarm_siren/state", "1");

    // if (!alarmEnabled) {
    //     alarmEnabled = true;
    //     alarmActive = true;
    //     alarmStarted = millis();
    //     alarmAttempt = 0;
    //     digitalWrite(PIN_LIGHT, HIGH);
    //     digitalWrite(PIN_SIREN, HIGH);
    //     Serial.println(F("[ ALARM ] Alarm has STARTED!"));
    //     // client.publish("/alarm_siren/state", String(alarmMaxAttempts).c_str());
    //     client.publish("/alarm_siren/state", "1");
    // } else {
    //     Serial.println(F("[ ALARM ] Alarm counter cleaned."));
    //     alarmAttempt = 0;
    // }
}

void alarmOff() {
    // alarmEnabled = false;
    digitalWrite(PIN_LIGHT, LOW);
    digitalWrite(PIN_SIREN, LOW);
    // Serial.println(F("[ ALARM ] STOPPED!"));
    // client.publish("/alarm_siren/state", "0");
}


void callback(char* topic, byte* payload, unsigned int length) {
    sTopic = topic;
    Serial.print(F("[ MQTT ] Message ["));
    Serial.print(topic);
    Serial.print(F("] "));
    Serial.println((char)payload[0]);
    if (sTopic == "/alarm_siren/alarm") {
        if ((char)payload[0] == '1') {
            client.publish("/alarm_siren/state", "1");
            timerAlarmOn3.start();
            // alarmOn();
        } else {
            // alarmOff();
            timerAlarmOn3.stop();
            client.publish("/alarm_siren/state", "0");
        }
    }
    if (sTopic == "/alarm_siren/blink") {
        if ((char)payload[0] == '1') {
            timerBlink1.start();
        } else if ((char)payload[0] == '2') {
            timerBlink2.start();
        } else if ((char)payload[0] == '3') {
            timerBlink3.start();
        }
    }
    // for (int i = 0; i < length; i++) {
        // sPayload += (char)payload[i];
        // Serial.print((char)payload[i]);
    // }
    // Serial.print(sPayload);
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print(F("[ MQTT ] Connecting..."));
        // Attempt to connect
        if (client.connect("alarmSiren", "mcore", "1209kia00")) {
            Serial.println(F("OK"));
            // Once connected, publish an announcement...
            client.publish("/alarm_siren/status", "connected");
            // ... and resubscribe
            client.subscribe("/alarm_siren/alarm");
            client.subscribe("/alarm_siren/blink");
            timerBlink3.start();
        } else {
            Serial.print(F("FAIL, rc="));
            Serial.print(client.state());
            Serial.println(F(" try again in 5 seconds"));
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}


uint32_t muteLastChanged = 0;
int muteState = HIGH;


void setup() {
    pinMode(PIN_LIGHT, OUTPUT);
    pinMode(PIN_SIREN, OUTPUT);
    // digitalWrite(PIN_RELAY, HIGH);
    // delay(100);
    digitalWrite(PIN_LIGHT, LOW);
    digitalWrite(PIN_SIREN, LOW);

    // pinMode(PIN_MUTE, INPUT_PULLUP);
    // mute.attach(PIN_MUTE);
    // mute.interval(50);

    Serial.begin(9600);
    Serial.println(F("\n\n\n[ INIT ] Controller has started!"));
    uint8_t mac[6] = { 0x74, 0x69, 0x69, 0x2D, 0x30, 0x31 };

    Ethernet.begin(mac);
    
    // Ethernet.begin(mac, ip, dnsserver, gw, mask);

    Serial.print(F("\tIP:\t"));
    Serial.println(Ethernet.localIP());
    Serial.println(F("\tLink:\t"));

    client.setServer(server, 1883);
    client.setCallback(callback);
    // timerBlink3.start();
    timerReset.start();
}

void loop() {
    timerReset.update();
    timerBlink1.update();
    timerBlink2.update();
    timerBlink3.update();
    timerBlinkOff.update();
    timerAlarmOn.update();
    timerAlarmOn3.update();
    timerAlarmOff.update();
    if (!client.connected()) {
        if (timerAlarmOn.state() == RUNNING) timerAlarmOn.stop();
        reconnect();
    }
    client.loop();

    // int muteCurrent = digitalRead(PIN_MUTE);
    // if ((muteCurrent != muteState) && (millis() > muteLastChanged + 50)) {
    //     muteLastChanged = millis();
    //     muteState = muteCurrent;
    //     Serial.println("[ MUTE ] Button has been pressed!");
    // }

    // mute.update();
    // if (mute.fell()) {
    //     Serial.println("[ MUTE ] Button has been pressed!");
    // }


    // if (alarmEnabled) {
    //     if (alarmActive) {
    //         if (millis() > alarmStarted + alarmActiveDelay) {
    //             alarmAttempt++;
    //             alarmActive = false;
    //             alarmPaused = millis();
    //             digitalWrite(PIN_SIREN, LOW);
    //             Serial.println(F("[ ALARM ] Alarm PAUSED"));
    //             client.publish("/alarm_siren/state", "0");
    //         }
    //     } else {
    //         if (alarmAttempt > alarmMaxAttempts) {
    //             alarmOff();
    //         } else if (millis() > alarmPaused + alarmPausedDelay) {
    //             alarmActive = true;
    //             alarmStarted = millis();
    //             digitalWrite(PIN_SIREN, HIGH);
    //             Serial.println(F("[ ALARM ] Alarm active again."));
    //             client.publish("/alarm_siren/state", "1");
    //             // client.publish("/alarm_siren/state", String(alarmMaxAttempts - alarmAttempt).c_str());
    //         }
    //     }
    // }

}