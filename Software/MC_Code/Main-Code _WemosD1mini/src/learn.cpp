// Make functions to learn. E.g. the LED management during learning times, starting, pausing, stopping, etc.
#include <Arduino.h>

void startLearningSession(const String &name, int duration, int breaktime) {
  Serial.println("Lernsession gestartet für: " + name);
  Serial.println("Dauer: " + String(duration) + " Minuten");
  Serial.println("Pausenzeit: " + String(breaktime) + " Minuten");
  // Hier können weitere Aktionen hinzugefügt werden, z.B. LED steuerung oder timer keine Ahnung wie ihr das machen wollt
}
