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
#include <opencv2/opencv.hpp>

#include <dlib/opencv.h>
//#include <dlib/opencv.hpp>
#include <dlib/image_io.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>

#include "checkposture.h"
#include "databaseconnection.h"

#include <QtSql>
#include <QSqlDatabase>

#include <QtWidgets>
#include <QtCharts>

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

    void on_rotationThreshold_valueChanged(int value);

    void on_heightThreshold_valueChanged(int value);

    void on_proximityThreshold_valueChanged(int value);

private:
    Ui::MainWindow *ui;

    QTimer *timer;
    VideoCapture cap;

    Mat frame;
    QImage qt_image;

    frontal_face_detector detector;
    shape_predictor shape_predictor;

    boolean calibrate;
    boolean calibrated;
    full_object_detection calibrated_pose;
    std::vector<double> calibrated_facePosition;

    boolean right_pose;

    QMediaPlayer *alertsound;
    QSystemTrayIcon *trayIcon;

    CheckPosture checkPosture;

    DatabaseConnection db;

    void show_frame(Mat &);

    void sound_alert();
    void tray_notification(boolean, QString);

    void face_rotation(full_object_detection);

    unsigned int getNumberOfDetectedFaces(full_object_detection &);
    //void FacePosition(full_object_detection, double, double, double, double, double, double);
    std::vector<double> get_facePosition(full_object_detection);

    void set_postureRecords(std::vector<unsigned int>, std::vector<QDateTime>);

    int numberOfAlerts;
    std::vector<qint64> durationOfAlert;
    // 0 good
    // 1
    // 2
    // 3
    // 4
};

#endif // MAINWINDOW_H
