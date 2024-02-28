/*
 * Main image segmentation module. This module handles all
 * object detection, fiducial markers, and coordinate system
 * initialization.
 *
 * Object detection and fiducial markers were implemented using OpenCV
 *
 * Coordinate system was implemented based on mathematical literature,
 * full documentation can be found in our "Resources" page from our
 * final report
 *
 * AUTHORS: Bassam M., Victor Huynh
 *
 **/

#ifndef IMAGESEGMENTATION_H
#define IMAGESEGMENTATION_H

#include <QObject>
#include <QDebug>

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include "point.h"
#include "macros.h"

#define PI 3.14159265358979323846

//------------------------------------------------------------------------

class ImageSegmentation : public QObject
{
    Q_OBJECT

//------------------------------------------------------------------------

// Class Private Members
private:

    // coordinate system calibration members
    std::vector<std::vector<cv::Point>> calibrationPoints;
    std::vector<cv::Point> coilLocs = std::vector<cv::Point>(4);
    std::vector<double> FOV; // will contain physical x and y size

    // This dictionary holds all the ArUco marker templates.
    // Please note the DICT_4x4_50 number, as this means that each marker
    // is a square of 4x4 bits and a total of 50 markers are in this dictionary
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

    cv::Point origin;
    cv::Rect boundingROI;

    // This is the total number of image frames required for the initial
    // fiducal marker calibration
    int calibrationStepLimit = 20;
    int calibrationStep = 0;

    // Boolean to make sure system is calibrated
    bool calibrated = false;

    // Found by manually measuring the vertical distance between +y,-y coils. Also mm-scale.
    double distanceBetweenCoilPair = 53.0;
    double distancePerPixel = 1.0;
    double angle_XAxis = 0;

    // image segmentation calibration members
    cv::Mat currentImage;
    cv::Mat cleanImage;

    // Total number of image frames required for the initial
    // background image (particle detection) calibration
    int synthesizeImageStepLimit = 12;
    int synthesizeImageCounter = 0;

    // Boolean to make sure a background image was synthesized
    bool cleanImageIsSet = false;
    bool startedParticleDetectionCalibration = false;

    bool displayAxes = true;

    //-----------------------------------------------------

    // particle detection members, see the ImageSegmentation.cpp
    // for full documentation
    cv::Mat dilationKernel;
    cv::Mat subImage;
    cv::Mat cleanSubImage;
    cv::Mat subImage_Grayscale;
    cv::Mat subImage_GrayscaleFiltered;
    cv::Mat differenceImage;
    cv::Mat binaryThreshImage;

    // current particle location in pixel
    cv::Point currentParticleLoc_Pixel = cv::Point(-1, -1);

    // current particle location in mm
    Point currentParticleLoc_PixelMapped = Point(-1, -1);

    // parameters used for particle localization
    int dilation_size = 6;
    int dilation_type = cv::MORPH_CROSS;
    int filterThreshold = 70;
    int particleSizeMin = 20;
    int particleSizeMax = 100;

    void release_MatMemory();

//------------------------------------------------------------------------

// Class Public Functions (non-slots)/Members
public:

    // constructors and destructors
    explicit ImageSegmentation();
    ~ImageSegmentation();

//------------------------------------------------------------------------

// Class Signals
signals:

    // Qt signals to initial calibration process
    // signal to write and display to the GUI's operation log
    void send_OperationLogMsg(QString msg);

    // signal to stop the system calibration process
    void send_StopCalibration(bool fornow);

    // signal to start the particle movement step for system calibration
    void send_StartParticleDetectionCalibration();

    // signal to stop the particle movement step of system calibration
    void send_StopParticleDetectionCalibration();

    // signal to send field-of-view data to the GUI
    void send_UpdatedFOV(double width_FOV, double height_FOV);

//------------------------------------------------------------------------

// Class Public Slots (Functions)
public slots:

    // Function that starts the calibration process given the initial image.
    // has an internal counter that increments for each image
    cv::Mat perform_FullCalibration(cv::Mat inputImage);

    // Given an input image, this function segments to find the
    // ArUco fiducial markers. Note: this function is only used
    // in the initial calibration process, since the operating area
    // doesn't move during particle manipulation
    cv::Mat find_CoilsAruco(cv::Mat inputImage);

    // Function to average all collected calibration images
    // to return a "clean" background image
    void synthesize_CleanImage();

    // After a background image is set, this function subtracts
    // each incoming image from the background image and does some
    // filtering to return particle location
    cv::Mat detect_Particle(cv::Mat particleImage);

    // Checks if image segmentation (aruco and background image)
    // are both calibrated
    bool isCalibrated();

    // Resets the calibration (aruco and background image) process
    void reset_Calibration();

    // Finds the point of intersection between the line segments
    // made up of the +X -X and the +Y -Y axis respectively.
    void calibrate_Origin();

    // Helper method for coil calibration. See .cpp for full documentation
    void calibrate_CoilLocations();

    // Calculates FOV based on actual measured distance and the
    // detected distance between coils
    void calculate_DistancePerPixelAndFOV(int imgWidth, int imgHeight);

    // Draws coordinate system on the given image
    cv::Mat display_CoordinateSystem(cv::Mat inputImage);

    // Maps pixel coordinates onto a mm-coordinate system
    // made up of the coil locations. See .cpp for more documentation
    Point map_ToTrueCoordinates(Point object);

    // Mapes mm-coordinates onto pixel coordiantes
    Point unmap_FromTrueCoordinates(Point physicalPt);

    // Returns true if axis are being displayed
    bool isDisplayingAxes();

    // Displays axis if passed a true boolean
    void toggle_displayAxes(bool on);

    // sets parameters for particle detection
    void receive_ParticleDetectionParams(int, int, int);

    // Getter methods for various particle detection parameters
    int get_filterThreshold();
    int get_particleSizeMin();
    int get_particleSizeMax();

    // Returns an array of Points (in pixel coordinates) cooresponding
    // to the four coil locations. Array is always in the form of
    // +X, -X, +Y, -Y
    std::vector<Point> get_CoilLocations();

    // Getter methods for various paramters
    double get_DistancePerPixel();
    Point get_Origin();

    // Returns a rectangle (in pixel coordinates) around the
    // operating area rectangle
    cv::Rect get_BoundingROI();

    // Returns particle location in pixel coordinates
    Point get_CurrentParticleLoc_Pixel();

    // Returns particle location in mm coordinates
    Point get_CurrentParticleLoc_Physical();

    // Returns particle location in pixel coordinate,
    // mapped onto the coordinate system
    Point get_CurrentParticleLoc_PixelMapped();

    // Returns particle location in mm coordinates,
    // mapped to the coordinate system
    Point get_CurrentParticleLoc_PhysicalMapped();

//------------------------------------------------------------------------

};

#endif
