#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "checkposture.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    checkPosture(),
    db()
{
    ui->setupUi(this);

    timer = new QTimer(this);

    ui->pushButton_Start->setEnabled(true);
    ui->pushButton_Stop->setEnabled(false);
    ui->pushButton_Calibrate->setEnabled(false);

    numberOfCalibrations = 0;

    right_pose = true;

    alertsound = new QMediaPlayer();
    alertsound->setMedia(QUrl("C://Windows//media//Windows Notify System Generic.wav"));

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/myappico.png"));

    QObject::connect(this, SIGNAL(calibrateButton_clicked(void)), &checkPosture, SLOT(calibratePosture()));
    QObject::connect(&checkPosture, SIGNAL(postureCalibrated()), this, SLOT(checkPosture_calibrated(void)));

    QObject::connect(&checkPosture, SIGNAL(postureStatus(int)), this, SLOT(processPosture(int)));


    // Rotation
    ui->rotationThreshold->setValue(20);
    ui->rotationDisplay->setValue(ui->rotationThreshold->value());

    // Height
    ui->heightThreshold->setValue(200);
    ui->heightDisplay->setValue(ui->heightThreshold->value());

    // Proximity
    ui->proximityThreshold->setValue(1000);
    ui->proximityDisplay->setValue(ui->proximityThreshold->value());


    if (db.openDatabase()) {
        qDebug() << "Database PostureStatus opened";
        std::vector<unsigned int> status;
        std::vector<QDateTime> dateTime;

        db.selectAllFromDatabase(status, dateTime);

        set_postureRecords(status, dateTime);
        qDebug() << "Number of Alerts: " << numberOfAlerts;
        qDebug() << "Duration of alert: " << durationOfAlert;


        qDebug() << "end of reading";
    } else { qDebug() << "error"; }




    /*
     * Initialize Charts
     */

    chronometer.start();
    previousPosture=0;
    for (int i=0; i<5; i++) {
        timeInEachState[i]=0;
        if (i<4) {
            numberOfAlarmsForEachState[i]=0;
        }
    }

    initializeCharts();
}

MainWindow::~MainWindow()
{
    if (numberOfCalibrations>0) {
        db.insertIntoDatabase(END);
    }

    delete ui;
}

void MainWindow::on_pushButton_Start_clicked()
{
    cap.open(0);

    if(!cap.isOpened()){
        qDebug() << "not able to open camera";
    } else {
        qDebug() << "camera is opened";
        ui->pushButton_Start->setEnabled(false);
        ui->pushButton_Stop->setEnabled(true);
        ui->pushButton_Calibrate->setEnabled(true);

        connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
        timer->start(50); //in miliseconds: 50ms = 0.05s => will grab a frame every 50ms = 20 frames/second
    }
}

void MainWindow::on_pushButton_Stop_clicked()
{
    ui->pushButton_Start->setEnabled(true);
    ui->pushButton_Stop->setEnabled(false);
    ui->pushButton_Calibrate->setEnabled(false);

    disconnect(timer, SIGNAL(timeout()),this, SLOT(update_window()));
    cap.release();
    qDebug() << "camera is closed";

    //fill display with a black screen
    cv::Mat image = cv::Mat::zeros(frame.size(),CV_8UC3);
    show_frame(image);
}

void MainWindow::on_pushButton_Calibrate_clicked()
{
    ui->pushButton_Calibrate->setEnabled(false);

    emit calibrateButton_clicked();
}

void MainWindow::checkPosture_calibrated()
{
    numberOfCalibrations += 1;
    if (numberOfCalibrations == 1) {
        ui->checkBox->setChecked(true);
        ui->pushButton_Calibrate->setText("Recalibrate");

        db.insertIntoDatabase(START);
    }
    ui->pushButton_Calibrate->setEnabled(true);
}

void MainWindow::on_rotationThreshold_valueChanged(int value) { ui->rotationDisplay->setValue(value); }
void MainWindow::on_heightThreshold_valueChanged(int value) { ui->heightDisplay->setValue(value); }
void MainWindow::on_proximityThreshold_valueChanged(int value) { ui->proximityDisplay->setValue(value); }


void MainWindow::update_window()
{
    cap >> frame;

    checkPosture.checkFrame(frame, ui->heightThreshold->value(), ui->proximityThreshold->value(), ui->rotationThreshold->value());

    // Flip horizontally so that image is shown like a mirror
    cv::flip(frame, frame, +1);

    show_frame(frame);
}

void MainWindow::show_frame(cv::Mat &image)
{
    // Resize image to the size of label_displayFace
    cv::Mat resized_image = image.clone();

    int width_of_label = ui->label_displayFace->width();
    int height_of_label = ui->label_displayFace->height();

    cv::Size size(width_of_label, height_of_label);
    cv::resize(image, resized_image, size);

    // Change color map so that it can be displayed in label_displayFace
    cvtColor(resized_image, resized_image, CV_BGR2RGB);
    ui->label_displayFace->setPixmap(QPixmap::fromImage(QImage(resized_image.data, resized_image.cols, resized_image.rows, QImage::Format_RGB888)));
}

void MainWindow::sound_alert()
{
    if (alertsound->state() == QMediaPlayer::PlayingState) {
        alertsound->setPosition(0);
    } else {
        alertsound->play();
    }
}

