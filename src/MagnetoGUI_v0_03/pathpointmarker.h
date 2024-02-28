/*
 *	PathPointMarker header file.
 *
 *
 *	AUTHOR: Victor Huynh
 *
 */
//------------------------------------------------------------------------

#ifndef PATHPOINTMARKER_H
#define PATHPOINTMARKER_H

#include <QWidget>
#include <QObject>
#include <QDebug>
#include <QPainter>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsSimpleTextItem>
#include <QStyle>

#include <opencv2/opencv.hpp>
#include "point.h"
#include "macros.h"

//------------------------------------------------------------------------

/*
 * Class Description:
 * PathPointMarker is the graphical representation of a user-drawn path point
 * on the screen. It is similar to a node of a linked list in that it contains
 * references to previous and next objects (other PathPointMarkers and the connecting
 * lines).
 */

//------------------------------------------------------------------------

class PathPointMarker : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT

//------------------------------------------------------------------------

// Class Private Members
private:

    PathPointMarker *prevMkr; // pointer to previous path marker
    PathPointMarker *nextMkr; // pointer to next path marker

    QGraphicsLineItem *prevPtLine; // pointer to previous connecting line
    QGraphicsLineItem *nextPtLine; // pointer to next connecting line

    QGraphicsSimpleTextItem *textLabel = new QGraphicsSimpleTextItem(); // pointer to text label associated with the path marker

    QPointF prevPtCenterPos; // position of the previous path marker
    QPointF nextPtCenterPos; // position of the next path marker

    bool mouseOver = false; // denotes whether the cursor is hovering over the PathPointMarker
    bool pathStart = false; // denotes whether the path marker represents the start of the path
    bool pathEnd = false; // denotes whether the path marker represents the end of the path
    bool particlePassed = false; // status boolean on whether the path marker has been passed by the particle

//------------------------------------------------------------------------

// Class Signals
signals:

    /* signal to allow creation of a new path marker (when the user's cursor is no longer
     * hovering over a path marker */
    void send_AllowNewMarker(bool allow);

    // signal to update the total path distance after the marker's change of position
    void send_PositionChanged(PathPointMarker * selfPointer);

//------------------------------------------------------------------------

// Class Public Functions (non-slots)/Members
public:

    // holds the location data of the PathPointtmarker
    PathPtStruct *data = new PathPtStruct;

    // constructor with references to previosu and next markers
    PathPointMarker(PathPointMarker *prevMkr, PathPointMarker *nextMkr);

    // destructor
    ~PathPointMarker();

    // triggered upon mouse press; implicitly selects the marker
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    /* triggered upon mouse release; implicitly deselects the marker;
     * also signals an update to total path distance */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    /* triggered by hovering cursor over the marker; restricts creation of
     * a new marker while the cursor is over the marker */
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);

    /* triggered by hovering cursor outside the marker; allows creation of
     * a new marker in this case */
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    // renders the path marker; updates marker color and text
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    // used to adjust the viewport position data of the marker and of its connecting lines
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    // adjusts screen position of previous and next connecting lines
    void adjustConnectingLinesPosition(QPointF newPos);

    // adjusts screen position of the marker text label
    void adjustTextLabelPosition(QPointF newPos);

    // sets the marker to unpassed status (not passed by the particle)
    void set_UnpassedStatus();

    // sets the marker to passed status (passed by the particle)
    void set_PassedStatus();

    // sets whether the path marker is the start, end, or both start and end of the path
    void set_PathStart_PathEnd(bool isStartingPt, bool isEndingPt);

    // updates the previous marker reference
    void set_PrevMkr(PathPointMarker *prevMkr);

    // updates the next marker reference
    void set_NextMkr(PathPointMarker *nextMkr);

    // updates the previous connecting line reference
    void set_PrevLine(QGraphicsLineItem *line);

    // updates the next connecting line reference
    void set_NextLine(QGraphicsLineItem *line);

    // retrieve the previous marker reference
    QGraphicsLineItem* get_PrevLineRef();

    // retrieve the next marker reference
    QGraphicsLineItem* get_NextLineRef();

    // retrieve the marker text label reference
    QGraphicsSimpleTextItem* get_TextLabelRef();

    /* retrieves the position of the path marker center (Qt uses top-left corner by default
     * and that is not what is desired) */
    Point get_MarkerCenterPos();

//------------------------------------------------------------------------

};

#endif // PATHPOINTMARKER_H
