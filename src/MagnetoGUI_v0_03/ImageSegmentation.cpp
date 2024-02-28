/*
 * This file implements all the function prototypes
 * specified in ImageSegmentation.cpp.
 *
 * AUTHORS: Bassam M., Victor Huynh
 *
 */

#include "ImageSegmentation.h"

//------------------------------------------------------------------------

/*
 * Constructor. Initializes the coil calibration array (which is a vector
 * containing vectors of particle locations at each image frame collected)
 * and initializes the kernel used for particle detection (see below)
 */
ImageSegmentation::ImageSegmentation()
{
    calibrationPoints = std::vector<std::vector<cv::Point>>(static_cast<unsigned long long>(calibrationStepLimit));
    dilationKernel = getStructuringElement(dilation_type, cv::Size(2*dilation_size+1, 2*dilation_size+1), cv::Point(dilation_size, dilation_size));
}

//------------------------------------------------------------------------

/*
 * Destructor
 */
ImageSegmentation::~ImageSegmentation()
{

}

//------------------------------------------------------------------------

/*
 * Initial function to begin the process of calibrating the coil marker locations
 * (aruco) and particle detection
 */
cv::Mat ImageSegmentation::perform_FullCalibration(cv::Mat inputImage) {

    // Stores the current image
    inputImage.copyTo(currentImage);

    // If particle detection has not yet begun, set the boolean to be
    // true and emit signal to begin image averagins
    if (!startedParticleDetectionCalibration) {
        startedParticleDetectionCalibration = true;
        emit send_StartParticleDetectionCalibration();
    }

    // If coil markers are not calibrated yet, send the same input image
    // to find the coil marker locations. See below for more details
    if (!calibrated) {inputImage = find_CoilsAruco(inputImage);}

    // If all calibration is complete, send a signal to the GUI
    // to stop sending an input image
    if (calibrated && cleanImageIsSet){
        emit send_StopCalibration(false);
    }

    // Draw the coordinate system onto the input image and return
    inputImage = display_CoordinateSystem(inputImage);
    return inputImage;
}

//------------------------------------------------------------------------
/*
 *	Detect coils based on aruco markers.
 *	Marker ids are: 10 17 34 37 for +x -x +y -y fiducial markers
 *  Note: This method is only called in the initial calibration process
 */
