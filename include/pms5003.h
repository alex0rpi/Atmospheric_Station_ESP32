#ifndef PMS5003_H
#define PMS5003_H

void startPMS5003();

void wakePMS5003();
void sleepPMS5003();
bool updatePMS5003();
bool hasPMSData();

float readPM1();
float readPM25();
float readPM10();

#endif