void MainWindow::tray_notification(boolean activate, QString message)
{
    //implementar redirecionamento do tray quando clicar
    //implementar mensagem quando hover

    if (activate) {
        trayIcon->show();
        trayIcon->showMessage("Warning", message, QSystemTrayIcon::Warning);
        trayIcon->messageClicked();
    } else {
        trayIcon->hide();
    }
}

void MainWindow::processPosture(int currentPosture)
{
    //remove
    if (previousPosture != currentPosture) {
        int index=0;
        if (previousPosture == LOW_HEIGHT) {
            index=1;
        }
        else if (previousPosture == TOO_CLOSE) {
            index=2;
        }
        else if (previousPosture == ROLL_RIGHT) {
            index=3;
        }
        else if (previousPosture == ROLL_LEFT) {
            index=4;
        }
        qDebug() << index;

        timeInEachState[index] += chronometer.elapsed();
        chronometer.restart();
        previousPosture = currentPosture;
    }

    for (int i=0; i<5; i++) {
        qDebug() << "Status " << i << " = " << timeInEachState[i]/1000;
    }
    qDebug() << endl;








    //qDebug() << "Status: " << postureCheck;
    if (right_pose) {
        // Roll left
        if (currentPosture == LOW_HEIGHT) {
            right_pose = false;
            //sound_alert();
            tray_notification(true, "Too low!");
            if (!db.insertIntoDatabase(currentPosture)) {}
        }
        // Roll right
        else if (currentPosture == TOO_CLOSE) {
            right_pose = false;
            tray_notification(true, "Too close!");
            if (!db.insertIntoDatabase(currentPosture)) {}
        }
        // low height
        else if (currentPosture == ROLL_RIGHT) {
            right_pose = false;
            tray_notification(true, "Rolling to the right!");
            if (!db.insertIntoDatabase(currentPosture)) {}
        }
        // too close
        else if (currentPosture == ROLL_LEFT) {
            right_pose = false;
            tray_notification(true, "Rolling to the left!");
            if (!db.insertIntoDatabase(currentPosture)) {}
        }
    } else {
        if (currentPosture == CORRECT_POSTURE) {
            right_pose = true;
            tray_notification(false, "");
            if (!db.insertIntoDatabase(currentPosture)) {}
        }
    }
}

void MainWindow::set_postureRecords(std::vector<unsigned int> status, std::vector<QDateTime> dateTime)
{
    // Initialize variables
    numberOfAlerts = 0;
    durationOfAlert.push_back(0);
    durationOfAlert.push_back(0);
    durationOfAlert.push_back(0);
    durationOfAlert.push_back(0);
    durationOfAlert.push_back(0);

    for (unsigned i=1; i<status.size(); i++) {
        if (status.at(i-1) == 0) {
            durationOfAlert.at(0) = dateTime.at(i-1).secsTo(dateTime.at(i));
        }
        else if (status.at(i-1) == 1) {
            numberOfAlerts += 1;
            durationOfAlert.at(1) = dateTime.at(i-1).secsTo(dateTime.at(i));
        }
        else if (status.at(i-1) == 2) {
            numberOfAlerts += 1;
            durationOfAlert.at(2) = dateTime.at(i-1).secsTo(dateTime.at(i));
        }
        else if (status.at(i-1) == 4) {
            numberOfAlerts += 1;
            durationOfAlert.at(3) = dateTime.at(i-1).secsTo(dateTime.at(i));
        }
        else if (status.at(i-1) == 8) {
            numberOfAlerts += 1;
            durationOfAlert.at(4) = dateTime.at(i-1).secsTo(dateTime.at(i));
        }
    }
}

void MainWindow::initializeCharts()
{
    // Bar Chart
    set0 = new QtCharts::QBarSet("Number of Ocurrencies");
    *set0 << 50 << 65 << 45 << 30 << 15 << 7;

    QtCharts::QBarSeries *barSeries = new QtCharts::QBarSeries();
    barSeries->append(set0);

    QtCharts::QChart *barChart = new QtCharts::QChart();
    barChart->addSeries(barSeries);
    barChart->setTitle("Simple barchart example");
    barChart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

    QStringList categories;
    categories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";
    axis = new QtCharts::QBarCategoryAxis();
    axis->append(categories);
    barChart->createDefaultAxes();
    barChart->setAxisX(axis, barSeries);

    barChart->legend()->setVisible(true);
    barChart->legend()->setAlignment(Qt::AlignBottom);

    QtCharts::QChartView *barChartView = new QtCharts::QChartView(barChart);
    barChartView->setRenderHint(QPainter::Antialiasing);


    ui->chart_1->addWidget(barChartView);


    // Pie Chart
    pieSeries = new QtCharts::QPieSeries();
    pieSeries->append("Correct", 5);
    pieSeries->append("Left rotation", 1);
    pieSeries->append("Right rotation", 2);
    pieSeries->append("Proximity", 1);
    pieSeries->append("Height", 2);

    /*for (int i=0; i<5;i++) {
        QtCharts::QPieSlice *slice = pieSeries->slices().at(i);
        slice->setExploded();
        slice->setLabelVisible();
    }*/

    QtCharts::QChart *pieChart = new QtCharts::QChart();
    pieChart->addSeries(pieSeries);
    //pieChart->setTitle("Time division between each states");
    //pieChart->legend()->hide();
    pieChart->legend()->setAlignment(Qt::AlignRight);

    QtCharts::QChartView *pieChartView = new QtCharts::QChartView(pieChart);
    pieChartView->setRenderHint(QPainter::Antialiasing);

    ui->chart_2->addWidget(pieChartView);
}
