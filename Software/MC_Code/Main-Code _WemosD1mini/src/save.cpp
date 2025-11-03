#include <LittleFS.h>

void saveLearntime(String name, String duration,  String breaktime) {
  String filePath = "learntimes.csv";
  File file = LittleFS.open(filePath, "w");
  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei zum Speichern der Lernzeit.");
    return;
  }
  file.println(name + ",");
  file.println(duration + ",");
  file.println(breaktime);
  file.close();
  Serial.println("Lernzeit gespeichert: " + filePath);
}