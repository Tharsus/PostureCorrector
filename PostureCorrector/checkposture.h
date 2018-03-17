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

#include "list_of_states.h"

class CheckPosture : public QObject
{
    Q_OBJECT

public:
    CheckPosture();

    void set_calibrateTrue(void);
    boolean postureCalibrated(void);

    void checkFrame(cv::Mat &, int heightThreshold, int proximityThreshold, int angleThreshold);

signals:
    void postureStatus(int);

private:
    dlib::frontal_face_detector detector;
    dlib::shape_predictor shape_predictor;

    void set_posture(std::vector<double>);
    void set_calibratedPosture(std::vector<double>);

    std::vector<double> checkFacePosition(cv::Mat, dlib::full_object_detection);

    int checkPosture(int, int, int);

    unsigned int numberOfFaces;
    std::vector<double> currentPosture;
    std::vector<double> calibratedPosture;
    dlib::full_object_detection calibratedLandmarks;

    boolean calibrate;
    boolean calibrated;

    unsigned int state;
    unsigned int current_state;
};

#endif // CHECKPOSTURE_H
