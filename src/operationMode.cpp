#include "operationMode.h"

static OperationMode currentMode = OperationMode::HOME;

void initiateOperationMode()
{
    currentMode = OperationMode::OUT;
}

OperationMode getOperationMode()
{
    return currentMode;
}

const char *getOperationModeString()
{
    switch (currentMode)
    {
    case OperationMode::HOME:
        return "HOME";
    case OperationMode::OUT:
        return "OUT";
    default:
        return "UNKNOWN";
    }
}

void toggleOperationMode()
{
    switch (currentMode)
    {
    case OperationMode::HOME:
        currentMode = OperationMode::OUT;
        break;
    case OperationMode::OUT:
        currentMode = OperationMode::HOME;
        break;
    }
}

bool setOperationMode(OperationMode mode)
{
    if (currentMode != mode)
    {
        currentMode = mode;
        return true;
    }
    return false;
}