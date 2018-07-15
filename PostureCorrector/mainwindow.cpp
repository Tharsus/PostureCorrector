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

    ui->tabWidget->hide();

    // Present loading animation
    QMovie *movie = new QMovie("C://Users//Thársus//Desktop//ajax-loader.gif");
    ui->loading->setMovie(movie);
    ui->loading->setAlignment(Qt::AlignCenter);
    movie->start();


    // Prepare variables
    timer = new QTimer(this);

    ui->pushButton_Start->setEnabled(true);
    ui->pushButton_Stop->setEnabled(false);
    ui->pushButton_Calibrate->setEnabled(false);

    ui->heightBar->hide();
    ui->proximityBar->hide();
    ui->rotationBar->hide();

    ui->label_4->hide();
    ui->label_5->hide();
    ui->label_6->hide();

    numberOfCalibrations = 0;

    right_pose = true;
    pause=false;

    alertsound = new QMediaPlayer();
    alertsound->setMedia(QUrl("C://Windows//media//Windows Notify System Generic.wav"));

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/myappico.png"));

    QObject::connect(this, SIGNAL(calibrateButton_clicked(void)), &checkPosture, SLOT(calibratePosture()));
    QObject::connect(&checkPosture, SIGNAL(postureCalibrated()), this, SLOT(checkPosture_calibrated(void)));

    QObject::connect(&checkPosture, SIGNAL(postureStatus(int, double, double, double)), this, SLOT(processPosture(int, double, double, double)));
    QObject::connect(&checkPosture, SIGNAL(postureState(int)), this, SLOT(processState(int)));


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

        db.fillChartVariables(days, alertsInEachState, durationInEachState);

        /*std::vector<unsigned int> status;
        std::vector<QDateTime> dateTime;

        db.selectAllFromDatabase(status, dateTime);

        set_postureRecords(status, dateTime);
        qDebug() << "Number of Alerts: " << numberOfAlerts;
        qDebug() << "Duration of alert: " << durationOfAlert;


        qDebug() << "end of reading";*/
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





    // hide loading widget and show tabWIdget
    ui->loading->hide();
    ui->tabWidget->show();
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

        if (numberOfCalibrations > 0) {
            ui->heightBar->setEnabled(true);
            ui->proximityBar->setEnabled(true);
            ui->rotationBar->setEnabled(true);

            ui->label_4->setEnabled(true);
            ui->label_5->setEnabled(true);
            ui->label_6->setEnabled(true);
        }

        connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
        timer->start(50); //in miliseconds: 50ms = 0.05s => will grab a frame every 50ms = 20 frames/second
    }
}

void MainWindow::on_pushButton_Stop_clicked()
{
    ui->pushButton_Start->setEnabled(true);
    ui->pushButton_Stop->setEnabled(false);
    ui->pushButton_Calibrate->setEnabled(false);

    ui->heightBar->setValue(0);
    ui->proximityBar->setValue(0);
    ui->rotationBar->setValue(0);

    ui->heightBar->setEnabled(false);
    ui->proximityBar->setEnabled(false);
    ui->rotationBar->setEnabled(false);

    ui->label_4->setEnabled(false);
    ui->label_5->setEnabled(false);
    ui->label_6->setEnabled(false);

    if (numberOfCalibrations > 0) {
        db.insertIntoDatabase(PAUSE);
        tray_notification(false, "");
        right_pose=true;
        pause = true;
    }

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
        ui->heightBar->show();
        ui->proximityBar->show();
        ui->rotationBar->show();

        ui->label_4->show();
        ui->label_5->show();
        ui->label_6->show();

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

    // Flip horizontally so that image shown feels like watching yourself in a mirror
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

void MainWindow::processPosture(int currentPosture, double heightTracker, double proximityTracker, double angleTracker)
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

        timeInEachState[index] += chronometer.elapsed();
        chronometer.restart();
        previousPosture = currentPosture;
    }

    if (heightTracker < 0) { ui->heightBar->setValue(0); }
    else if (heightTracker > 1) { ui->heightBar->setValue(100); }
    else {
        int ratio = trunc(heightTracker * 100);
        ui->heightBar->setValue( ratio );
    }

    if (proximityTracker < 0) { ui->proximityBar->setValue(0); }
    else if (proximityTracker > 1) { ui->proximityBar->setValue(100); }
    else {
        int ratio = trunc(proximityTracker * 100);
        ui->proximityBar->setValue( ratio );
    }

    int ratio = abs(trunc(angleTracker * 100));
    if (ratio > 100) { ui->rotationBar->setValue(100); }
    else {
        ui->rotationBar->setValue( ratio );
    }

    /*for (int i=0; i<5; i++) {
        qDebug() << "Status " << i << " = " << timeInEachState[i]/1000;
    }
    qDebug() << endl;*/
}

