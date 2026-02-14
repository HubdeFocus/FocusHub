// Make functions to learn. E.g. the LED management during learning times, starting, pausing, stopping, etc.
#include <Arduino.h>
#include <vector>

bool isLearning = false;

class Learntime {
  public:
    Learntime(const String &name, const std::vector<int> &durations)
      : name(name), durations(durations) {}

    String getName() const { return name; }
    const std::vector<int>& getDurations() const { return durations; }

  private:
    String name;
    std::vector<int> durations;
};


void runLearntime(const Learntime &lt) {
  Serial.println("Starte Lernprogramm: " + lt.getName());

  const std::vector<int> &durs = lt.getDurations();

  for (size_t i = 0; i < durs.size(); i++) {
    int mins = durs[i];
    bool isBreak = (i % 2 == 1);

    if (isBreak) {
      Serial.println("Pause: " + String(mins) + " Minuten");
    } else {
      Serial.println("Lernen: " + String(mins) + " Minuten");
    }

    unsigned long interval = mins * 60000UL;
    unsigned long start = millis();

    while (millis() - start < interval) {
      // Hier kommt LED-Steuerung, Button-Check, Webserver-Refresh, was auch immer
      // Aber bitte *keine* busy-waits .
    }
  }

  Serial.println("Learntime beendet.");
}

Learntime createLearntime(const String &name, int duration, int breaktime) {
  std::vector<int> durations;
  for (int i = 0; i < 4; i++) { // 4 Lernphasen
    durations.push_back(duration);
    if (i < 3) { // Pause nach jeder Lernphase außer der letzten
      durations.push_back(breaktime);
    }
  }
  return Learntime(name, durations);
}

void runNewLearntime(const String &name, int duration, int breaktime) {
  Learntime lt = createLearntime(name, duration, breaktime);
  runLearntime(lt);
}
