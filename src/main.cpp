#include <Arduino.h>
#include <UIPEthernet.h>
#include <PubSubClient.h>
// #include <Bounce2.h>

#define PIN_RELAY   4
#define PIN_MUTE    2


EthernetClient ethClient;
PubSubClient client(ethClient);
IPAddress server(192, 168, 0, 204);
// Bounce mute = Bounce();

// IPAddress ip(192, 168, 0, 66);
// IPAddress mask(255, 255, 255, 0);
// IPAddress gw(192, 168, 0, 1);
// IPAddress dnsserver(192, 168, 0, 1);

bool alarmEnabled = false;
bool alarmActive = false;
uint32_t alarmStarted = 0;
uint32_t alarmPaused = 0;
uint32_t alarmActiveDelay = 30000;
uint32_t alarmPausedDelay = 10000;
int alarmMaxAttempts = 25;
int alarmAttempt = 0;

void alarmOn() {
    if (!alarmEnabled) {
        alarmEnabled = true;
        alarmActive = true;
        alarmStarted = millis();
        alarmAttempt = 0;
        digitalWrite(PIN_RELAY, HIGH);
        Serial.println("[ ALARM ] Alarm has STARTED!");
        // client.publish("/alarm_siren/state", String(alarmMaxAttempts).c_str());
        client.publish("/alarm_siren/state", "1");
    } else {
        Serial.println("[ ALARM ] Alarm attempt counter has been cleaned.");
        alarmAttempt = 0;
    }
}

void alarmOff() {
    alarmEnabled = false;
    digitalWrite(PIN_RELAY, LOW);
    Serial.println("[ ALARM ] Alarm has STOPPED!");
    client.publish("/alarm_siren/state", "0");
}


void callback(char* topic, byte* payload, unsigned int length) {
    // String sTopic(topic);
    Serial.print("[ MQTT ] Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    if ((char)payload[0] == '1') {
        Serial.println("ON");
        alarmOn();
    } else if ((char)payload[0] == '0') {
        Serial.println("OFF");
        alarmOff();
    } else {
        Serial.println((char)payload[0]);
    }
    // for (int i = 0; i < length; i++) {
        // sPayload += (char)payload[i];
        // Serial.print((char)payload[i]);
    // }
    // Serial.print(sPayload);
    Serial.println();
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("[ MQTT ] Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("alarmSiren", "mcore", "1209kia00")) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            client.publish("/alarm_siren/status", "connected");
            // ... and resubscribe
            client.subscribe("/alarm_siren/alarm");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}


uint32_t muteLastChanged = 0;
int muteState = HIGH;


void setup() {
    pinMode(PIN_RELAY, OUTPUT);
    // digitalWrite(PIN_RELAY, HIGH);
    // delay(100);
    digitalWrite(PIN_RELAY, LOW);

    // pinMode(PIN_MUTE, INPUT_PULLUP);
    // mute.attach(PIN_MUTE);
    // mute.interval(50);

    Serial.begin(9600);
    Serial.println("\n\n\n[ INIT ] ALARM SIREN controller has started!");
    uint8_t mac[6] = { 0x74, 0x69, 0x69, 0x2D, 0x30, 0x31 };

    Ethernet.begin(mac);
    // Ethernet.begin(mac, ip, dnsserver, gw, mask);

    Serial.print("\tIP:\t");
    Serial.println(Ethernet.localIP());
    Serial.print("\tMask:\t");
    Serial.println(Ethernet.subnetMask());
    Serial.print("\tGW:\t");
    Serial.println(Ethernet.gatewayIP());
    Serial.print("\tDNS:\t");
    Serial.println(Ethernet.dnsServerIP());

    Serial.println("\tLink:\t");

    client.setServer(server, 1883);
    client.setCallback(callback);

}

void loop() {
    if (!client.connected()) {
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


    if (alarmEnabled) {
        if (alarmActive) {
            if (millis() > alarmStarted + alarmActiveDelay) {
                alarmAttempt++;
                alarmActive = false;
                alarmPaused = millis();
                digitalWrite(PIN_RELAY, LOW);
                Serial.print("[ ALARM ] Alarm paused for a while (");
                Serial.print(alarmAttempt);
                Serial.println(")");
                client.publish("/alarm_siren/state", "0");
            }
        } else {
            if (alarmAttempt > alarmMaxAttempts) {
                alarmOff();
            } else if (millis() > alarmPaused + alarmPausedDelay) {
                alarmActive = true;
                alarmStarted = millis();
                digitalWrite(PIN_RELAY, HIGH);
                Serial.println("[ ALARM ] Alarm active again.");
                client.publish("/alarm_siren/state", "1");
                // client.publish("/alarm_siren/state", String(alarmMaxAttempts - alarmAttempt).c_str());
            }
        }
    }

}