cv::Mat ImageSegmentation::find_CoilsAruco(cv::Mat inputImage) {

    // If the coil markers are already calibrated, don't do anything
    if (calibrated) {return inputImage;}

    std::vector<int> detectedMarkerIds;
    std::vector<std::vector<cv::Point2f>> detectedMarkerCorners;

    // Calls ArUco markers to segment an input image to find markers based on the
    // dictionary intially created. Returns the detected marker corners in an array vector
    // and a vector of Ids corresponding to the marker ID at each index of the detected
    // marker corners
    cv::aruco::detectMarkers(inputImage, dictionary, detectedMarkerCorners, detectedMarkerIds);
    std::vector<std::vector<cv::Point2f>> orderedMarkers(4);

    // Loops through and organizes markers into an array based on the detected features.
    // The orderedMarkers array is allways in the format: +x -x +y -y
    for (int i = 0; i < static_cast<int>(detectedMarkerIds.size()); i++) {

        // For +X Coil marker
        if (detectedMarkerIds.at(static_cast<unsigned long long>(i)) == 10) {
            orderedMarkers.at(0) = detectedMarkerCorners.at(static_cast<unsigned long long>(i));
        }
        // For -X coil marker
        else if (detectedMarkerIds.at(static_cast<unsigned long long>(i)) == 17) {
            orderedMarkers.at(1) = detectedMarkerCorners.at(static_cast<unsigned long long>(i));
        }
        // For +Y Coil marker
        else if (detectedMarkerIds.at(static_cast<unsigned long long>(i)) == 34) {
            orderedMarkers.at(2) = detectedMarkerCorners.at(static_cast<unsigned long long>(i));
        }
        // For -Y Coil marker
        else if (detectedMarkerIds.at(static_cast<unsigned long long>(i)) == 37) {
            orderedMarkers.at(3) = detectedMarkerCorners.at(static_cast<unsigned long long>(i));
        }
    }

    cv::Point2f firstCorner, secondCorner, plusXCoil, minusXCoil, plusYCoil, minusYCoil;

    // Next is to get the sides of the fiducial markers touching the inside "box" of our
    // operating area. It's important to note that the initial orientation of these markers
    // are physically fixed, so as long as the aruco marker structure is placed correctly,
    // then we can find the marker sides touching the inside operating area "box" easily.
    // This is simply getting the correct marker corners, and finding the midway point.

    //For +x
    try {
        firstCorner = orderedMarkers.at(0).at(0);	//this is the top-left corner of +x marker
        secondCorner = orderedMarkers.at(0).at(3);	//this is the bottom-left corner of +x marker

        // Now create a point midway between the two corners
        plusXCoil = cv::Point2f(((secondCorner.x - firstCorner.x) / 2) + firstCorner.x, ((secondCorner.y - firstCorner.y) / 2) + firstCorner.y);
    } catch (std::exception e) {qDebug() << "Cant find +x";}

    //For -X
    try {
        firstCorner = orderedMarkers.at(1).at(1);	//this is the top-left corner of -x marker
        secondCorner = orderedMarkers.at(1).at(2);	//this is the bottom-left corner of -x marker

        // Now create a point midway between the two corners
        minusXCoil = cv::Point2f(((secondCorner.x - firstCorner.x) / 2) + firstCorner.x, ((secondCorner.y - firstCorner.y) / 2) + firstCorner.y);
    } catch (std::exception e) {qDebug() << "Cant find -x";}

    //For +y
    try {
        firstCorner = orderedMarkers.at(2).at(3);	//this is the bottom-left corner of +y marker
        secondCorner = orderedMarkers.at(2).at(2);	//this is the bottom-right corner of +y marker

        // Now create a point midway between the two corners
        plusYCoil = cv::Point2f(((secondCorner.x - firstCorner.x) / 2) + firstCorner.x, ((secondCorner.y - firstCorner.y) / 2) + firstCorner.y);
    } catch (std::exception e) {qDebug() << "Cant find +y";}

    //For -y
    try {
        firstCorner = orderedMarkers.at(3).at(0);	//this is the bottom-left corner of +x marker
        secondCorner = orderedMarkers.at(3).at(1);	//this is the bottom-right corner of +x marker

        // Now create a point midway between the two corners
        minusYCoil = cv::Point2f(((secondCorner.x - firstCorner.x) / 2) + firstCorner.x, ((secondCorner.y - firstCorner.y) / 2) + firstCorner.y);
    } catch (std::exception e) {qDebug() << "Cant find -y";}


    // Add the coil marker location points to the calibration array,
    calibrationPoints.at(static_cast<unsigned long long>(calibrationStep)) = {plusXCoil, minusXCoil, plusYCoil, minusYCoil};

    // Set our current marker locations
    coilLocs = {plusXCoil, minusXCoil, plusYCoil, minusYCoil};

    // If we've reached the maximum calibration limit
    if (calibrationStep >= (calibrationStepLimit-1)) {

        // Set our calibration boolean to true
        calibrated = true;

        // Call a function to average the location of all previously stored coil locations
        // from calibrationPoints
        calibrate_CoilLocations();

        // Find the origin based on the intersection of line segments created from connecting
        // opposing coils
        calibrate_Origin();

        // Calculate FOV based on detected (in pixel coordinates) and actual (in mm) coil locations
        calculate_DistancePerPixelAndFOV(inputImage.cols, inputImage.rows);

        // Computes the X-AXIS line segment angle from the horizontal. This is used to ensure
        // consistent coordinate system mapping regardless of the rotation of coil structure.
        // Below is an exaggerated case where the X-axis coils are rotated around 70 degrees from the horizon
        //
        //                           (+X Coil marker)
        //                          /
        //                         /
        //                        /
        //                       / Theta
        //            ------------------------ (Horizontal)
        //                     /
        //                    /
        //                   /
        //                  /
        //                  (-X Coil Marker)
        //
        double adjacent = abs(coilLocs.at(0).x - origin.x);
        double hypontenuse = std::sqrt((std::pow(coilLocs.at(0).x - origin.x, 2)) + (std::pow((coilLocs.at(0).y - origin.y), 2)));
        angle_XAxis = std::acos(adjacent / hypontenuse);
    }
    else {

        // increment step counter
        calibrationStep++;
    }

    return inputImage;
}

