/*
 *	ControlModule header file.
 *
 *
 *	AUTHOR: Victor Huynh
 *
 */
//------------------------------------------------------------------------

#ifndef CONTROLMODULE_H
#define CONTROLMODULE_H

#include <QObject>
#include <QDebug>

#include "pathpointmarker.h"
#include "point.h"
#include "macros.h"

//------------------------------------------------------------------------

/*
 * Class Description:
 * Control Module processes particle location and path traversal data during
 * a delivery operation from start to finish, allowing for automated delivery.
 * It also contains features to ensure safe delivery/stoppage. The class is
 * written such that the user can modify a path mid-operation.
 */

//------------------------------------------------------------------------

class ControlModule : public QObject
{
    Q_OBJECT

//------------------------------------------------------------------------

// Class Public Functions (non-slots)/Members
public:

    // constructor
    explicit ControlModule();

//------------------------------------------------------------------------

// Class Private Members
private:

    std::vector<PathPtStruct> deliveryPath; // contains delivery path points' data
    PathPtStruct targetPathPtData; // the data of the current target path point

    int currentPathCheckpoint = 0; // used to keep track of the most recently passed path marker
    int targetPathPtIndex = 0; // index of the target path point within the delivery path
    int retryCounter = 0; // number of attempted translations to reach the target path point
    int totalNumPathPts = 0; // total number of path points in the current delivery path

    double interpolationDistance = 0.0; // max distance between 2 points; used to discretize the path
    double deviationTolerance = 0.0; // max acceptable distance that the particle can "deviate" from the target path point
    double actualDeviation = 0.0; // actual distance of the particle from the target path point
    double totalPathDistance = 0.0; // total path distance based on all path markers
    double remainingPathDistance = 0.0; // remaining path distance based on all passed path markers

//------------------------------------------------------------------------

// Class Signals
signals:

    // signal to write and display to the GUI's operation log
    void send_OperationLogMsg(QString msg);

    // signal to note that a path marker has been passed by the particle
    void send_CheckpointPassed(int checkpointIndex);

    // signal to end the operation
    void send_StopOpPrompt();

//------------------------------------------------------------------------

// Class Public Slots (Functions)
public slots:

    // updates class's internal path data through discretization and updating path starting point
    bool setup_PathTraversal(std::vector<PathPointMarker*> pathCheckpoints,
                             double totalPathDistance,
                             double deviationTolerance,
                             double interpolationDistance);

    // discretizes delivery path via path point insertion based on the interpolation distance
    void discretize_Path();

    // computes particle deviation from target path point and determines if the next translation should be allowed
    bool allow_NextTranslation(Point particleLoc);

    // retrieves the target path point data
    PathPtStruct get_targetPathPointData();

    // retrieves the current actual deviation of the particle from the target path point
    double get_ActualDeviation();

    // retrieves the remaining path distance for the particle to traverse
    double get_RemainingPathDistance();

    // retrieves the operation progress as a percentage
    double get_OperationProgress();

    // resets the class's member data; always called at the end of an operation
    void reset_Data();

//------------------------------------------------------------------------

};

#endif // CONTROLMODULE_H
