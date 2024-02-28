/*
 *	Dialog_SettingsWindow header file.
 *
 *
 *	AUTHOR: Victor Huynh
 *
 */
//------------------------------------------------------------------------

#ifndef DIALOG_SETTINGSWINDOW_H
#define DIALOG_SETTINGSWINDOW_H

#include <QDialog>
#include <QKeyEvent>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QDateTime>
#include <QElapsedTimer>
#include <QTimer>
#include <QTime>

#include "point.h"
#include "macros.h"

//------------------------------------------------------------------------

/*
 * Class Description:
 * Dialog_SettingsWindow generates the GUI's settings window. Key functionality
 * of the window includes system calibration and particle detection adjustment.
 */

//------------------------------------------------------------------------

namespace Ui {
class Dialog_SettingsWindow;
}

//------------------------------------------------------------------------

class Dialog_SettingsWindow : public QDialog
{
    Q_OBJECT

//------------------------------------------------------------------------

// Class Public Functions (non-slots)/Members
public:

    // constructor
    explicit Dialog_SettingsWindow(QWidget *parent = nullptr);

    // destructor
    ~Dialog_SettingsWindow();

//------------------------------------------------------------------------

// Class Private Members
private:
    Ui::Dialog_SettingsWindow *ui; // holds references to all window components (dialog_settingswindow.ui)

//------------------------------------------------------------------------

signals:

    // signal to initiate system calibration
    void send_CalibrationPrompt();

    // signal to send updated particle detection parameters
    void send_ParticleDetectionParams(int filterThreshold, int minSize, int maxSize);

    // signal to preview particle detection using the set parameters
    void send_PreviewParticleDetectionPrompt();

    // signal to enable display of system axes and bounding box
    void send_DisplaySystemAxesPrompt(bool);

    // signal to update the timeout of the timer responsible for executing translation commands
    void send_HardwareCommandTimeout(int);

    // signal to write and display to the GUI's operation log
    void send_OperationLogMsg(QString msg);

//------------------------------------------------------------------------

// Class Private Slots (Functions)
private slots:

    /* controls output of any keypress (outside of entering text);
     * all keypresses are effectively ignored for safety */
    void keyPressEvent(QKeyEvent *event);

    /* when a slider is updated, the value of the corresponding spinbox is updated;
     * when a spinbox is updated, the value of the corresponding slider is updated*/
    void on_horizontalSlider_FilterThreshold_valueChanged(int value);
    void on_horizontalSlider_MinParticleSize_valueChanged(int value);
    void on_horizontalSlider_MaxParticleSize_valueChanged(int value);
    void on_spinBox_FilterThreshold_editingFinished();
    void on_spinBox_MinParticleSize_editingFinished();
    void on_spinBox_MaxParticleSize_editingFinished();

//------------------------------------------------------------------------

// Class Public Slots (Functions)
public slots:

    // establishes all Dialog_SettingsWindow connections within the class
    void establish_Connections();

    // sets window component states (i.e. enabled or disabled) upon GUI startup
    void set_InitialWidgetStates();

    // sets window component states upon start of delivery operation
    void set_InOpWidgetStates();

    // sets window component states upon pausing of delivery operation
    void set_PausedOpWidgetStates();

    // sets window component states upon stoppage of delivery operation
    void set_StoppedOpWidgetStates();

    // sets up initial values for particle detection spinboxes and sldiers
    void setup_InitialParticleDetectionParams(int initialFilterThreshold, int initialMinSize, int initialMaxSize);

    // initiates system calibration process
    void initiate_SystemCalibration();

    // initiates particle detection using set parameters
    void preview_ParticleDetection();

    // enables button that initiates particle detection
    void enable_ParticleDetectionPreview();

    // applies settings to the program
    void apply_Settings();

//------------------------------------------------------------------------

};

#endif // DIALOG_SETTINGSWINDOW_H
