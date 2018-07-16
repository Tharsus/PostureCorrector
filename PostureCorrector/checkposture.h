#ifndef CHECKPOSTURE_H
#define CHECKPOSTURE_H

#include <iostream>
#include <QDebug>
#include <QObject>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <dlib/opencv.h>
//#include <dlib/opencv.hpp>
#include <dlib/image_io.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>

#include <QTimer>

#include "list_of_states.h"
#include "list_of_modes.h"

class CheckPosture : public QObject
{
    Q_OBJECT

public:
    CheckPosture();
    ~CheckPosture();

    void checkFrame(cv::Mat &, int heightThreshold, int proximityThreshold, int angleThreshold);

public slots:
    void emitState();
    void processMode(int);

signals:
    void postureCalibrated();
    void postureStatus(int, double, double, double);
    void postureState(int);

private:
    dlib::frontal_face_detector detector;
    dlib::shape_predictor shape_predictor;

    std::vector<double> checkFacePosition(cv::Mat, dlib::full_object_detection);

    int checkPosture(int, int, int);
    int checkStatus(int);

    unsigned int numberOfFaces;
    std::vector<double> currentPosture;
    std::vector<double> calibratedPosture;
    dlib::full_object_detection calibratedLandmarks;

    std::vector<double> temporaryCalibratedPosture;
    dlib::full_object_detection temporaryCalibratedLandmarks;

    boolean calibrate;
    boolean calibrated;

    // Variables to track the state of user
    int state;
    int state_to_emit;
    QTimer *state_chronometer;
    boolean counting;

    int detectionMode;
};

#endif // CHECKPOSTURE_H
