#include "checkposture.h"

CheckPosture::CheckPosture() {
    detector = dlib::get_frontal_face_detector();
    dlib::deserialize("C:/shape_predictor_68_face_landmarks.dat") >> shape_predictor;

    calibrate = false;
    calibrated = false;

    state = 0;
}

CheckPosture::~CheckPosture() {}

void CheckPosture::set_posture(std::vector<double> receivedPosture) { currentPosture = receivedPosture; }
void CheckPosture::set_calibratedPosture(std::vector<double> receivedPosture) { calibratedPosture = receivedPosture; }

void CheckPosture::calibratePosture()
{
    calibrate = true; calibrated = false;
}

void CheckPosture::checkFrame(cv::Mat &frame, int heightThreshold, int proximityThreshold, int angleThreshold)
{
    // Convert opencv image to dlib image
    dlib::array2d<dlib::bgr_pixel> dlib_image;
    assign_image(dlib_image, dlib::cv_image<dlib::bgr_pixel>(frame));

    // Detect faces
    std::vector<dlib::rectangle> detected_faces = detector(dlib_image);
    numberOfFaces = detected_faces.size();
    //qDebug() << "Number of faces: " << detected_faces.size();

    if (numberOfFaces == 1) {
        // Get face landmarks
        dlib::full_object_detection faceLandmarks = shape_predictor(dlib_image, detected_faces[0]);

        // Draw face landmarks
        for (unsigned j=0; j<68; j++) {
            cv::circle(frame, cv::Point(faceLandmarks.part(j).x(), faceLandmarks.part(j).y()), 2, cv::Scalar( 0, 0, 255), 1, cv::LINE_AA);
        }

        // Get face position
        std::vector<double> currentPosture = checkFacePosition(frame, faceLandmarks);

        // Set the current posture
        set_posture(currentPosture);

        // If calibrate == True, set the calibrated posture
        if (calibrate) {
            set_calibratedPosture(currentPosture);
            calibratedLandmarks = faceLandmarks;
            calibrated = true;
            calibrate = false;

            emit postureCalibrated();
        }

        // If calibrated, compare current posture with the calibrated one
        if (calibrated) {
            checkPosture(heightThreshold, proximityThreshold, angleThreshold);
        }
    }
}

