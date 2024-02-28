/*
 *	MainWindow header file.
 *
 *
 *	AUTHOR: Victor Huynh
 *
 */
//------------------------------------------------------------------------

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QTimer>
#include <QElapsedTimer>
#include <QThread>
#include <QDebug>
#include <QMessageBox>
#include <QDate>
#include <QDateTime>
#include <QPoint>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QScreen>
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsEllipseItem>
#include <QInputDialog>

#include "opencvworker.h"
#include "glwidget.h"
#include "dialog_settingswindow.h"
#include "Physics.h"
#include "point.h"
#include "pathpointmarker.h"
#include "ControlModule.h"
#include "macros.h"

//------------------------------------------------------------------------

/*
 * Class Description:
 * MainWindow generates the GUI's main window. Because its members contain
 * references to objects of the other major software modules, it can be
 * considered the central class of this Qt Project.
 */

//------------------------------------------------------------------------

namespace Ui {
class MainWindow;
}

//------------------------------------------------------------------------

class MainWindow : public QMainWindow
{
    Q_OBJECT

//------------------------------------------------------------------------

// Class Public Functions (non-slots)/Members
public:

    // contructor
    explicit MainWindow(QWidget *parent = nullptr);

    // destructor
    ~MainWindow();

//------------------------------------------------------------------------

// Class Private Members
private:

    Ui::MainWindow *ui; // holds references to all window components (mainwindow.ui)
    Dialog_SettingsWindow *settingsWindow = new Dialog_SettingsWindow(); // pointer to Settings Window object
    OpenCvWorker *worker = new OpenCvWorker(); // pointer to OpenCv Class object
    Physics *physicsModule = nullptr; // pointer to Physics Module object
    ControlModule *opControl = new ControlModule(); // pointer to Control Module object
    QGraphicsScene *pathScene = new QGraphicsScene(); // pointer to Graphics Scene object (for pathing purposes)

    std::vector<PathPointMarker*> pathPtMarkers; // vector of path points drawn by the user

    QThread *mainThread; /* thread to handle all image processing; allows for concurrent processing
                          * (no overlap in user GUI interaction (e.g. clicking buttons) and image
                          * processing code execution */
    QTimer *mainTimer; // timer that controls the frequency of image streaming
    QTimer *hardwareTimer; // timer that controls the frequency of translation commands
    QTimer *particleDetectionCalibrationTimer; // timer that controls the speed of particle translations during the calibration procedure
    QElapsedTimer *logTimer = new QElapsedTimer(); // timer that provides timestamps for data logs

    QFile *opLogFile; /* pointer to operation log text file; this file records user interaction with the
                       * GUI and operation events (i.e. detected errors, progress updates, etc.) */
    QFile *dataLogFile; /* pointer to data log .csv file; this file records the output of all translation commands of
                         * the particle delivery operation */
    QFile *loadedPathFile; // pointer to file containing a previously saved path
    QFile *savedPathFile; // pointer to file saving the currently drawn path

    QString userName = ""; // user's name; updates the filenames of the operation and data logs
    QString saveFolder = QDir::currentPath(); // filepath to export data logs
    QString loadedPathFilename; // filename of loaded path file
    QString savedPathFilename; // filename of saved path file

    QString defaultOpLogFilename = saveFolder + "/" + QDate::currentDate().toString("MM-dd-yy") + "_opLog.txt"; // default operation log filename format
    QString startUpOpLogFilename = QDir::currentPath() + "/" + "startupOpLog.txt"; // operation log filename only at GUI startup
    QString prevOpLogFilename = startUpOpLogFilename; // previous operation log filename (initially set to the startup filename)
    QString opLogFilename = defaultOpLogFilename; // filename of operation log
    QString opLogEntry; // line to be written to the operation log

    QString defaultDataLogFilename = saveFolder + "/" + QDate::currentDate().toString("MM-dd-yy") + "_dataLog.csv"; // default data log filename format
    QString dataLogFilename = defaultDataLogFilename; // filename of data log
    QStringList dataLogEntry; // list of numbers/characters comprising a single timestamped entry to the data log

