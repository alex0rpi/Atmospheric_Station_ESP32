#ifndef MQTT_H
#define MQTT_H

void startMQTT();
void stopMQTT();
void maintainMQTT();
void publish(const char *topic, const char *mensaje);
void publish(const char *topic, const char *mensaje, bool retained);
void publishHomeAssistantDiscovery();

#endif
