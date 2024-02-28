/*
 *	GraphicsViewer header file.
 *
 *
 *	AUTHOR: Victor Huynh
 *
 */
//------------------------------------------------------------------------

#ifndef GRAPHICSVIEWER_H
#define GRAPHICSVIEWER_H

#include <QWidget>
#include <QObject>
#include <QDebug>
#include <QMouseEvent>
#include <QGraphicsView>

#include "point.h"
#include "macros.h"

//------------------------------------------------------------------------

/*
 * Class Description:
 * GraphicsViewer is a custom implementation of QGraphicsView, a widget used to
 * display the contents of a QGraphicsScene. This widget was customized to account
 * for path drawing.
 */

//------------------------------------------------------------------------

class GraphicsViewer : public QGraphicsView
{
    Q_OBJECT

//------------------------------------------------------------------------

// Class Private Members
private:

    /* controls path marker creation (used to signify if user's mouse is
     * hovering over a marker or over empty area) */
    bool allowMouseClick = true;

//------------------------------------------------------------------------

// Class Public Functions (non-slots)/Members
public:

    // constructor
    GraphicsViewer(QWidget *parent = nullptr);

//------------------------------------------------------------------------

// Class Signals
signals:

    // signal to generate and display a path marker on user mouse click
    void send_AddPathMarker(QPointF pos);

    // signal to remove the most recent path marker created by the user
    void send_RemoveMostRecentPathMarker();

//------------------------------------------------------------------------

// Class Public Slots (Functions)
public slots:

    /* triggered upon mouse press; allows for either creation/deletion/context menu
     * display of a path marker */
    void mousePressEvent(QMouseEvent *event);

    /* triggered upon mouse double-click while cursor is over the widget; coded to
     * allow for more responsive marker deletion behavior */
    void mouseDoubleClickEvent(QMouseEvent *event);

    /* toggles whether to allow creation of a new path marker (toggled when the user's
     * mouse hovers/exits over a path marker) */
    void toggle_AllowLeftMouseClick(bool allow);

//------------------------------------------------------------------------

};

#endif // GRAPHICSVIEWER_H
