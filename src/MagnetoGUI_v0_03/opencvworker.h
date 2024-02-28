/*
 *	OpenCvWorker header file.
 *
 *
 *	AUTHOR: Victor Huynh
 *
 */
//------------------------------------------------------------------------

#ifndef OPENCVWORKER_H
#define OPENCVWORKER_H

#include <QObject>
#include <QImage>
#include <QDebug>
#include <QDir>
#include <QDate>
#include <QSound>
#include <QFileInfo>

#include <opencv2/opencv.hpp>
#include "point.h"
#include "macros.h"
#include "ImageSegmentation.h"

//------------------------------------------------------------------------

/*
 * Class Description:
 * OpenCvWorker controls all camera image streaming functionality and manages
 * image processing at a higher level.
 */

//------------------------------------------------------------------------

class OpenCvWorker : public QObject
{
    Q_OBJECT

//------------------------------------------------------------------------

// Class Public Functions (non-slots)/Members
public:

    // constructor and destructor
    explicit OpenCvWorker(QObject *parent = nullptr);
    ~OpenCvWorker();

    // object of ImageSegmentation class; performs all image processing functionality
    ImageSegmentation imageSegmenter;

//------------------------------------------------------------------------

// Class Private Members
private:
    cv::VideoCapture *capture = nullptr; // pointer to VideoCapture object, for streaming frames
    cv::Mat frameOrig; // original frame grabbed from camera
    cv::Mat frameProcessed; // frame after all iamge processing

    QString saveFolder; // filepath to save recorded frames

    bool streamLoaded = false; // denotes camera linked status
    bool streaming = false; // denotes camera streaming status
    bool calibrateFrames = false; // denotes whether the frame will be used for system calibration
    bool segmentFrames = false; // denotes whether particle detection is turned on

    int cameraPort = 1; // camera port number
    int originalFrameHeight = 0; // height of the native camera image
    int originalFrameWidth = 0; // width of the native camera image

    // determines how captured frames are to be processed; key image processing function
    void process();

//------------------------------------------------------------------------

// Class Signals
signals:

    // signal to pause GUI streaming display
    void send_PauseFrameDisplayPrompt();

    // signal to resume GUI streaming display
    void send_ResumeFrameDisplayPrompt();

    // signal to send a (processed) frame to the system for display
    void send_FrameforDisplay(cv::Mat *frame);

    // signal to send data to the GUI for proper image resizing
    void send_StreamOrientationParams(int originalFrameWidth, int originalFrameHeight, double resizeFactor);

    // signal to write and display to the GUI's operation log
    void send_OperationLogMsg(QString msg);

//------------------------------------------------------------------------

// Class Public Slots (Functions)
public slots:

    // connects camera to the program (actual implementation)
    void loadStream(int cameraPort);

    /* captures, processes, and saves camera images; directs output image to the GUI;
     * key function of this class */
    void grabFrame();

    // returns whether camera streaming is going on
    bool isStreaming();

    // toggles camera streaming
    void toggle_streaming(bool on);

    // toggles using captured frames for system calibration
    void toggle_calibrateFrames(bool on);

    // toggles using captured frames for particle detection
    void toggle_segmentFrames(bool on);

    // updates the directory to save a video file
    void receive_SaveFolder(QString saveFolder);

    // determines the factor to resize images for proper GUI screen display
    void set_ResizeFactorUsingDisplayHeight(int displayHeight);

//------------------------------------------------------------------------

};

#endif // OPENCVWORKER_H
