#include <LittleFS.h>

void saveLearntime(String name, String duration,  String breaktime) {
  String filePath = "learntimes.html";
  File file = LittleFS.open(filePath, "a");
  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei zum Speichern der Lernzeit.");
    return;
  }
  file.println("<tr><td>" + name + "</td><td>" + duration + "</td><td>" + breaktime + '</td><td><button onclick="location.href="/start_learntime"">Learntimes</button></td></tr><br>\n');
  file.close();
  Serial.println("Lernzeit gespeichert: " + filePath);
}