//------------------------------------------------------------------------

// Function to create a clean image from the initial calibration process
void ImageSegmentation::synthesize_CleanImage() {

    cv::Mat temp;
    cvtColor(currentImage,temp,cv::COLOR_BGR2GRAY);
    temp.convertTo(temp,CV_64FC1);

    // If this is not the first frame in the calibration process,
    // add the current frame into an image that is the summation of all
    // previous frames. Otherwise, set the cleanImage to be the current frame
    if (!cleanImage.empty()) {cleanImage += temp;}
    else {temp.copyTo(cleanImage);}

    // Release memory and increment the counter
    temp.release();
    synthesizeImageCounter += 1;

    // If we've reached the calibration limit,
    if (synthesizeImageCounter >= synthesizeImageStepLimit) {
        // Divide the summation image (of all calibration frames) by the maximum
        // to average the image out. This mitigates the particle's presence from view
        // and is a good background image for particle detection
        cleanImage.convertTo(cleanImage,CV_8UC3,1./synthesizeImageStepLimit);
        cleanImageIsSet = true;
        emit send_StopParticleDetectionCalibration();
    }
}

//------------------------------------------------------------------------

// Function to detect the particle location given the current frame and background image.
// The current frame is subtracted from a background image, and after some filtering the
// detected particle location can be found
cv::Mat ImageSegmentation::detect_Particle(cv::Mat particleImage) {
    if (cleanImage.empty() || particleImage.empty()) {
        emit send_OperationLogMsg(ERROR_FORMAT("Cannot detect particle. Clean or input image is empty."));
        return particleImage;
    }

    // Crop the whole image to just the operating area rectangle
    subImage = particleImage(boundingROI);
    if (cleanSubImage.empty()) {cleanSubImage = cleanImage(boundingROI);}

    // convert the image to grayscale
    cvtColor(subImage, subImage_Grayscale, cv::COLOR_BGR2GRAY);

    // Blur the image to reduce noise
    cv::bilateralFilter(subImage_Grayscale, subImage_GrayscaleFiltered, 5, 75, 75);

    // Subtract from background image
    cv::subtract(cleanSubImage, subImage_GrayscaleFiltered, differenceImage); // take difference between the two images

    // Dilate slightly
    cv::dilate(differenceImage, differenceImage, dilationKernel); //dilate result

    // Binarize image to reduce noise
    cv::threshold(differenceImage, binaryThreshImage, this->filterThreshold, 255, cv::THRESH_BINARY); // threshold image to convert to binary

    // Apply a contouring method to draw regions of interest around possible marker locations left after
    // binarization
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> empty;
    cv::findContours(binaryThreshImage, contours, empty, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // find contours

    cv::Rect contourBox;
    cv::Rect particleBox;

    // Loop through each detected feature
    for (std::vector<cv::Point> contour : contours) {

        // Create a rectangle around the detected feature cluster
        contourBox = cv::boundingRect(contour);

        // Make sure the detected feature is larger than the minimum particle
        // size and smaller than the maximum
        if ((static_cast<int>(contour.size()) > particleSizeMin) &&
                (static_cast<int>(contour.size()) < particleSizeMax)) {

            // This is just to make sure we are always getting the largest
            // possible feature within our bounds
            if (contourBox.area() > particleBox.area()) {
                particleBox = contourBox;
            }
        }
    }

    // Create a box around our chosen feature location
    particleBox = cv::Rect(particleBox.tl()+boundingROI.tl(),particleBox.br()+boundingROI.tl());

    // Get the pixel coordinates of the particle to be the center of the box
    if (particleBox.area() != 0.0) {
        currentParticleLoc_Pixel = cv::Point((particleBox.x + (particleBox.width/2)), (particleBox.y + (particleBox.height/2)));
    }

    // draw axis onto image
    if (displayAxes) {particleImage = display_CoordinateSystem(particleImage);}

    // Add a marker for particle location, draw a rectangle around particle
    cv::drawMarker(particleImage, currentParticleLoc_Pixel, cv::Scalar(0, 0, 255), cv::MARKER_CROSS, 7, 1);
    cv::rectangle(particleImage, particleBox, cv::Scalar(0, 255, 0), 2);
    release_MatMemory();

    // return the image with particle drawn
    return particleImage;
}

//------------------------------------------------------------------------

// Returns true if calibration is complete
bool ImageSegmentation::isCalibrated() {
    return this->calibrated && this->cleanImageIsSet;
}

//------------------------------------------------------------------------

// triggered at the start of every calibration, resets parameters
// so that calibration can begin
void ImageSegmentation::reset_Calibration() {
    calibrated = false;
    calibrationStep = 0;
    distancePerPixel = 0.0;
    FOV.clear();

    cleanImageIsSet = false;
    startedParticleDetectionCalibration = false;
    synthesizeImageCounter = 0;
    cleanImage.release();
    cleanSubImage.release();
    cleanImage.convertTo(cleanImage,CV_64FC1);

    emit send_OperationLogMsg("Calibration reset successful");
}

//------------------------------------------------------------------------
/*
 *	Calculates the intersecting point between the +-X line and +-Y line, sets as origin
 *	Math was found from this website: https://www.geeksforgeeks.org/program-for-point-of-intersection-of-two-lines/
 *
 *  Important to note that this implementation is convoluted and can be simplified easily for
 *  a future iteration of the project
 */
void ImageSegmentation::calibrate_Origin() {

    // X-axis line
    double a1 = coilLocs[1].y - coilLocs[0].y;
    double b1 = coilLocs[0].x - coilLocs[1].x;
    double c1 = a1 * (coilLocs[0].x) + b1 * (coilLocs[0].y);

    // Y-axis line
    double a2 = coilLocs[3].y - coilLocs[2].y;
    double b2 = coilLocs[2].x - coilLocs[3].x;
    double c2 = a2 * (coilLocs[2].x) + b2 * (coilLocs[2].y);

    double determinant = a1 * b2 - a2 * b1;

    int x = static_cast<int>(std::floor((b2*c1 - b1 * c2) / determinant));
    int y = static_cast<int>(std::floor((a1*c2 - a2 * c1) / determinant));

    origin = cv::Point(x, y);
}

//------------------------------------------------------------------------
/*
 *	Used during coil location calibration. Because there is some slight deviation in coil locations per image,
 *	an average is taken out of the total number of calibration points. The averaged coil locations can then
 *	be return from getCoilLocs()
 */
void ImageSegmentation::calibrate_CoilLocations() {
    if (!calibrated) {
        emit send_OperationLogMsg(ERROR_FORMAT("Calibration was not finished. Cannot set coil locations."));
        return;
    }

    int averageX;
    int averageY;

    cv::Point boundingBoxTopLeft(-1,-1);
    cv::Point boundingBoxBottomRight(-1,-1);

    // Loops through each coil
    for (int i = 0; i < 4; i++) {
        averageX = 0;
        averageY = 0;

        // Sums all the X and Y coordinates (pixels) of that detected coil marker location
        for (int j = 0; j < calibrationStepLimit; j++) {
            averageX += calibrationPoints.at(static_cast<unsigned long long>(j)).at(static_cast<unsigned long long>(i)).x;
            averageY += calibrationPoints.at(static_cast<unsigned long long>(j)).at(static_cast<unsigned long long>(i)).y;
        }

        // Divides by the calibration limit
        averageX = averageX / (calibrationStepLimit);
        averageY = averageY / (calibrationStepLimit);

        // Set the averaged coil marker
        coilLocs.at(static_cast<unsigned long long>(i)) = cv::Point(averageX, averageY);

        // construct the bounding ROI rectangle around the center operating area (petri-dish)
        if (i == 0) {
            boundingBoxTopLeft = coilLocs.at(static_cast<unsigned long long>(i));
            boundingBoxBottomRight = coilLocs.at(static_cast<unsigned long long>(i));
        }
        else {
            // Sets the boundaries of bounding area based on the coordinates of each
            // coil marker to account for rotation of coil structure
            if (coilLocs.at(static_cast<unsigned long long>(i)).x < boundingBoxTopLeft.x) {
                boundingBoxTopLeft.x = coilLocs.at(static_cast<unsigned long long>(i)).x;
            }
            else if (coilLocs.at(static_cast<unsigned long long>(i)).x > boundingBoxBottomRight.x) {
                boundingBoxBottomRight.x = coilLocs.at(static_cast<unsigned long long>(i)).x;
            }

            if (coilLocs.at(static_cast<unsigned long long>(i)).y < boundingBoxTopLeft.y) {
                boundingBoxTopLeft.y = coilLocs.at(static_cast<unsigned long long>(i)).y;
            }
            else if (coilLocs.at(static_cast<unsigned long long>(i)).y > boundingBoxBottomRight.y) {
                boundingBoxBottomRight.y = coilLocs.at(static_cast<unsigned long long>(i)).y;
            }
        }
    }

    // returns bounding box
    boundingROI = cv::Rect(boundingBoxTopLeft,boundingBoxBottomRight);

}

//------------------------------------------------------------------------
/*
 *	Given the location of each coil, this will return the distance per pixel.
 *	Return value is in mm-scale
 */
void ImageSegmentation::calculate_DistancePerPixelAndFOV(int imgWidth, int imgHeight) {
    if (!calibrated) {
        emit send_OperationLogMsg(ERROR_FORMAT("Cannot calculate the distance per pixel until the coil locations are set."));
        return;
    }

    // Calculates the total number of pixels between the +Y and -Y coils
    double numPixelsBetweenCoilPair = sqrt(pow((coilLocs[2].x - coilLocs[3].x), 2) + pow((coilLocs[2].y - coilLocs[3].y), 2));

    // Divides the actual (mm) distance by the total number of pixels to get Distance per Pixel
    distancePerPixel = distanceBetweenCoilPair / numPixelsBetweenCoilPair;

    // Multiplies by the length and width of image to get vertical and horizontal FOV
    FOV.push_back(imgWidth*distancePerPixel);
    FOV.push_back(imgHeight*distancePerPixel);

    emit send_UpdatedFOV(FOV[0], FOV[1]);
}

//------------------------------------------------------------------------

/*
 * Draws coordinate system onto the image.
 */
cv::Mat ImageSegmentation::display_CoordinateSystem(cv::Mat inputImage) {
    cv::line(inputImage, coilLocs.at(0), coilLocs.at(1), cv::Scalar(0,0,255), 1, CV_AA); // X-axis line
    cv::line(inputImage, coilLocs.at(2), coilLocs.at(3), cv::Scalar(0,255,0), 1, CV_AA); // Y-axis line

    cv::circle(inputImage, coilLocs.at(0), 1, cv::Scalar(0,255,255), 3); // +X Coil Circle
    cv::putText(inputImage, "+X", coilLocs.at(0), CV_FONT_HERSHEY_TRIPLEX, 1, cv::Scalar(255,255,0), 1, CV_AA);

    cv::circle(inputImage, coilLocs.at(1), 1, cv::Scalar(0,255,255), 3); // -X Coil Circle
    cv::putText(inputImage, "-X", coilLocs.at(1), CV_FONT_HERSHEY_TRIPLEX, 1, cv::Scalar(255,255,0), 1, CV_AA);

    cv::circle(inputImage, coilLocs.at(2), 1, cv::Scalar(0,255,255), 3); // +Y Coil Circle
    cv::putText(inputImage, "+Y", coilLocs.at(2), CV_FONT_HERSHEY_TRIPLEX, 1, cv::Scalar(255,255,0), 1, CV_AA);

    cv::circle(inputImage, coilLocs.at(3), 1, cv::Scalar(0,255,255), 3); // -Y Coil Circle
    cv::putText(inputImage, "-Y", coilLocs.at(3), CV_FONT_HERSHEY_TRIPLEX, 1, cv::Scalar(255,255,0), 1, CV_AA);

    if (calibrated) {
        cv::circle(inputImage, origin, 1, cv::Scalar(0,255,255), 5); // Origin Circle
        cv::rectangle(inputImage, boundingROI.tl(), boundingROI.br(), cv::Scalar(255,0,0), 1, CV_AA); // bounding ROI
    }

    return inputImage;
}

//------------------------------------------------------------------------
/*
 *	Maps pixel coordinates to a coordinate system based on the axis of the coils
 *	Accounts for rotation and translation of the axis.
 *	Math can be found here: https://www.stewartcalculus.com/data/CALCULUS%20Early%20Transcendentals/upfiles/RotationofAxes.pdf
 */
Point ImageSegmentation::map_ToTrueCoordinates(Point unmappedPt) {
    if (!calibrated) {
        emit send_OperationLogMsg(ERROR_FORMAT("Cannot map since the axes have not been calibrated."));
        return cv::Point();
    }

    double xPos_FromOrigin = unmappedPt.x() - origin.x;
    double yPos_FromOrigin = origin.y - unmappedPt.y(); // because of image coords; this is basically -1*(ptObject.y - origin.y)

    double xPos_True = xPos_FromOrigin*std::cos(angle_XAxis) + yPos_FromOrigin*std::sin(angle_XAxis);
    double yPos_True = -xPos_FromOrigin*std::sin(angle_XAxis) + yPos_FromOrigin*std::cos(angle_XAxis);

    return Point(xPos_True, yPos_True);
}

//------------------------------------------------------------------------

/*
 * Maps actual (mm) coordinates from the coordinate system to pixels on image
 */
Point ImageSegmentation::unmap_FromTrueCoordinates(Point mappedPt) {
    if (!calibrated) {
        emit send_OperationLogMsg(ERROR_FORMAT("System not calibrated. Cannot perform unmapping."));
        return Point(0,0);
    }

    double yPos_FromOrigin = mappedPt.y()*std::cos(angle_XAxis) + mappedPt.x()*std::sin(angle_XAxis);
    double yPos_Image = origin.y - yPos_FromOrigin;

    double xPos_FromOrigin = mappedPt.x()*std::cos(angle_XAxis) - mappedPt.y()*std::sin(angle_XAxis);
    double xPos_Image = xPos_FromOrigin + origin.x;

    return Point(xPos_Image,yPos_Image);
}

//------------------------------------------------------------------------

/*
 * Returns true if axis are currently being displayed
 */
bool ImageSegmentation::isDisplayingAxes() {
    return displayAxes;
}

//------------------------------------------------------------------------

/*
 * Toggles axis to be displayed
 */
void ImageSegmentation::toggle_displayAxes(bool on) {
    displayAxes = on;
}

//------------------------------------------------------------------------

/*
 * sets parameters for particle detection
 */
void ImageSegmentation::receive_ParticleDetectionParams(int filterThreshold, int minSize, int maxSize) {
    this->filterThreshold = filterThreshold;
    this->particleSizeMin = minSize;
    this->particleSizeMax = maxSize;
}

//------------------------------------------------------------------------
/*
 *	Given a calibrated coil system, this returns the averaged coil locations
 *  in mm.
 */
std::vector<Point> ImageSegmentation::get_CoilLocations() {
    if (calibrated) {
        Point trueCoilLoc_XPlus = map_ToTrueCoordinates(coilLocs.at(0))*distancePerPixel;
        Point trueCoilLoc_XNeg = map_ToTrueCoordinates(coilLocs.at(1))*distancePerPixel;
        Point trueCoilLoc_YPlus = map_ToTrueCoordinates(coilLocs.at(2))*distancePerPixel;
        Point trueCoilLoc_YNeg = map_ToTrueCoordinates(coilLocs.at(3))*distancePerPixel;

        return std::vector<Point>{trueCoilLoc_XPlus,trueCoilLoc_XNeg,trueCoilLoc_YPlus,trueCoilLoc_YNeg};
    }
    else {
        emit send_OperationLogMsg(ERROR_FORMAT("System not calibrated. Cannot retrieve coil locations."));
        return std::vector<Point>();
    }
}

//------------------------------------------------------------------------

/*
 * Returns the computed distance per pixel
 */
double ImageSegmentation::get_DistancePerPixel() {
    if (!calibrated) {
        emit send_OperationLogMsg(ERROR_FORMAT("System not calibrated. Cannot retrieve valid distance per pixel."));
        return -1.0;
    }

    return distancePerPixel;
}

//------------------------------------------------------------------------
/*
 *	Returns the origin (in pixel coordinates) if system is properly calibrated.
 */
Point ImageSegmentation::get_Origin() {
    if (calibrated) {
        return Point(origin.x, origin.y);
    }
    else {
        emit send_OperationLogMsg(ERROR_FORMAT("Origin point was not calibrated."));
        return Point(-1,-1);
    }
}

//------------------------------------------------------------------------

/*
 * Returns bounding box around the petri-dish ROI
 */
cv::Rect ImageSegmentation::get_BoundingROI() {
    return this->boundingROI;
}

//------------------------------------------------------------------------

// retrieves the filter threshold value
int ImageSegmentation::get_filterThreshold() {
    return filterThreshold;
}

//------------------------------------------------------------------------

// retrieves the particle min size value
int ImageSegmentation::get_particleSizeMin() {
    return particleSizeMin;
}

//------------------------------------------------------------------------

// retrieves the particle max size value
int ImageSegmentation::get_particleSizeMax() {
    return particleSizeMax;
}

//------------------------------------------------------------------------

/*
 * Returns current particle location in pixel coordinates
 */
Point ImageSegmentation::get_CurrentParticleLoc_Pixel() {
    return Point(currentParticleLoc_Pixel.x, currentParticleLoc_Pixel.y);
}

//------------------------------------------------------------------------

/*
 * Returns current particle location in mm coordinates
 */
Point ImageSegmentation::get_CurrentParticleLoc_Physical() {
    return Point(currentParticleLoc_Pixel.x*distancePerPixel,
                 currentParticleLoc_Pixel.y*distancePerPixel);
}

//------------------------------------------------------------------------

/*
 * Returns particle location in pixel cooridnates, mapped to coordinate system
 */
Point ImageSegmentation::get_CurrentParticleLoc_PixelMapped() {
    return this->map_ToTrueCoordinates(currentParticleLoc_Pixel);
}

//------------------------------------------------------------------------

/*
 * Returns current particle location in mm, mapped to coordinate system
 */
Point ImageSegmentation::get_CurrentParticleLoc_PhysicalMapped() {
    return this->map_ToTrueCoordinates(currentParticleLoc_Pixel)*distancePerPixel;
}

//------------------------------------------------------------------------

/*
 * Used to free memory for image members involved with particle detection;
 * allows the program to run more efficiently during particle detection
 * image processing
 */
void ImageSegmentation::release_MatMemory() {
    subImage.release();
    subImage_Grayscale.release();
    subImage_GrayscaleFiltered.release();
    differenceImage.release();
    binaryThreshImage.release();
}
