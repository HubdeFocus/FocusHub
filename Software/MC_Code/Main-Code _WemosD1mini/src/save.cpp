#include <LittleFS.h>

void saveLearntime(String name, String duration,  String breaktime) {
  String filePath = "learntimes.html";
  File file = LittleFS.open(filePath, "a");
  if (!file) {
    Serial.println("Fehler beim Öffnen der Datei zum Speichern der Lernzeit.");
    return;
  }
  file.print("<tr><td>");
  file.print(name);
  file.print("</td><td>");
  file.print(duration);
  file.print("</td><td>");
  file.print(breaktime);
  file.print("</td><td><a href=\"/start_learntime?name=");
  file.print(name);
  file.print("&duration=");
  file.print(duration);
  file.print("&breaktime=");
  file.print(breaktime);
  file.print("\"><button>Start</button></a></td></tr>\n");

  file.close();
  Serial.println("Lernzeit gespeichert: " + filePath);
}