#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QObject>

#include <QMediaPlayer>
#include <QSystemTrayIcon>

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <dlib/opencv.h>
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

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QSystemTrayIcon *trayIcon;

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void on_pushButton_Start_clicked();

    void on_pushButton_Stop_clicked();

    void on_pushButton_CalibrateMode_clicked();

    void on_pushButton_CalibrateNext_clicked();

    void on_pushButton_CalibrateCancel_clicked();

    void on_pushButton_Calibrate_clicked();

    void update_window();

    void on_rotationThreshold_valueChanged(int value);

    void on_heightThreshold_valueChanged(int value);

    void on_proximityThreshold_valueChanged(int value);

    void on_distanceThreshold_valueChanged(int value);

    void processPosture(int, double, double, double, double);

    void processState(int);

    void checkPosture_calibrated(void);

signals:
    void detectionMode(int);

private:
    Ui::MainWindow *ui;

    QTimer *timer;
    cv::VideoCapture cap;

    cv::Mat frame;
    QImage qt_image;

    unsigned int numberOfCalibrations;

    QMediaPlayer *alertsound;

    CheckPosture checkPosture;

    DatabaseConnection db;

    void show_frame(cv::Mat &);

    void sound_alert();
    void tray_notification(boolean, QString);

    int mode;
    int rotationThreshold;
    int heightThreshold;
    int proximityThreshold;
    int distanceThreshold;


    std::vector<QDate> days;
    std::vector< std::vector<int> > alertsInEachState;
    std::vector< std::vector<int> > durationInEachState;


    void set_postureRecords(std::vector<unsigned int>, std::vector<QDateTime>);

    int numberOfAlerts;
    std::vector<qint64> durationOfAlert;


    // Variables from statistics
    int previousPosture;
    QTime chronometer;
    int timeInEachState[6];
    int numberOfAlarmsForEachState[5];
    // Check previous and timeInEachState initialization


    // Bar Chart Variables
    int maxBarAxisY;
    QtCharts::QBarSet *set[5];
    QtCharts::QBarSeries *barSeries;
    QtCharts::QChart *barChart;
    QtCharts::QBarCategoryAxis *axis;
    QtCharts::QChartView *barChartView;

    // Pie Chart Variables
    QtCharts::QPieSeries *pieSeries;


    void initializeCharts(void);

    void updateBarChart(int);
    void updatePieChart(int, int);

    void showThresholds(boolean);
    void showResults(boolean);
    void enableResults(boolean);
};

#endif // MAINWINDOW_H
