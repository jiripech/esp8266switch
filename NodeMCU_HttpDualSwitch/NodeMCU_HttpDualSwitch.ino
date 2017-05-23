#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

// #include <ArduinoOTA.h>

ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer server(80);

const char* www_username = "admin";
const char* www_password = "putYourSecretHere";

const String JSON_begin = "{ \"switch1\": ";
const String JSON_mid   = ", \"switch2\": ";
const String JSON_ee    = ", \"saved\": ";
const String JSON_end   = " }";

const int pinSw1 = D2;
const int pinSw2 = D3;

boolean switched1On = true;
boolean switched2On = true;

boolean saved1 = true;
boolean saved2 = true;

unsigned int interval = 10000;
unsigned int subInterval  = 250;
unsigned int counter = 10;

unsigned long toggle1Millis = 0;
unsigned long toggle2Millis = 0;

unsigned long previousMillis = 0;
unsigned long previousTMillis = 0;
int ledState = HIGH;

const byte S1 = 1;
const byte S2 = 2;

void switchOn(byte sw = 0) {
  if (sw == S1) {
    switched1On = true;
    digitalWrite(pinSw1, LOW);
  } else if (sw == S2) {
    switched2On = true;
    digitalWrite(pinSw2, LOW);
  }  
}

void switchOff(byte sw = 0) {
  if (sw == S1) {
    switched1On = false;
    digitalWrite(pinSw1, HIGH);
  } else if (sw == S2) {
    switched2On = false;
    digitalWrite(pinSw2, HIGH);
  }
}

void toggle(byte sw = 0) {
  if (sw == S1) {
    if (switched1On) {
      switchOff(sw);
    } else {
      switchOn(sw);
    }
  } else if (sw == S2) {
    if (switched2On) {
      switchOff(sw);
    } else {
      switchOn(sw);
    }
  }
}

void reload() {
  // read a byte from the current address of the EEPROM
  if ((EEPROM.read(500) == byte('A')) && (EEPROM.read(501) == byte('S')) && (EEPROM.read(502) == byte('S'))) {
    switched1On = saved1 = EEPROM.read(503);
    switched2On = saved2 = EEPROM.read(504);
  }

  // initialize both switch pins as outputs
  pinMode(pinSw1, OUTPUT);
  pinMode(pinSw2, OUTPUT);

  if (switched1On) switchOn(S1);
    else switchOff(S1);

  if (switched2On) switchOn(S2);
    else switchOff(S2);
}

void handleAuth() {
  if(!server.authenticate(www_username, www_password))
    return server.requestAuthentication();
}

void handleStatus() {
  server.send(200, "application/json", JSON_begin + ((switched1On) ? ("true") : ("false")) + JSON_mid + ((switched2On) ? ("true") : ("false")) + JSON_ee + ((saved1 == switched1On && saved2 == switched2On) ? ("true") : ("false")) + JSON_end);
}

void setup() {
    EEPROM.begin(512);
    reload();

    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    delay(10);

    // We start by connecting to a WiFi network
    WiFiMulti.addAP("YourAP1", "YourSecret1");
    WiFiMulti.addAP("YourAP2", "YourSecret2");

    Serial.print("Waiting for WiFi: O");
    delay(500);

    while(WiFiMulti.run() != WL_CONNECTED) {
      Serial.print("o");
      delay(500);
    }

    Serial.println("h!");
    Serial.print("Connected with IP address: ");
    Serial.print(WiFi.localIP());
    Serial.println("");

    delay(500);

    if (MDNS.begin("airswitchesp8266")) {
      Serial.println("MDNS responder started.");
    }

    // No over-the-air upgrades at the moment
    // ArduinoOTA.begin();

    server.on("/", []() {
      handleAuth();
      server.send(200, "text/plain", "OK");
    });

    server.on("/status", []() {
      handleAuth();
      handleStatus();
      Serial.println("Served /status");
    });

    server.on("/on", []() {
      handleAuth();
      switchOn(S1);
      switchOn(S2);
      handleStatus();
      Serial.println("Served /on");
    });
    server.on("/off", []() {
      handleAuth();
      switchOff(S1);
      switchOff(S2);
      handleStatus();
      Serial.println("Served /off");
    });

    server.on("/on/1", []() {
      handleAuth();
      switchOn(S1);
      handleStatus();
      Serial.println("Served /on/1");
    });
    server.on("/on/2", []() {
      handleAuth();
      switchOn(S2);
      handleStatus();
      Serial.println("Served /on/2");
    });

    server.on("/off/1", []() {
      handleAuth();
      switchOff(S1);
      handleStatus();
      Serial.println("Served /off/1");
    });
    server.on("/off/2", []() {
      handleAuth();
      switchOff(S2);
      handleStatus();
      Serial.println("Served /off/2");
    });

    server.on("/cycle/1", []() {
      handleAuth();
      toggle(S1);
      toggle1Millis = millis() + 5000;
      handleStatus();
      Serial.println("Served /cycle/1");
    });
    server.on("/cycle/2", []() {
      handleAuth();
      toggle(S2);
      toggle2Millis = millis() + 5000;
      handleStatus();
      Serial.println("Served /cycle/2");
    });
    server.on("/restart/1", []() {
      handleAuth();
      switchOff(S1);
      toggle1Millis = millis() + 5000;
      handleStatus();
      Serial.println("Served /restart/1");
    });
    server.on("/restart/2", []() {
      handleAuth();
      switchOff(S2);
      toggle2Millis = millis() + 5000;
      handleStatus();
      Serial.println("Served /restart/2");
    });

    server.on("/save", []() {
      // only rewrite EEPROM if the state is different
      if (saved1 != switched1On || saved2 != switched2On) {
        EEPROM.write(500, byte('A'));
        EEPROM.write(501, byte('S'));
        EEPROM.write(502, byte('S'));
        saved1 = switched1On;
        EEPROM.write(503, switched1On);
        saved2 = switched2On;
        EEPROM.write(504, switched2On);
        EEPROM.commit();
        Serial.println("Served /save (and saved)");
      } else {
        Serial.println("Served /save (already saved)");
      }

      handleStatus();
    });

    server.on("/reload", []() {
      reload();
      handleStatus();
      Serial.println("Served /reload");
    });

    server.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  // ArduinoOTA.handle();
  server.handleClient();

  // every time the we reach the interval, reset counter
  if(currentMillis - previousTMillis >= interval) {
    previousTMillis = previousMillis = currentMillis;

    // two blinks if everything is off
    counter = 2;
    if (switched1On) {
      // plus 3 more blinks if switch 1 is on
      counter += 3;
    }
    if (switched2On) {
      // plus 5 more blinks if switch 1 is on
      counter += 5;
    }
    // multiply by two, because we want to count blinks, not light toggles
    counter = counter * 2;
  }

  if ((toggle1Millis > 0) && (currentMillis >= toggle1Millis)) {
    toggle1Millis = 0;
    toggle(S1);
  }

  if ((toggle2Millis > 0) && (currentMillis >= toggle2Millis)) {
    toggle2Millis = 0;
    toggle(S2);
  }

  if (counter > 0) {
    if (currentMillis - previousMillis >= subInterval) {
      previousMillis = currentMillis;

      counter--;
      ledState = (ledState == HIGH) ? (LOW) : (HIGH);
    }
  } else {
    ledState = HIGH; // off
  }

  digitalWrite(LED_BUILTIN, ledState);
}
