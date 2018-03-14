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

    calibrate = false;
    calibrated = false;

    right_pose = true;

    alertsound = new QMediaPlayer();
    alertsound->setMedia(QUrl("C://Windows//media//Windows Notify System Generic.wav"));

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/myappico.png"));

    QObject::connect(&checkPosture, SIGNAL(badPosture(int)), this, SLOT(notify(int)));

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
        qDebug() << "opened";
        std::vector<unsigned int> status;
        std::vector<QDateTime> dateTime;

        db.selectAllFromDatabase(status, dateTime);

        set_postureRecords(status, dateTime);
        qDebug() << numberOfAlerts;
        qDebug() << durationOfAlert;


        qDebug() << "end of reading";
    } else { qDebug() << "error"; }







    /*
     * TESTING HOW TO PLOT A BARCHART
     */

    QtCharts::QBarSet *set0 = new QtCharts::QBarSet("Number of Ocurrencies");
    *set0 << 50 << 65 << 45 << 30 << 15 << 7;

    QtCharts::QBarSeries *barSeries = new QtCharts::QBarSeries();
    barSeries->append(set0);

    QtCharts::QChart *barChart = new QtCharts::QChart();
    barChart->addSeries(barSeries);
    barChart->setTitle("Simple barchart example");
    barChart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);

    QStringList categories;
    categories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";
    QtCharts::QBarCategoryAxis *axis = new QtCharts::QBarCategoryAxis();
    axis->append(categories);
    barChart->createDefaultAxes();
    barChart->setAxisX(axis, barSeries);

    barChart->legend()->setVisible(true);
    barChart->legend()->setAlignment(Qt::AlignBottom);

    QtCharts::QChartView *barChartView = new QtCharts::QChartView(barChart);
    barChartView->setRenderHint(QPainter::Antialiasing);


    ui->chart_1->addWidget(barChartView);


    /*
     * TESTING HOW TO PLOT A PIECHART
     */

    QtCharts::QPieSeries *pieSeries = new QtCharts::QPieSeries();
    pieSeries->append("Ok", 1);
    pieSeries->append("Left", 2);
    pieSeries->append("Right", 3);
    pieSeries->append("Close", 4);
    pieSeries->append("Down", 5);

    QtCharts::QPieSlice *slice = pieSeries->slices().at(1);
    slice->setExploded();
    slice->setLabelVisible();
    slice->setPen(QPen(Qt::darkGreen, 2));
    slice->setBrush(Qt::green);

    QtCharts::QChart *pieChart = new QtCharts::QChart();
    pieChart->addSeries(pieSeries);
    pieChart->setTitle("Simple piechart example");
    pieChart->legend()->hide();

    QtCharts::QChartView *pieChartView = new QtCharts::QChartView(pieChart);
    pieChartView->setRenderHint(QPainter::Antialiasing);

    ui->chart_2->addWidget(pieChartView);

}

MainWindow::~MainWindow()
{
    delete ui;

    // include end and pause of software
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
    calibrate = true;
    ui->pushButton_Calibrate->setEnabled(false);
    checkPosture.set_calibrateTrue();
}

void MainWindow::on_rotationThreshold_valueChanged(int value) { ui->rotationDisplay->setValue(value); }
void MainWindow::on_heightThreshold_valueChanged(int value) { ui->heightDisplay->setValue(value); }
void MainWindow::on_proximityThreshold_valueChanged(int value) { ui->proximityDisplay->setValue(value); }


void MainWindow::update_window()
{
    cap >> frame;

    checkPosture.checkFrame(frame);

    if (calibrate && checkPosture.postureCalibrated()) {
        ui->checkBox->setChecked(true);
        ui->pushButton_Calibrate->setText("Recalibrate");
        ui->pushButton_Calibrate->setEnabled(true);
        calibrate=false;
        calibrated=true;
    }

    if (calibrated) {
        checkPosture.checkPosture(ui->heightThreshold->value(), ui->proximityThreshold->value(), ui->rotationThreshold->value());
    }
    show_frame(frame);
}

void MainWindow::show_frame(cv::Mat &image)
{
    //resize image to the size of label_displayFace
    cv::Mat resized_image = image.clone();

    int width_of_label = ui->label_displayFace->width();
    int height_of_label = ui->label_displayFace->height();

    cv::Size size(width_of_label, height_of_label);
    cv::resize(image, resized_image, size);

    //change color map so that it can be displayed in label_displayFace
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

void MainWindow::notify(int postureCheck)
{
    qDebug() << "Status: " << postureCheck;
    if (right_pose) {
        // Roll left
        if (postureCheck == LOW_HEIGHT) {
            right_pose = false;
            //sound_alert();
            tray_notification(true, "Too low!");
            if (!db.insertIntoDatabase(postureCheck)) {}
        }
        // Roll right
        else if (postureCheck == TOO_CLOSE) {
            right_pose = false;
            tray_notification(true, "Too close!");
            if (!db.insertIntoDatabase(postureCheck)) {}
        }
        // low height
        else if (postureCheck == ROLL_RIGHT) {
            right_pose = false;
            tray_notification(true, "Rolling to the right!");
            if (!db.insertIntoDatabase(postureCheck)) {}
        }
        // too close
        else if (postureCheck == ROLL_LEFT) {
            right_pose = false;
            tray_notification(true, "Rolling to the left!");
            if (!db.insertIntoDatabase(postureCheck)) {}
        }
    } else {
        if (postureCheck == CORRECT_POSTURE) {
            right_pose = true;
            tray_notification(false, "");
            if (!db.insertIntoDatabase(postureCheck)) {}
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
