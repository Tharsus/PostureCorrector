#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
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
    alertsound->setMedia(QUrl("C://Windows//media//Alarm10.wav"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_Start_clicked()
{
    cap.open(0);

    if(!cap.isOpened()){
        cout << "not able to open camera" << endl;
    } else {
        cout << "camera is opened" << endl;
        ui->pushButton_Start->setEnabled(false);
        ui->pushButton_Stop->setEnabled(true);
        ui->pushButton_Calibrate->setEnabled(true);

        connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
        timer->start(50); //in miliseconds: 50ms = 0.05s => will grab a frame every 50ms = 20 frames/second

        detector = get_frontal_face_detector();
        deserialize("C:/shape_predictor_68_face_landmarks.dat") >> shape_model;
    }
}

void MainWindow::on_pushButton_Stop_clicked()
{
    ui->pushButton_Start->setEnabled(true);
    ui->pushButton_Stop->setEnabled(false);
    ui->pushButton_Calibrate->setEnabled(false);

    disconnect(timer, SIGNAL(timeout()),this, SLOT(update_window()));
    cap.release();
    cout << "camera is closed" << endl;

    //fill display with a black screen
    Mat image = Mat::zeros(frame.size(),CV_8UC3);
    show_frame(image);
}

void MainWindow::on_pushButton_Calibrate_clicked()
{
    calibrate = true;
}

void MainWindow::update_window()
{
    cap >> frame;
    //cv::resize(frame, frame, cv::Size(), 1.0/4, 1.0/4);

    array2d<bgr_pixel> dlib_image;
    assign_image(dlib_image, cv_image<bgr_pixel>(frame));

    std::vector<dlib::rectangle> detected_faces = detector(dlib_image);
    //cout << "Number of detected faces: " << detected_faces.size() << endl;

    std::vector<full_object_detection> shapes;

    //int rect_x, rect_y, rect_w, rect_h;

    for (unsigned long i=0; i < detected_faces.size(); i++) {
        full_object_detection shape = shape_model(dlib_image, detected_faces[i]);

        shapes.push_back(shape);

        if (calibrate) {
            calibrated_pose = shape;
            ui->checkBox->setChecked(true);
            calibrate=false;
            calibrated = true;
            for (unsigned j=0; j<68; j++) {
                //cout << calibrated_pose.part(j).x() << ", " << calibrated_pose.part(j).y() << endl;
            }
        }

        for (unsigned j=0; j<68; j++) {
            circle(frame, Point(shape.part(j).x(), shape.part(j).y()), 2, Scalar( 0, 0, 255), 1, LINE_AA);

            if (calibrated) {
                circle(frame, Point(calibrated_pose.part(j).x(), calibrated_pose.part(j).y()), 2, Scalar( 0, 255, 0), 1, LINE_AA);
            }
            if (calibrated) {
                if (right_pose) {
                    if (compare(shape) != 0) {
                        right_pose = false;
                        sound_alert();
                    }
                } else {
                    if (!right_pose) {
                        if (compare(shape) == 0) {
                            right_pose = true;
                        }
                    }
                }
            }
        }
    }
    show_frame(frame);
}

void MainWindow::show_frame(Mat &image)
{
    //resize image to the size of label_displayFace
    Mat resized_image = image.clone();

    int width_of_label = ui->label_displayFace->width();
    int height_of_label = ui->label_displayFace->height();

    Size size(width_of_label, height_of_label);
    cv::resize(image, resized_image, size);

    //change color map so that it can be displayed in label_displayFace
    cvtColor(resized_image, resized_image, CV_BGR2RGB);
    ui->label_displayFace->setPixmap(QPixmap::fromImage(QImage(resized_image.data, resized_image.cols, resized_image.rows, QImage::Format_RGB888)));
}

int MainWindow::compare(full_object_detection current_pose)
{
    int pose_problems = 0;
    if (current_pose.part(8).y() > calibrated_pose.part(8).y()+10 || current_pose.part(8).y() < calibrated_pose.part(8).y()-10) {
        //mySystemTrayIcon = new QSystemTrayIcon;
        pose_problems = 1;
    }
    return pose_problems;
}

void MainWindow::sound_alert()
{
    /*QMediaPlayer *alert = new QMediaPlayer();
    alert->setMedia(QUrl(path));
    if (alert->state() == QMediaPlayer::PlayingState) {
        cout << "AQUI" << endl;
    }*/
    if (alertsound->state() == QMediaPlayer::PlayingState) {
        alertsound->setPosition(0);
    } else {
        alertsound->play();
    }
}
