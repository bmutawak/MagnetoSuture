/*
 *	PathPointMarker source file.
 *
 *
 *	AUTHOR: Victor Huynh
 *
 */
//------------------------------------------------------------------------

#include "pathpointmarker.h"
#include "ImageSegmentation.h"

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - constructor; sets up object flags and private members
 */
PathPointMarker::PathPointMarker(PathPointMarker *prevMkr, PathPointMarker *nextMkr) : QGraphicsEllipseItem ()
{

    // initialize PathPointMarker flags
    this->set_UnpassedStatus();

    // set marker references
    this->prevMkr = prevMkr;
    this->nextMkr = nextMkr;

    // customize text label
    this->textLabel->setFont(QFont("Courier",14));
    this->textLabel->setBrush(QBrush(Qt::white));

    // other properties
    this->setAcceptHoverEvents(true);
    this->setCursor(Qt::PointingHandCursor);
    this->setRect(0,0,15,15);

    // initialize the pixel screen position
    this->data->pixelScreen = Point(this->scenePos().x() + this->rect().x() + (this->rect().width()/2.0),
                               this->scenePos().y() + this->rect().y() + (this->rect().height()/2.0));

    // each path marker is treated as checkpoint within the delivery path (see controlmodule class)
    this->data->isCheckpoint = true;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - destructor
 */
PathPointMarker::~PathPointMarker() {

}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - implicitly selects the marker the marker
 */
void PathPointMarker::mousePressEvent(QGraphicsSceneMouseEvent *event) {

    // calling parent class's implementation to ensure normal behavior
    QGraphicsEllipseItem::mousePressEvent(event);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - triggered upon mouse release; implicitly deselects
 * the marker; also signals an update to total path distance
 */
void PathPointMarker::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    emit send_PositionChanged(this);

    // calling parent class's implementation to ensure normal behavior
    QGraphicsEllipseItem::mouseReleaseEvent(event);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - triggered by hovering cursor over the marker;
 * restricts creation of a new marker while the cursor is over the marker
 */
void PathPointMarker::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    mouseOver = true;
    emit send_AllowNewMarker(false);

    // force a call to paint to update marker color
    update();

    // calling parent class's implementation to ensure normal behavior
    QGraphicsEllipseItem::hoverEnterEvent(event);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - triggered by hovering cursor out of the marker;
 * allows creation of a new marker in this case
 */
void PathPointMarker::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    mouseOver = false;
    emit send_AllowNewMarker(true);

    // force a call to paint to update marker color
    update();

    // calling parent class's implementation to ensure normal behavior
    QGraphicsEllipseItem::hoverLeaveEvent(event);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - renders path marker (marker color and text label)
 */
void PathPointMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {

    // mouse hover case
    if (mouseOver) {
        painter->setBrush(QBrush(QColor(252, 185, 65)));
    }
    // passed by the particle case
    else if (particlePassed) {
        painter->setBrush(QBrush(QColor(46, 204, 113)));
    }
    else {
        // path start case
        if (pathStart && !pathEnd) {
            painter->setBrush(QBrush(QColor(159, 90, 253)));
            textLabel->setText("Start");
        }
        // path end case
        else if (pathEnd && !pathStart) {
            painter->setBrush(QBrush(QColor(242, 38, 19)));
            textLabel->setText("End");
        }
        // both path start and end case
        else if (pathStart && pathEnd) {
            painter->setBrush(QBrush(QColor(159, 90, 253)));
            textLabel->setText("Start & End");
        }
        // no hover case
        else {
            painter->setBrush(QBrush(QColor(255, 246, 143)));
        }
    }

    painter->setPen(QPen(Qt::black,3,Qt::SolidLine));
    painter->setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
    painter->drawEllipse(this->rect());
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - used to adjust the viewport position data of the marker
 * and of its connecting lines; called implicitly
 */
QVariant PathPointMarker::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionChange && scene()) {
        QPointF newPos = value.toPointF();
        QRectF rect = this->scene()->sceneRect();

        // binds the path marker within the QGraphicsScene (no leaving the viewport)
        if (!rect.contains(newPos) || !rect.contains(QPointF(newPos.x()+this->rect().width(),
                                                             newPos.y()+this->rect().height()))) {

            newPos.setX(qMin(rect.right()-this->rect().width(),
                             qMax(newPos.x(), rect.left())));

            newPos.setY(qMin(rect.bottom()-this->rect().height(),
                             qMax(newPos.y(), rect.top())));
        }

        adjustConnectingLinesPosition(newPos);
        adjustTextLabelPosition(newPos);

        // get center pixel position of the path marker
        this->data->pixelScreen = PathPoint(newPos.x()+(this->rect().width()/2.0),
                                            newPos.y()+(this->rect().height()/2.0));
        return newPos;
    }

    return QGraphicsItem::itemChange(change, value);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - adjusts screen position of previous and next connecting
 * lines
 */
void PathPointMarker::adjustConnectingLinesPosition(QPointF newPos) {

    // center position of the marker
    QPointF newCenterPos = QPointF(newPos.x() + this->rect().x() + this->rect().width()/2.0,
                                   newPos.y() + this->rect().y() + this->rect().height()/2.0);

    // update previous connecting line
    if (prevMkr != nullptr) {
        prevPtCenterPos = QPointF(prevMkr->scenePos().x() + prevMkr->rect().x() + prevMkr->rect().width()/2.0,
                                  prevMkr->scenePos().y() + prevMkr->rect().y() + prevMkr->rect().height()/2.0);
        prevPtLine->setLine(QLineF(prevPtCenterPos, newCenterPos));
        prevPtLine->setZValue(-1);
    }

    // update next connecting line
    if (nextMkr != nullptr) {
        nextPtCenterPos = QPointF(nextMkr->scenePos().x() + nextMkr->rect().x() + nextMkr->rect().width()/2.0,
                                  nextMkr->scenePos().y() + nextMkr->rect().y() + nextMkr->rect().height()/2.0);
        nextPtLine->setLine(QLineF(nextPtCenterPos, newCenterPos));
        nextPtLine->setZValue(-1);
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - adjusts screen position of the marker text label
 */
void PathPointMarker::adjustTextLabelPosition(QPointF newPos) {

    // center position of text label
    QPointF newTextPos = QPointF(newPos.x() + this->rect().x() + this->rect().width()/2.0,
                                 newPos.y() + this->rect().y() + this->rect().height());

    if (!textLabel->isActive()) {
        this->scene()->addItem(textLabel);
    }

    textLabel->setPos(newTextPos);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - sets the marker to unpassed status (not passed by the particle)
 */
void PathPointMarker::set_UnpassedStatus() {

    // flags are key to the behavior of the path marker
    this->setFlags(QGraphicsItem::ItemIsSelectable |
                   QGraphicsItem::ItemIsMovable |
                   QGraphicsItem::ItemSendsGeometryChanges |
                   QGraphicsItem::ItemSendsScenePositionChanges);

    this->particlePassed = false;
    update();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - sets the path marker to passed status (passed by the particle)
 */
void PathPointMarker::set_PassedStatus() {

    // make path marker no longer movable; safety feature
    this->setFlags(this->flags() & ~QGraphicsItem::ItemIsMovable);

    this->particlePassed = true;
    update();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - sets whether the path marker is the start, end, or
 * both stard and end of the path
 */
void PathPointMarker::set_PathStart_PathEnd(bool isStartingPt, bool isEndingPt) {
    pathStart = isStartingPt;
    pathEnd = isEndingPt;

    // force a call to paint to update marker appearance based on start/end status
    update();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - updates the previous marker reference
 */
void PathPointMarker::set_PrevMkr(PathPointMarker *prevMkr) {
    this->prevMkr = prevMkr;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - updates the next marker reference
 */
void PathPointMarker::set_NextMkr(PathPointMarker *nextMkr) {
    this->nextMkr = nextMkr;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - updates the previous connecting line reference;
 * no need to update line style as set_NextLine takes care of that in its
 * implementation (the previous line of one marker is the next line of another)
 */
void PathPointMarker::set_PrevLine(QGraphicsLineItem *line) {
    this->prevPtLine = line;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - updates the next connecting line reference
 */
void PathPointMarker::set_NextLine(QGraphicsLineItem *line) {
    this->nextPtLine = line;

    // updates the line style
    this->nextPtLine->setPen(QPen(QColor(0, 178, 255),4,Qt::DashLine));
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - retrieves the previous connecting line reference
 */
QGraphicsLineItem* PathPointMarker::get_PrevLineRef() {
    return this->prevPtLine;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - retrieves the next connecting line reference
 */
QGraphicsLineItem* PathPointMarker::get_NextLineRef() {
    return this->nextPtLine;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - retrieves the marker text label reference
 */
QGraphicsSimpleTextItem* PathPointMarker::get_TextLabelRef() {
    return this->textLabel;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - retrieves the position of the path marker center
 * (Qt uses top-left corner by default and that is not what is desired)
 */
Point PathPointMarker::get_MarkerCenterPos() {
    return this->data->pixelScreen;
}

//------------------------------------------------------------------------
