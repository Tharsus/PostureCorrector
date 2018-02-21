#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv2/opencv.hpp>

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
    alertsound->setMedia(QUrl("C://Windows//media//Windows Notify System Generic.wav"));

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/myappico.png"));
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

    for (unsigned long i=0; i < detected_faces.size(); i++) {
        full_object_detection shape = shape_model(dlib_image, detected_faces[i]);

        shapes.push_back(shape);

        if (calibrate) {
            calibrated_pose = shape;
            ui->checkBox->setChecked(true);
            calibrate=false;
            calibrated = true;
            //for (unsigned j=0; j<68; j++) {
                //cout << calibrated_pose.part(j).x() << ", " << calibrated_pose.part(j).y() << endl;
            //}
        }

        for (unsigned j=0; j<68; j++) {
            circle(frame, Point(shape.part(j).x(), shape.part(j).y()), 2, Scalar( 0, 0, 255), 1, LINE_AA);

            if (calibrated) {
                circle(frame, Point(calibrated_pose.part(j).x(), calibrated_pose.part(j).y()), 2, Scalar( 0, 255, 0), 1, LINE_AA);
            }
            if (calibrated) {
                posture_score(shape);
                face_rotation(shape);
                if (right_pose) {
                    if (compare(shape) != 0) {
                        right_pose = false;
                        //sound_alert();
                        //tray_notification(true);
                    }
                } else {
                    if (!right_pose) {
                        if (compare(shape) == 0) {
                            right_pose = true;
                            tray_notification(false);
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
        pose_problems = 1;
    }
    return pose_problems;
}

int MainWindow::posture_score(full_object_detection current_pose)
{
    int score = 0;

    //detect horizontal and vertical movement
    int horizontal_average = 0, vertical_average = 0, horizontal_average_calibrated = 0, vertical_average_calibrated = 0;
    for (unsigned j=0; j<68; j++) {
        horizontal_average += current_pose.part(j).x();
        horizontal_average_calibrated += calibrated_pose.part(j).x();

        vertical_average += current_pose.part(j).y();
        vertical_average_calibrated += calibrated_pose.part(j).y();
    }
    horizontal_average = horizontal_average / 68;
    horizontal_average_calibrated = horizontal_average_calibrated / 68;

    vertical_average = vertical_average / 68;
    vertical_average_calibrated = vertical_average_calibrated / 68;

    //cout << "Corrente: " << horizontal_average << "\tCalibrado: " << horizontal_average_calibrated << endl;

    //detect rotation

    return score;
}

void MainWindow::sound_alert()
{
    if (alertsound->state() == QMediaPlayer::PlayingState) {
        alertsound->setPosition(0);
    } else {
        alertsound->play();
    }
}

void MainWindow::tray_notification(boolean activate)
{
    //implementar redirecionamento do tray quando clicar
    //implementar mensagem quando hover

    if (activate) {
        trayIcon->show();
        trayIcon->showMessage("Warning", "Your posture is not adequate.", QSystemTrayIcon::Warning);
        trayIcon->messageClicked();
    } else {
        trayIcon->hide();
    }
}

void MainWindow::face_rotation(full_object_detection current_pose)
{
    // Read input image
    cv::Mat im = frame;

    // 2D image points. If you change the image, you need to change vector
    std::vector<cv::Point2d> image_points;
    image_points.push_back( cv::Point2d( current_pose.part(30).x(), current_pose.part(30).y()) );    // Nose tip
    image_points.push_back( cv::Point2d( current_pose.part(8).x(), current_pose.part(8).y()) );    // Chin
    image_points.push_back( cv::Point2d( current_pose.part(36).x(), current_pose.part(36).y()) );     // Left eye left corner
    image_points.push_back( cv::Point2d( current_pose.part(45).x(), current_pose.part(45).y()) );    // Right eye right corner
    image_points.push_back( cv::Point2d( current_pose.part(48).x(), current_pose.part(48).y()) );    // Left Mouth corner
    image_points.push_back( cv::Point2d( current_pose.part(54).x(), current_pose.part(54).y()) );    // Right mouth corner

    // 3D model points.
    std::vector<cv::Point3d> model_points;
    model_points.push_back(cv::Point3d(0.0f, 0.0f, 0.0f));               // Nose tip
    model_points.push_back(cv::Point3d(0.0f, -330.0f, -65.0f));          // Chin
    model_points.push_back(cv::Point3d(-225.0f, 170.0f, -135.0f));       // Left eye left corner
    model_points.push_back(cv::Point3d(225.0f, 170.0f, -135.0f));        // Right eye right corner
    model_points.push_back(cv::Point3d(-150.0f, -150.0f, -125.0f));      // Left Mouth corner
    model_points.push_back(cv::Point3d(150.0f, -150.0f, -125.0f));       // Right mouth corner

    // Camera internals
    double focal_length = im.cols; // Approximate focal length.
    Point2d center = cv::Point2d(im.cols/2,im.rows/2);
    cv::Mat camera_matrix = (cv::Mat_<double>(3,3) << focal_length, 0, center.x, 0 , focal_length, center.y, 0, 0, 1);
    cv::Mat dist_coeffs = cv::Mat::zeros(4,1,cv::DataType<double>::type); // Assuming no lens distortion

    //cout << "Camera Matrix " << endl << camera_matrix << endl ;
    // Output rotation and translation
    cv::Mat rotation_vector; // Rotation in axis-angle form
    cv::Mat translation_vector;

    // Solve for pose
    // PNPRansac more stable than the PNP version
    cv::solvePnP(model_points, image_points, camera_matrix, dist_coeffs, rotation_vector, translation_vector);


    // Project a 3D point (0, 0, 1000.0) onto the image plane.
    // We use this to draw a line sticking out of the nose

    std::vector<Point3d> nose_end_point3D;
    std::vector<Point2d> nose_end_point2D;
    nose_end_point3D.push_back(Point3d(0,0,1000.0));

    projectPoints(nose_end_point3D, rotation_vector, translation_vector, camera_matrix, dist_coeffs, nose_end_point2D);


    /*for(unsigned int i=0; i < image_points.size(); i++)
    {
        circle(im, image_points[i], 3, Scalar(0,0,255), -1);
    }*/

    cv::line(im,image_points[0], nose_end_point2D[0], cv::Scalar(255,0,0), 2);

    //cout << "Rotation Vector: " << rotation_vector << endl;
    //cout << "Translation Vector: " << translation_vector << endl;

    //cout <<  nose_end_point2D << endl;






    Mat rotation_matrix, mtxR, mtxQ;
    cv::Rodrigues(rotation_vector, rotation_matrix);
    cv::Vec3d euler_angles = RQDecomp3x3(rotation_matrix, mtxR, mtxQ);

    cout << "\npitch: " << euler_angles[0] << endl; // cima/baixo
    cout << "yaw: " << euler_angles[1] << endl; // esquerda/direita
    cout << "roll: " << euler_angles[2] << endl; // rotação

}