void MainWindow::processState(int state)
{
    qDebug() << "TO DB: " << state;
    if (state == CORRECT_POSTURE) {
        right_pose = true;
        pause = false;
        tray_notification(false, "");
        if (!db.insertIntoDatabase(state)) {}
    }
    else if (state == LOW_HEIGHT) {
        right_pose = false;
        pause = false;
        tray_notification(true, "Too low!");
        if (!db.insertIntoDatabase(state)) {}
    }
    else if (state == TOO_CLOSE) {
        right_pose = false;
        pause = false;
        tray_notification(true, "Too close!");
        if (!db.insertIntoDatabase(state)) {}
    }
    else if (state == ROLL_RIGHT) {
        right_pose = false;
        pause = false;
        tray_notification(true, "Rolling to the right!");
        if (!db.insertIntoDatabase(state)) {}
    }
    else if (state == ROLL_LEFT) {
        right_pose = false;
        pause = false;
        tray_notification(true, "Rolling to the left!");
        if (!db.insertIntoDatabase(state)) {}
    }
    else if (state == COULD_NOT_DETECT) {
        right_pose = false;
        pause = false;
        sound_alert();
        tray_notification(true, "Not able to detect!");
        if (!db.insertIntoDatabase(state)) {}
    }


    /*if (right_pose || pause) {
        // Roll left
        if (state == LOW_HEIGHT) {
            right_pose = false;
            pause = false;
            //sound_alert();
            tray_notification(true, "Too low!");
            if (!db.insertIntoDatabase(state)) {}
        }
        // Roll right
        else if (state == TOO_CLOSE) {
            right_pose = false;
            pause = false;
            tray_notification(true, "Too close!");
            if (!db.insertIntoDatabase(state)) {}
        }
        // low height
        else if (state == ROLL_RIGHT) {
            right_pose = false;
            pause = false;
            tray_notification(true, "Rolling to the right!");
            if (!db.insertIntoDatabase(state)) {}
        }
        // too close
        else if (state == ROLL_LEFT) {
            right_pose = false;
            pause = false;
            tray_notification(true, "Rolling to the left!");
            if (!db.insertIntoDatabase(state)) {}
        }

        if (pause) {
            pause = false;
            if (!db.insertIntoDatabase(state)) {}
        }
    } else if (!right_pose) {
        if (state == CORRECT_POSTURE) {
            right_pose = true;
            pause = false;
            tray_notification(false, "");
            if (!db.insertIntoDatabase(state)) {}
        }
    }*/
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
    set0 = new QtCharts::QBarSet("Low Height");
    set1 = new QtCharts::QBarSet("Too Close");
    set2 = new QtCharts::QBarSet("Rolling Right");
    set3 = new QtCharts::QBarSet("Rolling Left");
    for (unsigned i=0; i<alertsInEachState.size(); i++) {
        *set0 << alertsInEachState[i][0];
        *set1 << alertsInEachState[i][1];
        *set2 << alertsInEachState[i][2];
        *set3 << alertsInEachState[i][3];
    }

    QtCharts::QBarSeries *barSeries = new QtCharts::QBarSeries();
    barSeries->append(set0);
    barSeries->append(set1);
    barSeries->append(set2);
    barSeries->append(set3);

    QtCharts::QChart *barChart = new QtCharts::QChart();
    barChart->addSeries(barSeries);
    barChart->setTitle("Number of Ocurrencies");
    barChart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

    QStringList categories;
    for (unsigned i=0; i<days.size(); i++) {
        QString day_month = QString::number(days[i].day()) + "/" + QString::number(days[i].month());
        categories << day_month;
    }
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

    int duration[5] = {0};
    for (unsigned i=0; i<durationInEachState.size(); i++) {
        for (unsigned j=0; j<5; j++) {
            duration[j] += durationInEachState[i][j];
        }
    }
    pieSeries->append("Correct", duration[0]);
    pieSeries->append("Low Height", duration[1]);
    pieSeries->append("Too Close", duration[2]);
    pieSeries->append("Rolling Right", duration[3]);
    pieSeries->append("Rolling Left", duration[4]);

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
