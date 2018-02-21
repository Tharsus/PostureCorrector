#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include <QMediaPlayer>
#include <QSystemTrayIcon>

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <dlib/opencv.h>
//#include <dlib/opencv.hpp>
#include <dlib/image_io.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>

using namespace std;
using namespace cv;
using namespace dlib;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_Start_clicked();

    void on_pushButton_Stop_clicked();

    void on_pushButton_Calibrate_clicked();

    void update_window();

private:
    Ui::MainWindow *ui;

    QTimer *timer;
    VideoCapture cap;

    Mat frame;
    QImage qt_image;

    frontal_face_detector detector;
    shape_predictor shape_model;

    boolean calibrate;
    boolean calibrated;
    full_object_detection calibrated_pose;

    boolean right_pose;

    QMediaPlayer *alertsound;
    QSystemTrayIcon *trayIcon;

    void show_frame(Mat &);

    int compare(full_object_detection current_pose);
    int posture_score(full_object_detection current_pose);
    void sound_alert();
    void tray_notification(boolean);

    void face_rotation(full_object_detection current_pose);



    /*int mImageW = 0;
    int mImageH = 0;

    virtual inline int det(cv::Mat& image) {
        mImageW = image.cols;
        mImageH = image.rows;
    }

    //Create two new functions
    int getFrameWidth() {return mImageW;}
    int getFrameHeight() {return mImageH;}*/
};

#endif // MAINWINDOW_H
