#ifndef OPERATION_MODE_H
#define OPERATION_MODE_H

enum class OperationMode
{
    HOME,
    OUT
};
void initiateOperationMode();
void toggleOperationMode();
OperationMode getOperationMode();
const char *getOperationModeString();
bool setOperationMode(OperationMode mode);

#endif