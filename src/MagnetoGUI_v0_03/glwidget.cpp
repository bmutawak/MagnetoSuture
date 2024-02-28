/*
 *	GLWdiget source file.
 *
 *
 *	AUTHOR: Victor Huynh
 *
 */
//------------------------------------------------------------------------

#include "glwidget.h"

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - constructor; sets up pen styles
 */
GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent) {
    pointPen.setColor(Qt::white);
    pointPen.setWidth(4);
    linePen.setColor(Qt::blue);
    linePen.setWidth(3);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - initialization function; a virtual function called
 * once implicitly before the 1st instance of painting; necessary to utilize
 * this widget
 */
void GLWidget::initializeGL() {
    // mandatory to call this first
    initializeOpenGLFunctions();

    // sets initial widget color
    glClearColor(0, 0, 0, 1);

    // disables depth buffer
    glDisable(GL_DEPTH_TEST);

    // specifies modifiable viewing pipeline matrices
    glMatrixMode(GL_PROJECTION);
    glMatrixMode(GL_MODELVIEW);

    // resets current matrix to identity matrix
    glLoadIdentity();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - called on each painting instance; performs image
 * rendering; key function of the class
 */
void GLWidget::paintGL() {

    // clears buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // disables depth buffer
    glDisable(GL_DEPTH_TEST);

    // specifies modifiable viewing pipeline matrices
    glMatrixMode(GL_PROJECTION);
    glMatrixMode(GL_MODELVIEW);

    // resets current matrix to identity matrix
    glLoadIdentity();

    // convert image data to QImage format (which is compatable for painting)
    const uchar *qImageBuffer = const_cast<uchar*>(frameMat.data);
    frameQImage = QImage(qImageBuffer, frameMat.cols, frameMat.rows, static_cast<int>(frameMat.step), QImage::Format_RGB888);

    // using QPainter to draw the image onto the GLWidget (note the .begin and .end)
    painter.begin(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, 1);
    painter.setRenderHint(QPainter::HighQualityAntialiasing, 1);

    // draw image
    painter.drawImage(this->rect(), frameQImage);

    // draw target vector
    if (displayTargetVector) {draw_TargetVector();}

    painter.end();

    // used to flush out buffered commands
    glFlush();
    glFinish();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - updates current image data and forces a call to paintGL
 */
void GLWidget::set_Image(cv::Mat *frame) {
    frameMat = *frame;
    this->update();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - updates target vector screen coordinates
 */
void GLWidget::set_TargetVector(Point currentParticleLoc_inPixel, Point targetPathPt_inPixel) {
    targetVector.setPoints(currentParticleLoc_inPixel, targetPathPt_inPixel);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - retrieves current image frame (function not used in this
 * program but could be helpful in the future
 */
QImage GLWidget::get_Image() {
    return frameQImage;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - returns whether target vector is being displayed
 */
bool GLWidget::isDisplayingTargetVector() {
    return displayTargetVector;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - toggles display of target vector
 */
void GLWidget::toggle_DisplayTargetVector(bool on) {
    displayTargetVector = on;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - draws target vector onto QImage
 */
void GLWidget::draw_TargetVector() {

    /* prevents unintentional vectors from being drawn (resulting from faulty particle
     * detection */
    if (targetVector.p1() == QPointF(0,0) || targetVector.p2() == QPointF(0,0)) {
        return;
    }

    painter.setPen(linePen);
    painter.drawLine(targetVector);

    // highlight the target point
    painter.setPen(pointPen);
    painter.drawPoint(targetVector.p2());
}

//------------------------------------------------------------------------
