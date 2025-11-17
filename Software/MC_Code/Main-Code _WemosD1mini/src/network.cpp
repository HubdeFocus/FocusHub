#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <LittleFS.h>
#include "eeprom_management.h"
#include "network.h"
#include "save.h"
#include "learn.h"


const char* ap_ssid = "FocusHub_Setup_AP";
const char* ap_password = "12345678";

extern void writeWiFiCredentials(const String &ssid, const String &password);
extern String readSSID();
extern String readPassword();
extern void saveLearntime(String name, String duration, String breaktime);
extern void startLearningSession(const String &name, int duration, int breaktime);

//* WLAN Verbindung 
bool tryConnectToWiFi(String ssid, String password) {
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.println("Verbinde mit Heim-WLAN...");

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Verbunden! IP: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("Fehler - starte wieder im AP-Modus.");
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_password);
    return false;
  }
}


// Access point (also Hotelwlan seite) einrichten
void setupAP(ESP8266WebServer &server, DNSServer &dnsServer) {
  const int DNS_PORT = 53;
  //IP_Address: 192.168.4.1
  IPAddress apIP(192, 168, 4, 1);

  Serial.println("Starte Access Point...");
  WiFi.softAP(ap_ssid, ap_password);
  delay(500);

  Serial.print("Access Point Name: ");
  Serial.println(ap_ssid);
  Serial.println("Access Point Password: ");
  Serial.println(ap_password);

  dnsServer.start(DNS_PORT, "*", apIP);
  server.begin();

  server.on("/", [&server]() {
    File file = LittleFS.open("/AP_form.html", "r");
    if (!file) {
      server.send(404, "text/plain", "File not found");
      return;
    }
    server.streamFile(file, "text/html");
    file.close();
  });

  server.on("/save", HTTP_POST, [&server]() {
    String password = server.arg("password");
    String ssid = server.arg("ssid");
    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);
    server.send(200, "text/html", "<h2>Verbindung wird hergestellt...</h2><p>Du kannst dieses Fenster schließen.</p>");
    
    writeWiFiCredentials(ssid, password);
    tryConnectToWiFi(ssid, password);
    server.stop();
  });

   server.onNotFound([&server]() {
    server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true); // -> redirect
    server.send(302, "text/plain", "");
  });

  //MEGADragon2.0
  server.begin();
  Serial.println("HTTP-Server gestartet.");
  // Hab ich mal unkommentiert, weil sonst natürlich about blank kommt - der server ist dann nie online wenn man noch keine verbindung hatte.


  // Ich dachte es funktioniert auch so, aber vielleicht braucht man das processing während der verbindung schon
  while (WiFi.getMode() == WIFI_AP) {
    dnsServer.processNextRequest();
    server.handleClient();
    delay(10);
  }
}


bool checkAndResetWifi(ESP8266WebServer &server, DNSServer &dnsServer) {
  const int CLEAR_BUTTON_PIN = D3;
  pinMode(CLEAR_BUTTON_PIN, INPUT_PULLUP);
  unsigned long pressedTime = 0;
  const unsigned long requiredHold = 5000;

  if (digitalRead(CLEAR_BUTTON_PIN) == LOW) {
    pressedTime = millis();
    while (digitalRead(CLEAR_BUTTON_PIN) == LOW) {
      if (millis() - pressedTime >= requiredHold) {
        setupAP(server, dnsServer);
        return true;
      }
      delay(10);
    }
  }
  return false;
}


void setupInterfaceServer(ESP8266WebServer &server) {
  // get Ip and print
  Serial.print("Hosting interface at: ");
  Serial.println(WiFi.localIP());
  // server starten
  server.begin();
  Serial.println("Server gestartet");
  

  server.on("/", [&server]() {
    Serial.println("Interface requested");
    File file = LittleFS.open("index.html", "r");
    if (!file) {
      server.send(404, "text/plain", "File not found");
      return;
    }
    server.streamFile(file, "text/html");
    file.close();
  });
  server.on("/test", [&server]() {
    Serial.println("Test page requested");
    server.send(200, "text/html", "<h2>Test Seite funktioniert!</h2><p>Alles in Ordnung.</p>");
  });
  server.on("/create_learntime", [&server]() {
    Serial.println("Create Learntime requested");
    File file = LittleFS.open("create_learntime.html", "r");
    if (!file) {
      server.send(404, "text/plain", "File not found");
      return;
    }
    server.streamFile(file, "text/html");
    file.close();
  });
  server.on("/save_learntime", HTTP_POST, [&server]() {
    String name = server.arg("name");
    String duration = server.arg("duration");
    String breaktime = server.arg("breaktime");
    saveLearntime(name, duration, breaktime);
    Serial.println("Learntime Created:");
    Serial.println("Name: " + name);
    Serial.println("Duration: " + duration);
    Serial.println("Breaktime: " + breaktime);
    //request->redirect("/overview_learntimes");
    server.sendHeader("Location", "/overview_learntimes");
    server.send(303);
  });
  server.on("/overview_learntimes", [&server]() {

  File data_file = LittleFS.open("/learntimes.html", "r");
  File html_file = LittleFS.open("/overview_learntimes.html", "r");

  if (!data_file || !html_file) {
    server.send(404, "text/plain", "File not found");
    return;
  }

  String html = html_file.readString();
  html_file.close();

  // Build table with fixed headers
  String table = "<table border='1' style='border-collapse:collapse;width:100%;text-align:center'>";
  table += "<thead><tr><th>Name</th><th>Duration</th><th>Breaktime</th><th></th></tr></thead><tbody>";

  table += data_file.readString();
  
  table += "</tbody></table>";
  data_file.close();

  html.replace("{{CSV_TABLE}}", table);
  server.send(200, "text/html", html);
});
  server.on("/start_learntime", HTTP_GET, [&server]() {
    // Read parameters from URL query string
    String name = server.arg("name");
    String duration = server.arg("duration");
    String breaktime = server.arg("breaktime");

    Serial.println("Start Learntime requested");

    startLearningSession(name, duration.toInt(), breaktime.toInt());
    server.sendHeader("Location", "/");
    server.send(303);

});
}