std::vector<double> CheckPosture::checkFacePosition(cv::Mat frame, dlib::full_object_detection current_pose)
{
    // 2D image points obtained from DLIB
    std::vector<cv::Point2d> image_points;
    image_points.push_back( cv::Point2d( current_pose.part(30).x(), current_pose.part(30).y()) );   // Nose tip
    image_points.push_back( cv::Point2d( current_pose.part(8).x(), current_pose.part(8).y()) );     // Chin
    image_points.push_back( cv::Point2d( current_pose.part(36).x(), current_pose.part(36).y()) );   // Left eye left corner
    image_points.push_back( cv::Point2d( current_pose.part(45).x(), current_pose.part(45).y()) );   // Right eye right corner
    image_points.push_back( cv::Point2d( current_pose.part(48).x(), current_pose.part(48).y()) );   // Left Mouth corner
    image_points.push_back( cv::Point2d( current_pose.part(54).x(), current_pose.part(54).y()) );   // Right mouth corner

    // 3D model points.
    std::vector<cv::Point3d> model_points;
    model_points.push_back(cv::Point3d(0.0, 0.0, 0.0));             // Nose tip
    model_points.push_back(cv::Point3d(0.0, -330.0, -65.0));        // Chin
    model_points.push_back(cv::Point3d(-225.0, 170.0, -135.0));     // Left eye left corner
    model_points.push_back(cv::Point3d(225.0, 170.0, -135.0));      // Right eye right corner
    model_points.push_back(cv::Point3d(-150.0, -150.0, -125.0));    // Left Mouth corner
    model_points.push_back(cv::Point3d(150.0, -150.0, -125.0));     // Right mouth corner

    // Camera internals
    double focal_length = frame.cols; // Approximate focal length.
    cv::Point2d center = cv::Point2d(frame.cols/2,frame.rows/2);
    cv::Mat camera_matrix = (cv::Mat_<double>(3,3) << focal_length, 0, center.x, 0 , focal_length, center.y, 0, 0, 1);
    cv::Mat dist_coeffs = cv::Mat::zeros(4,1,cv::DataType<double>::type); // Assuming no lens distortion

    // Output rotation and translation
    cv::Mat rotation_vector; // Rotation in axis-angle form
    cv::Mat translation_vector;

    // Solve for pose
    // PNPRansac more stable than the PNP version
    cv::solvePnP(model_points, image_points, camera_matrix, dist_coeffs, rotation_vector, translation_vector);


    // Get Euler angles through the rotation_vector
    cv::Mat rotation_matrix, mtxR, mtxQ;
    cv::Rodrigues(rotation_vector, rotation_matrix);
    cv::Vec3d euler_angles = RQDecomp3x3(rotation_matrix, mtxR, mtxQ);

    /* As the camera, looking in the object position:
     *
     * Z -> X
     * |
     * v
     * Y
     *
     * x increases as the object goes to the right (user goes to the left)
     *  pitch = tilt looking to the left/right
     * y increases as the object goes down (user bends down)
     *  yaw = tilt looking up/down
     * z increases as the object get away from camera (user goes back)
     *  roll = tilt left down, but looking to the camera
     */

    /*
     * pitch = euler_angles[0];
     * yaw = euler_angles[1];
     * roll = euler_angles[2];
     * x = translation_vector.at<double>(0);
     * y = translation_vector.at<double>(1);
     * z = translation_vector.at<double>(2);
     */

    std::vector<double> facePosition(translation_vector);
    facePosition.push_back(euler_angles[0]);
    facePosition.push_back(euler_angles[1]);
    facePosition.push_back(euler_angles[2]);

    return facePosition;
}


/*
 * Proximidade
 *
 * Redução na altura
 *
 * Inclinação da face ??
 * Sugestão: alteração da altura + alteração da posição horizontal (eixo x) + inclinação da face
 *
 *
 *
 */
int CheckPosture::checkPosture(int heightThreshold, int proximityThreshold, int angleThreshold)
{
    int result = CORRECT_POSTURE;
    double heightTracker = 0;
    double proximityTracker = 0;
    double angleTracker = 0;

    if (numberOfFaces != 1) {
        result = COULD_NOT_DETECT;
    } else {
        // Incorrect posture due to the height
        heightTracker = ( currentPosture[1]-calibratedPosture[1] ) / heightThreshold ;
        if (currentPosture[1] > calibratedPosture[1] + heightThreshold) { result = LOW_HEIGHT; }

        // Incorrect posture due to the proximity of the camera
        proximityTracker = ( calibratedPosture[2]-currentPosture[2] ) / proximityThreshold ;
        if (currentPosture[2] < calibratedPosture[2] - proximityThreshold) { result = TOO_CLOSE; }

        // Rotation
        angleTracker = ( calibratedPosture[5]-currentPosture[5] ) / angleThreshold ;
        // Incorrect posture due to the rotation to the right
        if (currentPosture[5] < calibratedPosture[5] - angleThreshold) { result = ROLL_RIGHT; }

        // Incorrect posture due to the rotation to the left
        if (currentPosture[5] > calibratedPosture[5] + angleThreshold) { result = ROLL_LEFT; }
    }


    /*
     * PREPARE CLOCK TO COUNT. IF RESULT CHANGES, RESTART CLOCK
     * SET QCONNECT WITH THIS CLOCK, IF EMIT TIMEOUT, EMIT NOTIFICATION
     *
     */

    emit postureStatus(result, heightTracker, proximityTracker, angleTracker);

    return result;
}


