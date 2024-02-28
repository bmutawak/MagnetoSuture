/*
 *	Point header file.
 *
 *
 *	AUTHOR: Victor Huynh
 *
 */
//------------------------------------------------------------------------

#ifndef POINT_H
#define POINT_H

#include <QObject>
#include <QPointF>

#include <opencv2/opencv.hpp>

//------------------------------------------------------------------------

/*
 * Class Description:
 * Point and PathPoint are data structures used to represent particle location
 * and the components of the delivery path. Point is the base class, PathPoint
 * is the child class. Currently, there is no functional distinction between PathPoint
 * and Point other than name. This can be expanded on in future iterations of the project.
 */

//------------------------------------------------------------------------
class Point : public QPointF {
//------------------------------------------------------------------------

// Class Public Functions (non-slots)/Members
public:

    // various constructors (for convenience purposes)
    Point() : QPointF() {}
    Point(double x, double y) : QPointF(x,y) {}
    Point(cv::Point cvPt) : QPointF(cvPt.x,cvPt.y) {}
    Point(QPointF ptF) : QPointF(ptF.x(), ptF.y()) {}

    // string representation
    QString toString() {
        return "(" + QString::number(this->x()) + "," + QString::number(this->y()) + ")";
    }

    // used to calculate the distance between 2 points
    static double computeEuclideanDist(Point p1, Point p2) {
        return sqrt(pow((p1.x() - p2.x()), 2.0) + pow((p1.y() - p2.y()), 2.0));
    }
};

//------------------------------------------------------------------------
class PathPoint : public Point {
//------------------------------------------------------------------------

// Class Public Functions (non-Slots)/Members
public:

    // various constructors (for convenience purposes)
    PathPoint() : Point() {}
    PathPoint(double x, double y) : Point(x, y) {}
    PathPoint(cv::Point cvPt) : Point(cvPt) {}
    PathPoint(Point pt) : Point(pt.x(), pt.y()) {}
    PathPoint(QPointF ptF) : Point(ptF) {}
};

//------------------------------------------------------------------------

// this struct conveniently holds the physical and pixel location data of a path point
typedef struct _PathPtStruct {

    // pixel coordinates on the screen
    PathPoint pixelScreen;

    // pixel coordinates from the native camera image
    PathPoint pixelNative;

    // physical coordinates
    PathPoint physical;

    // denotes whether the path point is a checkpoint (see controlmodule class)
    bool isCheckpoint = false;

} PathPtStruct;

//------------------------------------------------------------------------

#endif // POINT_H
