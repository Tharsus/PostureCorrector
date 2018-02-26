#ifndef CHECKPOSTURE_H
#define CHECKPOSTURE_H

#include <iostream>
#include <vector>

class CheckPosture
{
public:
    CheckPosture();

    void set_angleThreshold(int);
    void set_heightThreshold(int);
    void set_proximityThreshold(int);

    void set_calibratedPosture(std::vector<double>);
    void set_posture(std::vector<double>);

    int checkPosture(void);

private:
    int angleThreshold;
    int heightThreshold;
    int proximityThreshold;

    std::vector<double> calibratedPosture;
    std::vector<double> posture;

};

#endif // CHECKPOSTURE_H