    Point currentParticleLoc = Point(-1,-1); // current particle location point
    PathPoint targetLocation = PathPoint(-1,-1); // current target location for the particle to move to

    std::vector<int> translationCommandArray; // vector with numeric values for the translation command

    int calibrationCycler = -1; // used to cycle through particle translations during the calibration process
    int commandCounter = 0; // records the total number of commands sent to the motor controllers
    int numPathMarkers = 0; // number of path points drawn by the user

    /* used to accumulate the time elapsed for the delivery operation (useful for cases of
     * pausing/resuming operation) */
    double elapsedLogTime = 0.0;
    double screenResizeFactor = 1.0; // used to resize camera capture images to fill the screen
    double totalPathDistance = 0.0; // used to hold the physical path distance (mm)
    double t_current = 0; // timestamp of current translation
    double t_prev = 0; // timestamp of the previous translation command
    double x_init = 0.0; // particle x-position (mm) right before the current translation command
    double y_init = 0.0; // particle y-position (mm) right before the current translation command
    double x_final = 0.0; // particle x-position (mm) right after the current translation command
    double y_final = 0.0; // particle y-position (mm) right after the current translation command
    double distanceMoved = 0.0; // particle distance traveled from the current translation
    double velocity = 0.0; // particle velocity from the current translation

    bool cameraReady = false; // denotes camera connection status
    bool motorControllersReady = false; // denotes motor controller connection status
    bool logsReady = false; // denotes that the both the operation and data logs are set up
    bool drawPathMode = false; // toggles whether the user can draw a path
    bool operationInProgress = false; // denotes whether the delivery operation is ongoing or not; key variable for system automation
    bool atStartUp = true; // used to coordinate operation log file setup

//------------------------------------------------------------------------

// Class Signals
signals:

    // signal to disable/enable certain UI components when delivery operation is in-progress
    void send_InOpWidgetStatesPrompt();

    // signal to disable/enable certain UI components when delivery operation is paused
    void send_PausedOpWidgetStatesPrompt();

    // signal to disable/enable certain UI components when delivery operation is stopped
    void send_StoppedOpWidgetStatesPrompt();

    // signal to finish out system calibration by generating the background "clean" image
    void send_SynthesizeCleanImagePrompt();

    // signal to start/restart the delivery operation
    void send_RestartOpPrompt();

    // signal to stop the delivery operation
    void send_StopOpPrompt();

    // signal to stop the translation command timer (halt translations)
    void send_HaltTranslationPrompt();

//------------------------------------------------------------------------

// Class Private Slots (Functions)
private slots:

    /* establishes all connections with the other software modules/within the class;
     * key for communication between classes */
    void establish_Connections();

    /* controls output of any keypress (outside of entering text);
     * all keypresses are effectively ignored for safety */
    void keyPressEvent(QKeyEvent *event);

    // controls UI resizing behaviors (i.e. from screen to screen, while dragging the window, etc.)
    void resizeEvent(QResizeEvent *event);

    // controls UI exit behavior; ensures proper closeout
    void closeEvent(QCloseEvent *event);

    // sets UI component states (i.e. enabled or disabled) upon GUI startup
    void set_InitialWidgetStates();

    // sets UI component states upon start of delivery operation
    void set_InOpWidgetStates();

    // sets UI component states upon pausing of delivery operation
    void set_PausedOpWidgetStates();

    // sets UI component states upon stoppage of delivery operation
    void set_StoppedOpWidgetStates();

    /* toggles state of operation control buttons (enables or disables);
     * convenience function tied to the other "set_" functions */
    void set_OpControlPanel(bool enable);

    // sets timeout of the translation command timer (essentially the frequency of translation commands)
    void set_HardwareCommandTimeout(int);

    // sets up settings window
    void setup_SettingsWindow();

    // sets up camera viewport (resizing original camera image to fill viewport space)
    void setup_CameraViewport(int originalFrameWidth, int originalFrameHeight, double screenResizeFactor);

    // sets up data and operation logs (calls setup_DataLog and setup_OperationLog)
    void setup_Logs();

    // sets up operation log for writing (at GUI Startup)
    void setup_OperationLog();

    // sets up data log for writing
    void setup_DataLog();

