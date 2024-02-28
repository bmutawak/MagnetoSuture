/*
 *	OpenCvWorker source file.
 *
 *
 *	AUTHOR: Victor Huynh
 *
 */
//------------------------------------------------------------------------

#include "opencvworker.h"

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - constructor; establishes 1 connection with its
 * ImageSegmentation object regarding system calibration
 */
OpenCvWorker::OpenCvWorker(QObject *parent) : QObject(parent) {
    connect(&imageSegmenter, SIGNAL(send_StopCalibration(bool)), this, SLOT(toggle_calibrateFrames(bool)));
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - destructor; VideoCapture and VideoWriter objects are properly released
 * upon OpenCvWorker deletion
 */
OpenCvWorker::~OpenCvWorker() {
    if(capture->isOpened()) {
        capture->release();
        cv::destroyAllWindows();
    }
    delete capture;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - connects camera to the program
 */
void OpenCvWorker::loadStream(int cameraPort) {    

    // this line is for safety purposes
    toggle_streaming(false);

    this->cameraPort = cameraPort;

    // initialize VideoCapture
    cv::VideoCapture *oldCapture = capture;
    capture = new cv::VideoCapture(this->cameraPort);
    capture->set(CV_CAP_PROP_BUFFERSIZE, 10);
    capture->set(CV_CAP_PROP_FPS, 30);

    // read the 1st frame and quit streaming if this fails
    capture->read(frameOrig);
    if (frameOrig.empty()) {
        emit send_OperationLogMsg(ERROR_FORMAT("Empty frame encountered. Switching stream off."));
        toggle_streaming(false);
        capture->release();
        capture = nullptr;
        return;
    }

    originalFrameHeight = static_cast<int>(capture->get(CV_CAP_PROP_FRAME_HEIGHT));
    originalFrameWidth = static_cast<int>(capture->get(CV_CAP_PROP_FRAME_WIDTH));

    // frees memory from the previous VideoCapture (if there was one)
    if (oldCapture != nullptr && oldCapture->isOpened()) {
        oldCapture->release();
        oldCapture = nullptr;
    }

    toggle_streaming(true);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - captures, processes, and saves camera images;
 * directs output image to the GUI; key function of this class
 */
void OpenCvWorker::grabFrame() {

    // safety checks to prevent crashes or unintended streaming
    if (!streaming) {return;}
    if (!capture->isOpened()) {
        emit send_OperationLogMsg(ERROR_FORMAT("Issue with camera frame capture. Switching stream off."));
        toggle_streaming(false);
        return;
    }

    capture->read(frameOrig);

    // if an empty frame is read (i.e. cannot read the next frame), quit streaming
    if (frameOrig.empty()) {
        emit send_OperationLogMsg(ERROR_FORMAT("Empty frame encountered. Switching stream off."));
        toggle_streaming(false);
        return;
    }

    // process and send frame out for GUI display
    this->process();
    emit send_FrameforDisplay(&frameProcessed);
    frameOrig.release();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - manages image processing of captured frame by utilizing
 * the ImageSegmentation class
 */
void OpenCvWorker::process() {

    // case #1: system calibration
    if (calibrateFrames) {
        frameProcessed = imageSegmenter.perform_FullCalibration(frameOrig);
    }
    // case #2: particle detection case
    else if (segmentFrames) {
        frameProcessed = imageSegmenter.detect_Particle(frameOrig); // careful about memory on this one
    }
    // case #3: just displaying the calibrated axes
    else if (imageSegmenter.isCalibrated() && imageSegmenter.isDisplayingAxes()) {
        frameProcessed = imageSegmenter.display_CoordinateSystem(frameOrig);
    }
    // case #4: normal streaming
    else {
        frameProcessed = frameOrig;
    }

    // because OpenCV uses RGB and Qt uses BGR, a color channel conversion must be made
    cv::cvtColor(frameProcessed, frameProcessed, CV_RGB2BGR);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - returns whether the program is streaming camera frames
 */
bool OpenCvWorker::isStreaming() {
    return streaming;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - toggles camera streaming
 */
void OpenCvWorker::toggle_streaming(bool on) {
    streaming = on;

    // the signals here prompt MainWindow to resume or pause its timer for camera streaming
    if (streaming) {emit send_ResumeFrameDisplayPrompt();}
    else {emit send_PauseFrameDisplayPrompt();}
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - toggles using frames for system calibration
 */
void OpenCvWorker::toggle_calibrateFrames(bool on) {
    calibrateFrames = on;

    /* if toggling on, makes sure system calibration and particle detection
     * are mutually exclusive; also makes sure certain system parameters are reset
     * for this calibration */
    if (calibrateFrames) {
        segmentFrames = false;
        imageSegmenter.reset_Calibration();
    }
    else {
        emit send_OperationLogMsg("Calibration successfully stopped");
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - toggles using frames for particle detection
 */
void OpenCvWorker::toggle_segmentFrames(bool on) {

    // safety check; system cannot perform particle detection until after calibration
    if (!imageSegmenter.isCalibrated()) {
        segmentFrames = false;
        emit send_OperationLogMsg(ERROR_FORMAT("No clean image has been set."));
        return;
    }

    segmentFrames = on;

    /* if toggling on, makes sure particle detection and system calibration are
     * mutually exclusive */
    if (segmentFrames) {calibrateFrames = false;}
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - updates the save directory for recorded frames
 */
void OpenCvWorker::receive_SaveFolder(QString saveFolder) {
    this->saveFolder = saveFolder;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - determines the factor to resize images for proper
 * GUI screen display
 */
void OpenCvWorker::set_ResizeFactorUsingDisplayHeight(int displayHeight) {

    // makes sure frames can be streamed before calculating resize factor
    if (capture->isOpened()) {
        emit send_StreamOrientationParams(originalFrameWidth, originalFrameHeight,
                                          static_cast<double>(displayHeight)/originalFrameHeight);
    }
    else {
        emit send_OperationLogMsg(ERROR_FORMAT("Cannot display at proper resolution. Check if the camera is connected."));
    }
}

//------------------------------------------------------------------------
