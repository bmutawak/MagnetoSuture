#include "ControlModule.h"
#include "ControlModule.h"

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - constructor
 */
ControlModule::ControlModule()
{

}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - updates class's internal path data through
 * discretization and updating path starting point
 */
bool ControlModule::setup_PathTraversal(std::vector<PathPointMarker*> pathCheckPoints,
                                        double totalPathDistance,
                                        double deviationTolerance,
                                        double interpolationDistance) {

    // safety checks
    if (static_cast<int>(pathCheckPoints.size()) == 0) {
        send_OperationLogMsg(ERROR_FORMAT("Cannot set up path for travseral. Path not drawn"));
        return false;
    }
    else if (deviationTolerance <= 0.0) {
        send_OperationLogMsg(ERROR_FORMAT("Cannot set up path for travseral. Invalid deviation tolerance: " + QString::number(deviationTolerance)));
        return false;
    }
    else if (interpolationDistance <= 0.0) {
        send_OperationLogMsg(ERROR_FORMAT("Cannot set up path for travseral. Invalid interpolation distance." + QString::number(interpolationDistance)));
        return false;
    }
    else {
        this->deviationTolerance = deviationTolerance;
        this->interpolationDistance = interpolationDistance;
    }

    // form the delivery path and initialize the first target point
    if (!deliveryPath.empty()) {deliveryPath.clear();}

    for (int i = currentPathCheckpoint; i < static_cast<int>(pathCheckPoints.size()); i++) {
        deliveryPath.push_back(*pathCheckPoints.at(static_cast<unsigned long long>(i))->data);
    }

    // initialize position of the target path point
    targetPathPtData = deliveryPath.at(static_cast<unsigned long long>(0));
    targetPathPtIndex = 0;

    // assign other class members
    this->totalPathDistance = totalPathDistance;
    if (remainingPathDistance == 0.0) {
        remainingPathDistance = this->totalPathDistance;
    }
    retryCounter = 0;

    // discretize the delivery path; safer to perform this last in the function
    this->discretize_Path();
    return true;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - discretizes delivery path via path point insertion
 * based on the interpolation distance
 */
void ControlModule::discretize_Path() {

    totalNumPathPts = static_cast<int>(deliveryPath.size());

    bool fullyDiscretized;
    double distBetween2Pts;

    PathPtStruct currentPt;
    PathPtStruct previousPt;
    PathPtStruct intermediatePt;

    /* this segment of code performs as:
     * while the distance between 2 path points are greater than the interpolation distance,
     * insert a path point in the middle of the 2 points and check the distance between
     * them again; repeat this process until all points are within the interpolation distance
     * of each other */
    do {
        fullyDiscretized = true;

        for (int i = 1; i < totalNumPathPts; i++) {
            currentPt = deliveryPath.at(static_cast<unsigned long long>(i));
            previousPt = deliveryPath.at(static_cast<unsigned long long>(i-1));
            distBetween2Pts = Point::computeEuclideanDist(currentPt.physical, previousPt.physical);

            if (distBetween2Pts > interpolationDistance) {
                fullyDiscretized = false;

                intermediatePt.pixelScreen = PathPoint((currentPt.pixelScreen.x() + previousPt.pixelScreen.x())/2,
                                                     (currentPt.pixelScreen.y() + previousPt.pixelScreen.y())/2);

                intermediatePt.pixelNative = PathPoint((currentPt.pixelNative.x() + previousPt.pixelNative.x())/2,
                                                     (currentPt.pixelNative.y() + previousPt.pixelNative.y())/2);

                intermediatePt.physical = PathPoint((currentPt.physical.x() + previousPt.physical.x())/2,
                                                     (currentPt.physical.y() + previousPt.physical.y())/2);

                deliveryPath.insert(deliveryPath.begin() + i, intermediatePt);
                totalNumPathPts += 1;
                i -= 1;
            }
        }

    } while(!fullyDiscretized);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - computes particle deviation from target path point
 * and determines if the next translation should be allowed
 */
bool ControlModule::allow_NextTranslation(Point particleLoc) {

    // calculate the actual deviation of the particle from the target path point
    actualDeviation = Point::computeEuclideanDist(particleLoc, targetPathPtData.physical);

    // if the particle is at the final point on the path, the delivery operation is complete
    if (((totalNumPathPts - targetPathPtIndex - 1) <= 0) && (actualDeviation <= deviationTolerance)) {
        emit send_StopOpPrompt();
        emit send_OperationLogMsg("Operation complete!");
        return false;
    }
    /* if the particle is within the deviation tolerance of the target path point, update that
     * point and other internal data */
    else if (actualDeviation <= deviationTolerance) {

        /* update the path checkpoint if appropriate; key to allowing the user to modify
         * the delivery path mid-operation */
        if (targetPathPtData.isCheckpoint) {
            emit send_CheckpointPassed(currentPathCheckpoint);
            currentPathCheckpoint += 1;
        }

        targetPathPtIndex += 1;
        targetPathPtData = deliveryPath.at(static_cast<unsigned long long>(targetPathPtIndex));

        remainingPathDistance -= Point::computeEuclideanDist(targetPathPtData.physical,
                                                             deliveryPath.at(static_cast<unsigned long long>(targetPathPtIndex-1)).physical);
        if (remainingPathDistance < 0.0) {
            remainingPathDistance = 0.0;
        }

        retryCounter = 0;
    }
    // if the particle is not close enough to the target path point, increment the "unsuccessfull attempts" counter
    else {
        retryCounter += 1;
    }

    /* if the "unsuccessful attempts" counter reaches too high, it is sign that somethign is wrong
     * with the operation and that it should be stopped for safety */
    if (retryCounter >= 40) {
        emit send_StopOpPrompt();
        send_OperationLogMsg(ERROR_FORMAT("Translations have been repeatedly unsuccessful. Stopping particle delivery for safety."));
        return false;
    }

    return true;
}


//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - retrieves the target path point data
 */
PathPtStruct ControlModule::get_targetPathPointData() {
    return targetPathPtData;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - // retrieves the current actual deviation of the
 * particle from the target path point
 */
double ControlModule::get_ActualDeviation() {
    return actualDeviation;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - retrieves the remaining path distance for the
 * particle to traverse
 */
double ControlModule::get_RemainingPathDistance() {
    return remainingPathDistance;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - retrieves the operation progress as a percentage
 */
double ControlModule::get_OperationProgress() {
    return 100 * (1.0 - (remainingPathDistance/totalPathDistance));
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - resets the class's member data; always called at
 * the end of an operation
 */
void ControlModule::reset_Data() {

    deliveryPath.clear();
    currentPathCheckpoint = 0;
    targetPathPtIndex = 0;
    retryCounter = 0;
    totalNumPathPts = 0;
    interpolationDistance = 0.0;
    deviationTolerance = 0.0;
    actualDeviation = 0.0;
    totalPathDistance = 0.0;
    remainingPathDistance = 0.0;
}
