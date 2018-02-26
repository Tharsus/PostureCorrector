#include "checkposture.h"

CheckPosture::CheckPosture() {}

void CheckPosture::set_angleThreshold(int angle)
{
    angleThreshold = angle;
}

void CheckPosture::set_heightThreshold(int height)
{
    heightThreshold = height;
}

void CheckPosture::set_proximityThreshold(int proximity)
{
    proximityThreshold = proximity;
}

void CheckPosture::set_calibratedPosture(std::vector<double> receivedPosture)
{
    calibratedPosture = receivedPosture;
}

void CheckPosture::set_posture(std::vector<double> receivedPosture)
{
    posture = receivedPosture;
}

int CheckPosture::checkPosture(void)
{
    int result = 0;

    // Analyze roll of user
    if (posture[5] > calibratedPosture[5] + angleThreshold) {
        result = 1;
    }
    else if (posture[5] < calibratedPosture[5] - angleThreshold) {
        result = 2;
    }

    // Analyze height of user
    if (posture[1] > calibratedPosture[1] + heightThreshold) {
        result = 4;
    }

    // Analyze proximity of user
    if (posture[2] < calibratedPosture[2] - proximityThreshold) {
        result = 8;
    }

    return result;
}