    // sets up graphics scene for path drawing
    void setup_GraphicsScene();

    // ensures the system is ready for the delivery operation (checks hardware connections, logs, internal data fields, etc.)
    void setup_Operation();

    // changes username, updating the data log and operation log filenames
    void change_UserName();

    // changes save directory for data exporting
    void change_SaveFolder();

    // changes filename for the data log to be saved under
    void change_DataLogFilename();

    // changes filename for the operation log to be saved under
    void change_OperationLogFilename();

    // resets the data and operation log filenames to default format
    void reset_LogFilenames(bool useDefaultFilenames);

    // links USB-connected camera to the GUI; initiates camera streaming
    void connect_Camera();

    // links USB_connected motor controllers to the GUI
    void connect_MotorControllers();

    // used for safely reenabling the Connect Camera button after connecting motor controllers
    void re_enable_ConnectCameraButton();

    // used for safely reenabling the Connect MC button after connecting the camera
    void re_enable_ConnectMCButton();

    // receives camera frame (after processing if any) and displays it in the viewport
    void receive_FrameforDisplay(cv::Mat *frame);

    // initiates system calibration process
    void receive_CalibrationPrompt();

    // receives updated field-of-view values (physical area being viewed)
    void receive_UpdatedFOV(double width_FOV, double height_FOV);

    // initiates particle detection from camera images
    void receive_ParticleDetectionPrompt(); // maybe rename

    // enables or disables user path drawing
    void receive_DrawPathPrompt();

    // receives the current translation command vector from the Physics Module
    void receive_TranslationCommandInfo(std::vector<int> translationCommandArray);

    // opens or raises the settings window into view
    void open_SettingsWindow();

    // enables and makes visible the Path Drawing tab
    void enable_PathDrawingTab();

    // allows the user to start an operation (only allows! doesn't actually start it)
    void enable_Operation();

    // moves particle around the petri dish during the calibration process (for image averaging purposes)
    void move_ParticleForCalibration();

    // ends particle movement during system calibration process
    void stop_ParticleMovementForCalibration();

    // adds a path marker where the user clicks to the graphics scene (path drawing)
    void add_PathMarker(QPointF mkrPos);

    // removes the latest user-drawn path marker
    void remove_MostRecentPathMarker();

    // updates path marker location data
    void calibrate_PathMarker(PathPointMarker *mkr);

    // resets the path markers to unpassed status (readies them for the next operation)
    void reset_PathStatusToUnpassed();

    /* sets the designed path marker to passed status so that the user can no longer modify it
     * until the operation is stopped; is a safety feature */
    void update_PathMarkerStatusToPassed(int mkrIndex);

    // updates the total path distance based on the currently drawn path
    void update_CurrentPathDistance();

    // should update in control module too; need to update
    // deletes the current path and clears all path markers from the graphics scene
    void clear_Path();

    // loads a previously drawn path and displays onto the graphics scene
    void load_Path();

    // saves the current drawn path markers into a text file
    void save_Path();

    // starts or pauses the current delivery operation
    void startPause_Operation();

    // ends the current delivery operation
    void stop_Operation();

    // terminates the current delivery operation
    void forceStop_Operation();

    // computes the next particle translation command and outputs it to the motor controllers
    void execute_TranslationCommand();

    // calculates data fields to be written to the data log after the current translation;
    QStringList generate_DataEntry();

    // updates the fields displayed on the Progress Updates tab
    void update_ProgressUpdatesTab();

    // resets the fields displayed on the Progress Updates tab
    void reset_ProgressUpdatesTab();

    // writes a line(s) of text to the operation log
    void write_OperationLogMsg(QString msg);

    // writes a timestamped entry to the data log
    void write_DataLogMsg(QStringList entry);

    // performs drop-down behavior of the Drawing Instructions menu
    void on_pushButton_ShowHideDrawingInstructions_clicked();

    // performs drop-down behavior of the Path Info menu
    void on_pushButton_ShowHidePathInfo_clicked();

    // hides/displays the drawn path based on the current GUI tab being viewed
    void on_tabWidget_currentChanged(int index);

//------------------------------------------------------------------------

};

#endif // MAINWINDOW_H
