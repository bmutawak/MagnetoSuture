/*
 *	GraphicsViewer source file.
 *
 *
 *	AUTHOR: Victor Huynh
 *
 */
//------------------------------------------------------------------------

#include "graphicsviewer.h"

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - constructor; nothing special needed here
 */
GraphicsViewer::GraphicsViewer(QWidget *parent) :
    QGraphicsView (parent)
{

}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - triggered upon mouse press; allows for either
 * creation/deletion/context menu display of a path marker
 */
void GraphicsViewer::mousePressEvent(QMouseEvent *event) {

    // left mouse click case: signals the creation of path marker
    if (event->button() == Qt::LeftButton && allowMouseClick) {
        emit send_AddPathMarker(event->pos());
    }

    /* right mouse click case: signals the removal of the most recent path marker */
    else if (event->button() == Qt::RightButton) {
        emit send_RemoveMostRecentPathMarker();
        toggle_AllowLeftMouseClick(true);
    }

    // calling QGraphicsView's (parent class) implementation to ensure normal behavior
    QGraphicsView::mousePressEvent(event);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - triggered upon mouse double-click while cursor is
 * over the widget; coded to allow for more responsive marker deletion behavior
 */
void GraphicsViewer::mouseDoubleClickEvent(QMouseEvent *event) {

    /* double right click allows for deletion of the most recent path marker; allows
     * the user to right click rapidly to delete (even if user intended single right
     * click, system sees it as double right click so this is needed) */
    if (event->button() == Qt::RightButton) {
        emit send_RemoveMostRecentPathMarker();
    }

    // calling QGraphicsView's (parent class) implementation to ensure normal behavior
    QGraphicsView::mouseDoubleClickEvent(event);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION: toggles whether to allow creation of a new path marker
 * (toggled when the user's mouse hovers/exits over a path marker)
 */
void GraphicsViewer::toggle_AllowLeftMouseClick(bool allow) {
    allowMouseClick = allow;
}

//------------------------------------------------------------------------
