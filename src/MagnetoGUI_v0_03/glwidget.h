/*
 *	GLWdiget header file.
 *
 *
 *	AUTHOR: Victor Huynh
 *
 */
//------------------------------------------------------------------------

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QWidget>
#include <QObject>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QStaticText>
#include <QLineF>

#include <qopengl.h>
#include <opencv2/opencv.hpp>
#include "point.h"
#include "macros.h"

//------------------------------------------------------------------------

/*
 * Class Description:
 * GLWidget utilizes OpenGL to improve image rendering capabilities by allocating
 * most of the workload from to the GPU to the CPU. This cuts down significantly
 * on CPU usage and program memory and mitigates visual artifacts. Streamed images
 * are displayed onto the GLWidget.
 */

//------------------------------------------------------------------------

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

//------------------------------------------------------------------------

// Class Private Members
private:

    cv::Mat frameMat; // frame acquired in Mat format from OpenCv
    QImage frameQImage; // frame in QImage format to be displayed on the widget
    QPainter painter; // performs low-level drawing of images

    QPen pointPen; // pen style for drawing points
    QPen linePen; // pen style for drawing lines

    /* line segment representing vector from current particle location
     * to the next desired location */
    QLineF targetVector = QLineF();

    bool displayTargetVector = false; // denotes whether to draw and display the target vector

//------------------------------------------------------------------------

/* Class Public Functions (non-slots)/Members;
 * overriding default QOpenGLWidget functions for custom  behavior
 */
protected:

    // prepares widget for graphics rendering
    void initializeGL() override;

    // draws new image onto the widget
    void paintGL() override;

//------------------------------------------------------------------------

// Class Public Functions (non-slots)/Members
public:

    // constructor
    GLWidget(QWidget *parent);

//------------------------------------------------------------------------

// Class Public Slots (Functions)
public slots:

    // updates the image frame to be displayed
    void set_Image(cv::Mat *frame);

    // updates the target vector
    void set_TargetVector(Point currentParticleLoc_inPixel, Point targetPathPt_inPixel);

    // retrieves current image frame (function not used, but potentially useful)
    QImage get_Image();

    // returns whether target vector is being displayed
    bool isDisplayingTargetVector();

    // toggles display of target vector (vector needs to be set first)
    void toggle_DisplayTargetVector(bool on);

    // draws target vector onto QImage
    void draw_TargetVector();

//------------------------------------------------------------------------

};

#endif // GLWIDGET_H
