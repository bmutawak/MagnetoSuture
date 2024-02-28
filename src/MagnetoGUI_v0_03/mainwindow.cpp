/*
 *	MainWindow source file.
 *
 *
 *	AUTHOR: Victor Huynh
 *
 */
//------------------------------------------------------------------------

#include "mainwindow.h"
#include "ui_mainwindow.h"

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - constructor is called at GUI startup; aligns certain UI components,
 * sets initial component states, and establishes UI connections
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Magneto v0.03");

    // disable/hide certain widgets
    this->statusBar()->setSizeGripEnabled(false);
    ui->mainToolBar->setVisible(false);

    // center align certain widgets within the UI tabs
    ui->groupBox_SettingsConfigure->layout()->setAlignment(ui->pushButton_SettingsWindow, Qt::AlignRight);
    ui->groupBox_SettingsConfigure->layout()->setAlignment(ui->pushButton_GoToPathDrawing, Qt::AlignHCenter);
    ui->groupBox_DrawPathTools->layout()->setAlignment(ui->frame_PathButtons, Qt::AlignHCenter);
    ui->groupBox_DrawPathTools->layout()->setAlignment(ui->frame_PathOptions, Qt::AlignHCenter);

    // set initial text in line edit widgets
    ui->lineEdit_SaveDirectory->setText(saveFolder);
    ui->lineEdit_FilenameDataLog->setText(QDate::currentDate().toString("MM-dd-yy") + userName + "_dataLog.csv");
    ui->lineEdit_FilenameOpLog->setText(QDate::currentDate().toString("MM-dd-yy") + "_opLog.txt");

    // prevent user from directly modifiying certain line edit widgets
    ui->lineEdit_SaveDirectory->setReadOnly(true);
    ui->lineEdit_CameraStatus->setReadOnly(true);
    ui->lineEdit_MotorControllerStatus->setReadOnly(true);
    ui->plainTextEdit_OperationLog->setReadOnly(true);
    ui->textEdit_DrawPathInstructions->setReadOnly(true);

    /* important to call establish_Connections first before the other code in order
     * to construct MainWindow's class members */
    this->establish_Connections();

    this->set_InitialWidgetStates();
    this->setup_SettingsWindow();

    // sends save directory information to OpenCvWorker member
    worker->receive_SaveFolder(saveFolder);

    // sets up initial startup operation log
    setup_OperationLog();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - destructor is called upon exiting the GUI
 */
MainWindow::~MainWindow()
{
    // frees ui pointers
    delete ui;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - establishes all MainWindow connections with the
 * other classes/within the class and sets up key threads and timers;
 * very important setup function
 */
void MainWindow::establish_Connections() {

    // set up threads and timers
    mainThread = new QThread();
    mainTimer = new QTimer();
    mainTimer->setInterval(1);
    hardwareTimer = new QTimer();
    hardwareTimer->setInterval(800);
    particleDetectionCalibrationTimer = new QTimer();
    particleDetectionCalibrationTimer->setInterval(400);

    // thread and timer connections
    connect(mainThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(mainThread, SIGNAL(finished()), opControl, SLOT(deleteLater()));
    connect(mainThread, SIGNAL(finished()), pathScene, SLOT(deleteLater()));
    connect(mainThread, SIGNAL(finished()), mainTimer, SLOT(deleteLater()));
    connect(mainThread, SIGNAL(finished()), hardwareTimer, SLOT(deleteLater()));
    connect(mainThread, SIGNAL(finished()), particleDetectionCalibrationTimer, SLOT(deleteLater()));
    connect(worker, SIGNAL(send_ResumeFrameDisplayPrompt()), mainTimer, SLOT(start()));
    connect(worker, SIGNAL(send_PauseFrameDisplayPrompt()), mainTimer, SLOT(stop()));
    connect(settingsWindow, SIGNAL(send_HardwareCommandTimeout(int)), this, SLOT(set_HardwareCommandTimeout(int)));

    // camera streaming connections
    connect(worker, SIGNAL(send_StreamOrientationParams(int, int, double)), this, SLOT(setup_CameraViewport(int, int, double)));
    connect(mainTimer, SIGNAL(timeout()), worker, SLOT(grabFrame()));
    connect(worker, SIGNAL(send_FrameforDisplay(cv::Mat*)), this, SLOT(receive_FrameforDisplay(cv::Mat*)));
    connect(ui->pushButton_ConnectCamera, SIGNAL(clicked()), this, SLOT(connect_Camera()));

    // hardware-related connections
    connect(ui->pushButton_ConnectMotorControllers, SIGNAL(clicked()), this, SLOT(connect_MotorControllers()));

    // image processing-related connections
    connect(this, SIGNAL(send_SynthesizeCleanImagePrompt()), &worker->imageSegmenter, SLOT(synthesize_CleanImage()));
    connect(particleDetectionCalibrationTimer, SIGNAL(timeout()), this, SLOT(move_ParticleForCalibration()));
    connect(&worker->imageSegmenter, SIGNAL(send_StartParticleDetectionCalibration()), particleDetectionCalibrationTimer, SLOT(start()));
    connect(&worker->imageSegmenter, SIGNAL(send_StopParticleDetectionCalibration()), this, SLOT(stop_ParticleMovementForCalibration()));
    connect(&worker->imageSegmenter, SIGNAL(send_StopParticleDetectionCalibration()), settingsWindow, SLOT(enable_ParticleDetectionPreview()));
    connect(settingsWindow, SIGNAL(send_CalibrationPrompt()), this, SLOT(receive_CalibrationPrompt()));
    connect(settingsWindow, SIGNAL(send_ParticleDetectionParams(int, int, int)), &worker->imageSegmenter, SLOT(receive_ParticleDetectionParams(int, int, int)));
    connect(settingsWindow, SIGNAL(send_PreviewParticleDetectionPrompt()), this, SLOT(receive_ParticleDetectionPrompt()));
    connect(settingsWindow, SIGNAL(send_DisplaySystemAxesPrompt(bool)), &worker->imageSegmenter, SLOT(toggle_displayAxes(bool)));

    // path drawing connections
    connect(ui->graphicsView_Path, SIGNAL(send_AddPathMarker(QPointF)), this, SLOT(add_PathMarker(QPointF)));
    connect(ui->graphicsView_Path, SIGNAL(send_RemoveMostRecentPathMarker()), this, SLOT(remove_MostRecentPathMarker()));
    connect(ui->pushButton_GoToPathDrawing, SIGNAL(clicked()), this, SLOT(enable_PathDrawingTab()));
    connect(ui->pushButton_DrawPath, SIGNAL(clicked()), this, SLOT(receive_DrawPathPrompt()));
    connect(ui->pushButton_ClearPath, SIGNAL(clicked()), this, SLOT(clear_Path()));
    connect(ui->pushButton_LoadPath, SIGNAL(clicked()), this, SLOT(load_Path()));
    connect(ui->pushButton_SavePath, SIGNAL(clicked()), this, SLOT(save_Path()));
    connect(&worker->imageSegmenter, SIGNAL(send_UpdatedFOV(double,double)), this, SLOT(receive_UpdatedFOV(double,double)));

    // operation control connections
    connect(this, SIGNAL(send_RestartOpPrompt()), hardwareTimer, SLOT(start()));
    connect(this, SIGNAL(send_HaltTranslationPrompt()), hardwareTimer, SLOT(stop()));
    connect(this, SIGNAL(send_InOpWidgetStatesPrompt()), settingsWindow, SLOT(set_InOpWidgetStates()));
    connect(this, SIGNAL(send_PausedOpWidgetStatesPrompt()), settingsWindow, SLOT(set_PausedOpWidgetStates()));
    connect(this, SIGNAL(send_StoppedOpWidgetStatesPrompt()), settingsWindow, SLOT(set_StoppedOpWidgetStates()));
    connect(opControl, SIGNAL(send_CheckpointPassed(int)), this, SLOT(update_PathMarkerStatusToPassed(int)));
    connect(opControl, SIGNAL(send_StopOpPrompt()), this, SLOT(stop_Operation()));
    connect(hardwareTimer, SIGNAL(timeout()), this, SLOT(execute_TranslationCommand()));
    connect(ui->pushButton_GoToOperation, SIGNAL(clicked()), this, SLOT(enable_Operation()));
    connect(ui->pushButton_StartPauseOperation, SIGNAL(clicked()), this, SLOT(startPause_Operation()));
    connect(ui->pushButton_StopOperation, SIGNAL(clicked()), this, SLOT(stop_Operation()));
    connect(ui->pushButton_TerminateOperation, SIGNAL(clicked()), this, SLOT(forceStop_Operation()));

    // data collection setup connections
    connect(ui->lineEdit_UserName, SIGNAL(returnPressed()), this, SLOT(change_UserName()));
    connect(ui->pushButton_BrowseSaveFolder, SIGNAL(clicked()), this, SLOT(change_SaveFolder()));
    connect(ui->lineEdit_FilenameDataLog, SIGNAL(returnPressed()), this, SLOT(change_DataLogFilename()));
    connect(ui->lineEdit_FilenameOpLog, SIGNAL(returnPressed()), this, SLOT(change_OperationLogFilename()));
    connect(ui->checkBox_UseDefaultFilenames, SIGNAL(toggled(bool)), this, SLOT(reset_LogFilenames(bool)));
    connect(ui->pushButton_SettingsWindow, SIGNAL(clicked()), this, SLOT(open_SettingsWindow()));

    // writing to operationg log connections
    connect(settingsWindow, SIGNAL(send_OperationLogMsg(QString)), this, SLOT(write_OperationLogMsg(QString)));
    connect(worker, SIGNAL(send_OperationLogMsg(QString)), this, SLOT(write_OperationLogMsg(QString)));
    connect(&worker->imageSegmenter, SIGNAL(send_OperationLogMsg(QString)), this, SLOT(write_OperationLogMsg(QString)));
    connect(opControl, SIGNAL(send_OperationLogMsg(QString)), this, SLOT(write_OperationLogMsg(QString)));

    // allocate certain members to mainThread to prevent interference with the UI thread
    worker->moveToThread(mainThread);
    mainTimer->moveToThread(mainThread);

    mainThread->start();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - defines GUI behavior upon any keypress
 * (which in this case is to ignore keypresses for safety reasons)
 */
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key()) {return;}
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - defines GUI resizing behavior; allows GUI to keep
 * fixed, maximixed size within a screen regardless of the window being moved around
 */
void MainWindow::resizeEvent(QResizeEvent *event) {
    if (event->type() == QEvent::Resize) {
        QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
        this->setFixedWidth(screen->availableGeometry().width());
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - defines GUI behavior upon exit
 */
void MainWindow::closeEvent(QCloseEvent *event) {

    // user is prompted to confirm if he/she wants to exit and close the GUI
    QMessageBox closeConfirm;
    closeConfirm.setIcon(QMessageBox::Question);
    closeConfirm.setWindowTitle(tr("Exit Magneto?"));
    closeConfirm.setText(tr("Are you sure you want to close and exit Magneto?\nIf so, make sure the operation is stopped first."));
    QAbstractButton *yesButton = closeConfirm.addButton(tr("Yes"), QMessageBox::YesRole);
    QAbstractButton *noButton = closeConfirm.addButton(tr("No"), QMessageBox::NoRole);

    closeConfirm.exec();

    // settings window is closed first before exiting
    if (closeConfirm.clickedButton() == yesButton) {
        if (mainThread != nullptr && mainThread->isRunning()) {
            mainThread->quit();
        }

        settingsWindow->close();
        event->accept();
    }
    else if (closeConfirm.clickedButton() == noButton) {
        event->ignore();
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - sets initial widget states; called within constructor
 */
void MainWindow::set_InitialWidgetStates() {

    // tab states (only Instructions and Setup tabs should be initially available)
    ui->tabWidget->setCurrentIndex(1);
    ui->tabWidget->setTabEnabled(2,false);

    // setup tab widgets
    ui->pushButton_SettingsWindow->setEnabled(false);
    ui->pushButton_GoToPathDrawing->setEnabled(false);

    // path drawing tab widgets
    ui->textEdit_DrawPathInstructions->hide();
    ui->graphicsView_Path->setEnabled(false);

    // operation control panel should not be available on GUI startup
    set_OpControlPanel(false);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - sets widget states right before a delivery operation
 * is started
 */
void MainWindow::set_InOpWidgetStates() {

    // operation control panel is enabled
    set_OpControlPanel(true);

    // hardware-related and log-related widgets are disabled
    ui->pushButton_BrowseSaveFolder->setEnabled(false);
    ui->lineEdit_UserName->setEnabled(false);
    ui->lineEdit_FilenameDataLog->setEnabled(false);
    ui->lineEdit_FilenameOpLog->setEnabled(false);
    ui->checkBox_UseDefaultFilenames->setEnabled(false);
    ui->pushButton_ConnectCamera->setEnabled(false);
    ui->pushButton_ConnectMotorControllers->setEnabled(false);

    // path drawing widgets are disabled
    ui->pushButton_LoadPath->setEnabled(false);
    ui->pushButton_SavePath->setEnabled(false);
    ui->pushButton_DrawPath->setEnabled(false);
    ui->pushButton_ClearPath->setEnabled(false);
    ui->doubleSpinBox_InterpolationAmount->setEnabled(false);
    ui->doubleSpinBox_PathTolerance->setEnabled(false);

    // disable relevant widgets of the settings window
    emit send_InOpWidgetStatesPrompt();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - sets widget states for when a delivery operation
 * is paused
 */
void MainWindow::set_PausedOpWidgetStates() {

    // hardware-related and log-related widgets are disabled
    ui->pushButton_BrowseSaveFolder->setEnabled(false);
    ui->lineEdit_UserName->setEnabled(false);
    ui->lineEdit_FilenameDataLog->setEnabled(false);
    ui->lineEdit_FilenameOpLog->setEnabled(false);
    ui->checkBox_UseDefaultFilenames->setEnabled(false);
    ui->pushButton_ConnectCamera->setEnabled(false);
    ui->pushButton_ConnectMotorControllers->setEnabled(false);

    // need to redo path drawing for this to work out
    // path drawing widgets are enabled
    ui->pushButton_LoadPath->setEnabled(true);
    ui->pushButton_SavePath->setEnabled(true);
    ui->pushButton_DrawPath->setEnabled(true);
    ui->pushButton_ClearPath->setEnabled(true);
    ui->doubleSpinBox_InterpolationAmount->setEnabled(true);
    ui->doubleSpinBox_PathTolerance->setEnabled(true);

    // enable certain widgets of the settings window
    emit send_PausedOpWidgetStatesPrompt();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - sets widget states for when a delivery operation
 * is stopped
 */
void MainWindow::set_StoppedOpWidgetStates() {

    // operation control panel should not be available once operation has ended
    set_OpControlPanel(false);

    // hardware-related and log-related widgets are enabled
    ui->pushButton_BrowseSaveFolder->setEnabled(true);
    ui->lineEdit_UserName->setEnabled(true);
    ui->lineEdit_FilenameDataLog->setEnabled(true);
    ui->lineEdit_FilenameOpLog->setEnabled(true);
    ui->checkBox_UseDefaultFilenames->setEnabled(true);
    ui->pushButton_ConnectCamera->setEnabled(true);
    ui->pushButton_ConnectMotorControllers->setEnabled(true);

    // path drawing widgets are enabled
    ui->pushButton_LoadPath->setEnabled(true);
    ui->pushButton_SavePath->setEnabled(true);
    ui->pushButton_DrawPath->setEnabled(true);
    ui->pushButton_ClearPath->setEnabled(true);
    ui->doubleSpinBox_InterpolationAmount->setEnabled(true);
    ui->doubleSpinBox_PathTolerance->setEnabled(true);

    // reshows the Go To Operation button
    ui->pushButton_GoToOperation->show();

    // enable all widgets of the settings window
    emit send_StoppedOpWidgetStatesPrompt();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - convenience function used for enabling/disabling all
 * key buttons of the operation control panel
 */
void MainWindow::set_OpControlPanel(bool enable) {
    ui->pushButton_StartPauseOperation->setEnabled(enable);
    ui->pushButton_StopOperation->setEnabled(enable);
    ui->pushButton_TerminateOperation->setEnabled(enable);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - controls the frequency of translation commands being sent
 * out to the motor controllers
 */
void MainWindow::set_HardwareCommandTimeout(int timeout) {
    hardwareTimer->setInterval(timeout);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - provides initial image processing parameters to the settings window;
 * this provides the initial slider and spinbox values; function can be expanded upon
 * if the settings window receives more functionality
 */
void MainWindow::setup_SettingsWindow() {
    settingsWindow->setup_InitialParticleDetectionParams(worker->imageSegmenter.get_filterThreshold(),
                                                         worker->imageSegmenter.get_particleSizeMin(),
                                                         worker->imageSegmenter.get_particleSizeMax());
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - resizes camera viewport to fill the allocated UI space
 */
void MainWindow::setup_CameraViewport(int originalFrameWidth, int originalFrameHeight, double screenResizeFactor) {

    // resizing viewport
    this->screenResizeFactor = screenResizeFactor;
    ui->imageViewer_GL->setFixedSize(static_cast<int>(originalFrameWidth*this->screenResizeFactor),
                                     static_cast<int>(originalFrameHeight*this->screenResizeFactor));

    // set viewport container size to be fixed
    ui->frame_ImageViewerHolder->layout()->setAlignment(Qt::AlignHCenter);
    ui->frame_ImageViewerHolder->setFixedSize(ui->frame_ImageViewerHolder->size());

    /* once this viewport is fixed, the UI central widget can also be fixed, preventing
     * any unexpected resizing when moving the window */
    this->centralWidget()->setFixedSize(this->centralWidget()->size());
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - creates and sets up data log and relocates the operation log
 * to the specified save directory; prompts user to confirm overwriting an existing log
 */
void MainWindow::setup_Logs() {

    // makes sure the data and operation log filenames are updated to what the user typed in
    change_OperationLogFilename();
    change_DataLogFilename();

    QMessageBox detectedExistingLog;
    detectedExistingLog.setIcon(QMessageBox::Question);
    QAbstractButton *yesButton = detectedExistingLog.addButton(tr("Yes"), QMessageBox::YesRole);
    QAbstractButton *noButton = detectedExistingLog.addButton(tr("No"), QMessageBox::NoRole);

    QMessageBox promptRenameFile;
    promptRenameFile.setIcon(QMessageBox::Information);

    // prompts user if he/she wishes to continue with the current operation log
        if (QFile::exists(opLogFilename)) {
            detectedExistingLog.setWindowTitle("Existing Operation Log Detected!");
            detectedExistingLog.setText(tr("There is an existing file with the same operation log filename.\nDo you want to continue writing to this file?"));
            detectedExistingLog.exec();

            if (detectedExistingLog.clickedButton() == noButton) {
                promptRenameFile.setText(tr("If you wish to use a different file, please rename your operation log filename for this operation."));
                promptRenameFile.exec();
                return;
            }
        }

    // prompts user if he/she wishes to override an existing data log
    if (QFile::exists(dataLogFilename)) {
        detectedExistingLog.setWindowTitle("Existing Data Log Detected!");
        detectedExistingLog.setText(tr("There is an existing file with the same data log filename.\nDo you want to override this file?"));
        detectedExistingLog.exec();

        if (detectedExistingLog.clickedButton() == yesButton) {
            QFile::remove(dataLogFilename);
        }
        else if (detectedExistingLog.clickedButton() == noButton) {
            promptRenameFile.setText(tr("If you wish to keep the existing file, please rename your data log filename for this operation."));
            promptRenameFile.exec();
            return;
        }
    }

    // set up the data and operation logs
    setup_DataLog();
    setup_OperationLog();

    // if log setup is successful, status boolean is changed to true
    logsReady = true;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - creates and sets up operation log at GUI startup
 */
void MainWindow::setup_OperationLog() {

    // Case #1: GUI is at startup; creates a default startup operation log
    if (atStartUp) {
        qDebug() << "Operation Log Case 1";

        // override any pre-existing startup operation log
        if (QFileInfo::exists(startUpOpLogFilename)) {
            QFile::remove(startUpOpLogFilename);
        }

        opLogFile = new QFile(startUpOpLogFilename);

        // if the operation log cannot be written to at startup, system exits for safety
        if (!opLogFile->open(QIODevice::ReadWrite | QIODevice::Append)) {
            QMessageBox opLogFailure;
            opLogFailure.setIcon(QMessageBox::Critical);
            opLogFailure.setText(tr("[Error] Failed to initialize system log due to unknown error. Exiting system..."));
            opLogFailure.exec();

            opLogFile->remove();
            this->close();
            return;
        }

        opLogEntry = "Magneto v0.03 Operation Log - created " +
                QDate::currentDate().toString("MM-dd-yy") + ", " +
                QTime::currentTime().toString();

        QTextStream outputStream(opLogFile);
        outputStream << opLogEntry << "\r\n" << "\r\n";
        opLogFile->close();

        // write header line to the Operation Log Tab and update status boolean
        ui->plainTextEdit_OperationLog->appendPlainText(opLogEntry + "\n");

        atStartUp = false;
    }
    else {

        // Case #2: case is triggered by the 1st operation run; only the startup operation log exists at this point
        if (QFileInfo::exists(startUpOpLogFilename) && !QFileInfo::exists(opLogFilename)) {
            qDebug() << "Operation Log Case 2";

            QFile::copy(startUpOpLogFilename, opLogFilename);
            QFile::remove(startUpOpLogFilename);

            opLogFile = new QFile(opLogFilename);
        }
        // Case #3: case is triggered if the user changes the operation log filepath/filename; only the previous operation log exists at this point;
        // the previous operation log file will be kept
        else if (QFileInfo::exists(prevOpLogFilename) && !QFileInfo::exists(opLogFilename) && !QFileInfo::exists(startUpOpLogFilename)) {
            qDebug() << "Operation Log Case 3";

            QFile::copy(prevOpLogFilename, opLogFilename);

            opLogFile = new QFile(opLogFilename);
            write_OperationLogMsg("Writing to new file.\r\nPrev: "+ prevOpLogFilename + "\r\nNew: " + opLogFilename + "\r\n");
        }
        // Case #4: case is triggered if continuing to write (appending) to the current operation log file
        else {
            qDebug() << "Operation Log Case 4";
            opLogFile = new QFile(opLogFilename);
        }

    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - creates and sets up data log
 */
void MainWindow::setup_DataLog() {
    dataLogFile = new QFile(dataLogFilename);

    // form the header row with data fields
    QStringList headerRow;
    headerRow << "Time (s)"
              << "Init. X-Pos (mm)"
              << "Init. Y-Pos (mm)"
              << "Final X-Pos (mm)"
              << "Final Y-Pos (mm)"
              << "Dist. Moved (mm)"
              << "Velocity (mm/s)"
              << "Plus X Current Scale"
              << "Plus X Current Duration"
              << "Minus X Current Scale"
              << "Minus X Current Duration"
              << "Plus Y Current Scale"
              << "Plus Y Current Duration"
              << "Minus Y Current Scale"
              << "Minus Y Current Duration"
              << "Total Commands Sent"
              << "Target Location X-Pos"
              << "Target Location Y-Pos";

    // write header row to the data log file and update status boolean
    write_DataLogMsg(headerRow);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - sets up graphics scenes for path drawing functionality
 */
void MainWindow::setup_GraphicsScene() {

    // steps to keep the graphics view fixed (no scrolling) with proper rendering
    ui->graphicsView_Path->setEnabled(true);
    ui->graphicsView_Path->setFixedSize(ui->imageViewer_GL->size());
    ui->graphicsView_Path->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView_Path->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView_Path->setSceneRect(ui->imageViewer_GL->rect());
    ui->graphicsView_Path->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    ui->graphicsView_Path->show();

    /* steps to set up graphics scene (if inactive within the graphics view);
     * for safety, invalidate the previous scene first */
    if (!pathScene->isActive()) {
        pathScene->invalidate();
        pathScene = new QGraphicsScene();
        pathScene->setSceneRect(ui->graphicsView_Path->rect());
        ui->graphicsView_Path->setScene(pathScene);
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - several safety checks to ensure system is ready to perform
 * a delivery operation
 */
void MainWindow::setup_Operation() {

    // set up data and operation logs; if logs are still not setup, return safely
    if (!logsReady) {this->setup_Logs(); }
    if (!logsReady) {return;}

    // set up control module
    if (!opControl->setup_PathTraversal(pathPtMarkers, totalPathDistance, ui->doubleSpinBox_PathTolerance->value(), ui->doubleSpinBox_InterpolationAmount->value())) {
        write_OperationLogMsg(ERROR_FORMAT("Control module is not set up. Cannot start operation."));
        return;
    }

    // ensure camera is connected
    if (!cameraReady) {
        write_OperationLogMsg(ERROR_FORMAT("Camera is not connected. Cannot start operation."));
        return;
    }

    /* ensure motor controllers are connected; safer to use the Physics Module function than
     * than the motorControllersReady status boolean */
    if (!physicsModule->motorsAvailable()) {
        write_OperationLogMsg(ERROR_FORMAT("Motor controllers are not connected. Cannot start operation."));
        return;
    }

    // ensure the system is calibrated
    if (!worker->imageSegmenter.isCalibrated()) {
        write_OperationLogMsg(ERROR_FORMAT("System has not been calibrated. Cannot start operation."));
        return;
    }

    // feed (latest) coil locations to the Physics Module
    physicsModule->setCoilLocs(worker->imageSegmenter.get_CoilLocations());

    // in case particle detection is off
    worker->toggle_segmentFrames(true);

    operationInProgress = true;
    ui->pushButton_StartPauseOperation->setIcon(QIcon(":/resources/Icons/Pause_Icon.png"));
    set_InOpWidgetStates();

    if (!logTimer->isValid()) {
        logTimer->restart();
    }

    emit send_RestartOpPrompt();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - updates username after the user types in the respective
 * line edit
 */
void MainWindow::change_UserName() {

    // GUEST or guest is default; results in no modification to log filenames
    QString prevUserName = userName;
    userName = ui->lineEdit_UserName->text();
    if (userName == "GUEST" || userName == "guest") {userName = "";}

    QStringList userNameStrList_wo_spaces = userName.split(" ");
    userName = userNameStrList_wo_spaces.join("_");

    // update default log filenames
    if (userName == "") {
        defaultDataLogFilename = saveFolder + "/" + QDate::currentDate().toString("MM-dd-yy") + "_dataLog.csv";
        defaultOpLogFilename = saveFolder + "/" + QDate::currentDate().toString("MM-dd-yy") + "_opLog.txt";
    }
    else {
        defaultDataLogFilename = saveFolder + "/" + QDate::currentDate().toString("MM-dd-yy") + "_dataLog_" + userName + ".csv";
        defaultOpLogFilename = saveFolder + "/" + QDate::currentDate().toString("MM-dd-yy") + "_opLog_" + userName + ".txt";
    }

    // updating the text in the log filename line edits
    QString dataLogStr;
    QString opLogStr;

    if (prevUserName != "") {
        dataLogStr = ui->lineEdit_FilenameDataLog->text().split("_" + prevUserName).join("").split(".csv").join("");
        opLogStr = ui->lineEdit_FilenameOpLog->text().split("_" + prevUserName).join("").split(".txt").join("");
    }
    else {
        dataLogStr = ui->lineEdit_FilenameDataLog->text().split(".csv").join("");
        opLogStr = ui->lineEdit_FilenameOpLog->text().split(".txt").join("");
    }

    if (userName != "") {
        ui->lineEdit_FilenameDataLog->setText(dataLogStr + "_" + userName + ".csv");
        ui->lineEdit_FilenameOpLog->setText(opLogStr + "_" + userName + ".txt");
        ui->lineEdit_UserName->setText(userName);
    }
    else {
        ui->lineEdit_FilenameDataLog->setText(dataLogStr + ".csv");
        ui->lineEdit_FilenameOpLog->setText(opLogStr + ".txt");
        ui->lineEdit_UserName->setText("GUEST");
    }

    // also update the log filenames
    change_DataLogFilename();
    change_OperationLogFilename();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - updates save directory after the user finishes
 * browsing the new folder
 */
void MainWindow::change_SaveFolder() {
    QString selectedFolder = QFileDialog::getExistingDirectory(this,
                                                               "Select the directory to export this session's data.",
                                                               QDir::currentPath(),
                                                               QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (selectedFolder.isEmpty() || selectedFolder.isNull()) {return;}
    else {saveFolder = selectedFolder;}

    worker->receive_SaveFolder(saveFolder);
    ui->lineEdit_SaveDirectory->setText(saveFolder);

    // also update the log filenames
    change_DataLogFilename();
    change_OperationLogFilename();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - updates data log filename after the user types
 * in the respective line edit
 */
void MainWindow::change_DataLogFilename() {

    // removing all .csv extensions in the typed filename (for safety)
    int numCsvExtensionsInString = 0;
    int pos = ui->lineEdit_FilenameDataLog->text().indexOf(".csv");

    while (pos != -1) {
        numCsvExtensionsInString += 1;
        pos = ui->lineEdit_FilenameDataLog->text().indexOf(".csv", pos+1);
    }

    if (numCsvExtensionsInString != 1 || ui->lineEdit_FilenameDataLog->text().mid(ui->lineEdit_FilenameDataLog->text().length()-4) != ".csv") {
        ui->lineEdit_FilenameDataLog->setText(ui->lineEdit_FilenameDataLog->text().split(".csv").join("") + ".csv");
    }

    dataLogFilename = saveFolder + "/" + ui->lineEdit_FilenameDataLog->text();

    if (dataLogFilename == defaultDataLogFilename && opLogFilename == defaultOpLogFilename) {
        ui->checkBox_UseDefaultFilenames->setEnabled(false);
        ui->checkBox_UseDefaultFilenames->setCheckState(Qt::Checked);
    }
    else {
        ui->checkBox_UseDefaultFilenames->setCheckState(Qt::Unchecked);
        ui->checkBox_UseDefaultFilenames->setEnabled(true);
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - updates operation log filename after the user types
 * in the respective line edit
 */
void MainWindow::change_OperationLogFilename() {

    // removing all .txt extensions in the typed filename (for safety)
    int numExtensions = 0;
    int pos = ui->lineEdit_FilenameOpLog->text().indexOf(".txt");

    while (pos != -1) {
        numExtensions += 1;
        pos = ui->lineEdit_FilenameOpLog->text().indexOf(".txt", pos+1);
    }

    if (numExtensions != 1 || ui->lineEdit_FilenameOpLog->text().mid(ui->lineEdit_FilenameOpLog->text().length()-4) != ".txt") {
        ui->lineEdit_FilenameOpLog->setText(ui->lineEdit_FilenameOpLog->text().split(".txt").join("") + ".txt");
    }

    // updates the previous and current log filenames
    QString pendingOpLogFilename = saveFolder + "/" + ui->lineEdit_FilenameOpLog->text();
    if ((QFileInfo::exists(opLogFilename)) && (opLogFilename != pendingOpLogFilename)) {prevOpLogFilename = opLogFilename;}
    opLogFilename = pendingOpLogFilename;

    if (dataLogFilename == defaultDataLogFilename && opLogFilename == defaultOpLogFilename) {
        ui->checkBox_UseDefaultFilenames->setEnabled(false);
        ui->checkBox_UseDefaultFilenames->setCheckState(Qt::Checked);
    }
    else {
        ui->checkBox_UseDefaultFilenames->setCheckState(Qt::Unchecked);
        ui->checkBox_UseDefaultFilenames->setEnabled(true);
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - resets log filenames to default format; tied in connection
 * with a checkbox
 */
void MainWindow::reset_LogFilenames(bool useDefaultFilenames) {
    if (useDefaultFilenames) {
        dataLogFilename = defaultDataLogFilename;
        opLogFilename = defaultOpLogFilename;

        if (userName == "") {
            ui->lineEdit_FilenameDataLog->setText(QDate::currentDate().toString("MM-dd-yy") + "_dataLog.csv");
            ui->lineEdit_FilenameOpLog->setText(QDate::currentDate().toString("MM-dd-yy") + "_opLog.txt");
        }
        else {
            ui->lineEdit_FilenameDataLog->setText(QDate::currentDate().toString("MM-dd-yy") + "_dataLog_" + userName + ".csv");
            ui->lineEdit_FilenameOpLog->setText(QDate::currentDate().toString("MM-dd-yy") + "_opLog_" + userName + ".txt");
        }

        ui->checkBox_UseDefaultFilenames->setEnabled(false);
        ui->checkBox_UseDefaultFilenames->setCheckState(Qt::Checked);
    }
    else {
        ui->checkBox_UseDefaultFilenames->setCheckState(Qt::Unchecked);
        ui->checkBox_UseDefaultFilenames->setEnabled(true);
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - connects the camera to the GUI
 */
void MainWindow::connect_Camera() {

    /* for safety, temporarily disable the ability to connect the motor controllers until
     * the camera is completely connected */
    ui->pushButton_ConnectMotorControllers->setEnabled(false);

    // user enters the camera connection port (typically 0 for webcam and 1 for an external camera, but user needs to experiment to make sure)
    QInputDialog *cameraPortSelection = new QInputDialog();
    bool oKPressed = false;
    int cameraPort = cameraPortSelection->getInt(nullptr,"", "Select Camera Port", 1, 0, 10, 1, &oKPressed, Qt::SubWindow);
    if (!oKPressed) {
        re_enable_ConnectMCButton();
        return;
    }

    this->setCursor(Qt::WaitCursor);

    worker->loadStream(cameraPort);
    worker->set_ResizeFactorUsingDisplayHeight(ui->imageViewer_GL->height());

    // update status boolean and UI on whether the camera is successfully connected
    cameraReady = worker->isStreaming();
    if (cameraReady) {
        ui->lineEdit_CameraStatus->setStyleSheet("QLineEdit {font: 11pt \"Segoe UI\";"
                                                 "color: rgb(0,100,0);"
                                                 "background-color: rgb(255, 255, 240);}");
        ui->lineEdit_CameraStatus->setText("Connected: Port " + QString::number(cameraPort));
    }
    else {
        ui->lineEdit_CameraStatus->setStyleSheet("QLineEdit {font: 11pt \"Segoe UI\"; "
                                                 "color: rgb(200,0,0);"
                                                 "background-color: rgb(255, 255, 240);}");
        ui->lineEdit_CameraStatus->setText("Failed to connect. Try again.");
    }

    // enable ability to connect motor controllers after a short delay
    QTimer::singleShot(300, this, SLOT(re_enable_ConnectMCButton()));

    // enable settings window only after both camera and motor controllers are connected
    if (cameraReady && motorControllersReady) {
        ui->pushButton_SettingsWindow->setEnabled(true);
    }

    this->setCursor(Qt::ArrowCursor);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - connects the roboclaw motor controllers to the GUI
 */
void MainWindow::connect_MotorControllers() {

    /* for safety, temporarily disable the ability to connect the camera until
     * the motor controllers are completely connected */
    ui->pushButton_ConnectCamera->setEnabled(false);

    /* this constructs an object of the Physics Module; make sure the USBs for roboclaws
     * are connected to the computer (otherwise the GUI will crash here) */
    if (physicsModule == nullptr) {
        physicsModule = new Physics();
        connect(physicsModule, SIGNAL(send_OperationLogMsg(QString)), this, SLOT(write_OperationLogMsg(QString)));
    }

    this->setCursor(Qt::WaitCursor);

    // update status boolean and UI on whether the motor controllers successfully connected
    motorControllersReady = physicsModule->motorsAvailable();
    if (motorControllersReady) {
        ui->lineEdit_MotorControllerStatus->setStyleSheet("QLineEdit {font: 11pt \"Segoe UI\";"
                                                          "color: rgb(0,100,0);"
                                                          "background-color: rgb(255, 255, 240);}");
        ui->lineEdit_MotorControllerStatus->setText("Connected: RoboX " + physicsModule->getComPortXName() +
                                                    "; RoboY " + physicsModule->getComPortYName());
    }
    else {
        ui->lineEdit_CameraStatus->setStyleSheet("QLineEdit {font: 11pt \"Segoe UI\";"
                                                 "color: rgb(200,0,0);"
                                                 "background-color: rgb(255, 255, 240);}");
        ui->lineEdit_MotorControllerStatus->setText("Failed to connect. Try again.");
    }

    // enable ability to connect camera after a short delay
    QTimer::singleShot(300, this, SLOT(re_enable_ConnectCameraButton()));

    // enable settings window only after both camera and motor controllers are connected
    if (motorControllersReady && cameraReady) {
        ui->pushButton_SettingsWindow->setEnabled(true);
    }

    this->setCursor(Qt::ArrowCursor);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - used for safely reenabling the Connect Camera button
 * after connecting motor controllers
 */
void MainWindow::re_enable_ConnectCameraButton() {
    ui->pushButton_ConnectCamera->setEnabled(true);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - used for safely reenabling the Connect MC button
 * after connecting the camera
 */
void MainWindow::re_enable_ConnectMCButton() {
    ui->pushButton_ConnectMotorControllers->setEnabled(true);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - processing function to ensure camera image display
 * on the viewport
 */
void MainWindow::receive_FrameforDisplay(cv::Mat *frame) {
    /* during operation, a vector from the current particle location to the
     * target location is displayed and updated continually */
    if (ui->imageViewer_GL->isDisplayingTargetVector()) {
        ui->imageViewer_GL->set_TargetVector(worker->imageSegmenter.get_CurrentParticleLoc_Pixel()*screenResizeFactor,
                                             opControl->get_targetPathPointData().pixelScreen);
    }

    // this is what displays the image
    ui->imageViewer_GL->set_Image(frame);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - initiates system calibration after performing
 * camera safety checks
 */
void MainWindow::receive_CalibrationPrompt() {
    if (!cameraReady) {this->connect_Camera();}
    else if (!worker->isStreaming()) {worker->toggle_streaming(true);}

    worker->toggle_calibrateFrames(true);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - displays updated field-of-view values calculated from
 * image processing
 */
void MainWindow::receive_UpdatedFOV(double width_FOV, double height_FOV) {
    ui->label_CameraFOV->setText(QString::number(static_cast<int>(width_FOV)) + " x " + QString::number(static_cast<int>(height_FOV)) + " mm");
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - initiates particle detection functionality of image
 * processing code after performing camera safety checks; enables path drawing
 * after particle detection is initiated
 */
void MainWindow::receive_ParticleDetectionPrompt() {
    if (!cameraReady) {this->connect_Camera();}
    else if (!worker->isStreaming()) {worker->toggle_streaming(true);}

    worker->toggle_segmentFrames(true);

    if (!ui->pushButton_GoToPathDrawing->isEnabled()) {
        ui->pushButton_GoToPathDrawing->setEnabled(true);
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - toggles path drawing
 */
void MainWindow::receive_DrawPathPrompt() {

    // if-statements are safety-checks
    if (!cameraReady || !worker->imageSegmenter.isCalibrated()) {return;}
    if (!pathScene->isActive()) {setup_GraphicsScene();}

    // Draw Mode
    if (!drawPathMode) {
        ui->graphicsView_Path->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        ui->pushButton_DrawPath->setText("Pause Draw");
    }
    // No-Draw Mode
    else {
        ui->graphicsView_Path->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        ui->pushButton_DrawPath->setText("Draw Path");
    }

    // toggles status boolean (from draw to no-draw, from no-draw to draw)
    drawPathMode = !drawPathMode;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - receives the current translation command from Physics Module
 */
void MainWindow::receive_TranslationCommandInfo(std::vector<int> translationCommandArray) {
    this->translationCommandArray = translationCommandArray;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - opens or raises the settings window into view
 */
void MainWindow::open_SettingsWindow() {
    if (settingsWindow->isVisible()) {settingsWindow->raise();}
    else {settingsWindow->show();}
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - enables and switches to the Path Drawing Tab; initializes the
 * graphics scene
 */
void MainWindow::enable_PathDrawingTab() {
    ui->tabWidget->setTabEnabled(2, true);
    ui->tabWidget->setCurrentIndex(2);

    setup_GraphicsScene();
    ui->pushButton_GoToPathDrawing->hide();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION -  allows the user to start an operation
 * (only allows! doesn't actually start the operation)
 */
void MainWindow::enable_Operation() {
    set_OpControlPanel(true);
    enable_PathDrawingTab();

    ui->pushButton_GoToOperation->hide();
    reset_ProgressUpdatesTab();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - called 4 times per system calibration to move
 * the particle around the petri dish in a diamond shape for image
 * averaging purposes
 */
void MainWindow::move_ParticleForCalibration() {

    // safety check on motor controllers
    if (!physicsModule->motorsAvailable()) {
        write_OperationLogMsg(ERROR_FORMAT("Motor controllers not connected. Cannot continue calibration."));
        stop_ParticleMovementForCalibration();
        return;
    }

    /* amount of current scale found adequate to move the particle
     * to each corner of the diamond trajectory */
    uint8_t currentScale = 100;

    // starting point of diamond is at -X coil
    if (calibrationCycler == -1) {
        physicsModule->translateParticle(0,0,127,300,0,0,0,0);
        calibrationCycler = 0;
        return;
    }

    // 1st: move to +Y coil
    if (calibrationCycler == 0) {
        physicsModule->translateParticle(0,0,0,0,currentScale,250,0,0);
        calibrationCycler += 1;
    }
    // 2nd: move to +X coil
    else if (calibrationCycler == 1) {
        physicsModule->translateParticle(currentScale,250,0,0,0,0,0,0);
        calibrationCycler += 1;
    }
    // 3rd: move to -Y coil
    else if (calibrationCycler == 2) {
        physicsModule->translateParticle(0,0,0,0,0,0,currentScale,250);
        calibrationCycler += 1;
    }
    // 4th: move back to -X coil
    else if (calibrationCycler == 3) {
        physicsModule->translateParticle(0,0,currentScale,250,0,0,0,0);
        calibrationCycler = 0;
    }

    // after each translation, notify system to take a picture for image averaging
    emit send_SynthesizeCleanImagePrompt();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - ends particle movement for system calibration
 */
void MainWindow::stop_ParticleMovementForCalibration() {
    particleDetectionCalibrationTimer->stop();
    calibrationCycler = -1;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - draws a path point marker (and connecting lines)
 * where the user clicks on the graphics scene
 */
void MainWindow::add_PathMarker(QPointF mkrPos) {

    // safety check
    if (!pathScene->isActive()) {return;}

    PathPointMarker *mkr;

    // 1st marker
    if (pathPtMarkers.empty()) {
        mkr = new PathPointMarker(nullptr,nullptr);
        mkr->set_PathStart_PathEnd(true, true);
    }
    // 2nd marker
    else if (pathPtMarkers.size() == 1){
        mkr = new PathPointMarker(pathPtMarkers.back(),nullptr);
        QGraphicsLineItem *prevLine = new QGraphicsLineItem();
        mkr->set_PrevLine(prevLine);
        mkr->set_PathStart_PathEnd(false, true);

        pathScene->addItem(prevLine);

        // update references of the 1st marker
        pathPtMarkers.back()->set_NextMkr(mkr);
        pathPtMarkers.back()->set_NextLine(mkr->get_PrevLineRef());
        pathPtMarkers.back()->set_PathStart_PathEnd(true, false);
        pathPtMarkers.back()->get_TextLabelRef()->setText("Start");
    }
    // Any other marker
    else {
        mkr = new PathPointMarker(pathPtMarkers.back(),nullptr);
        QGraphicsLineItem *prevLine = new QGraphicsLineItem();
        QGraphicsLineItem *nextLine = new QGraphicsLineItem();
        mkr->set_PrevLine(prevLine);
        mkr->set_NextLine(nextLine);
        mkr->set_PathStart_PathEnd(false, true);

        pathScene->addItem(prevLine);
        pathScene->addItem(nextLine);

        // update references of the current back of the path
        pathPtMarkers.back()->set_NextMkr(mkr);
        pathPtMarkers.back()->set_NextLine(mkr->get_PrevLineRef());
        pathPtMarkers.back()->set_PathStart_PathEnd(false, false);
        pathPtMarkers.back()->get_TextLabelRef()->setText("");
    }

    pathScene->addItem(mkr);

    /* center the marker position around mouse cursor
     * (otherwise Qt would place the top-left of the marker at the cursor) */
    mkr->setPos(mkrPos.x()-(mkr->rect().width()/2.0),
                mkrPos.y()-(mkr->rect().height()/2.0));

    connect(mkr, SIGNAL(send_AllowNewMarker(bool)), ui->graphicsView_Path, SLOT(toggle_AllowLeftMouseClick(bool)));
    connect(mkr, SIGNAL(send_PositionChanged(PathPointMarker *)), this, SLOT(calibrate_PathMarker(PathPointMarker *)));
    calibrate_PathMarker(mkr);

    // make the marker the new back of the path
    pathPtMarkers.push_back(mkr);
    numPathMarkers += 1;
    ui->label_NumPathMarkers->setText(QString::number(numPathMarkers) + " markers");
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - removes the most recently drawn path marker from the path
 * and graphics scene
 */
void MainWindow::remove_MostRecentPathMarker() {
    if (pathScene->items().empty() || pathPtMarkers.empty()) {return;}

    // remove the line connecting the marker and the previous marker
    if (pathPtMarkers.size() != 1) {
        pathScene->removeItem(pathPtMarkers.back()->get_PrevLineRef());
    }

    // case of ending with only 1 path marker left
    if (pathPtMarkers.size() == 2) {
        pathPtMarkers.at(pathPtMarkers.size()-2)->set_PathStart_PathEnd(true, true);
    }
    // all other cases for > 1 marker
    else if (pathPtMarkers.size() != 1){
        pathPtMarkers.at(pathPtMarkers.size()-2)->set_PathStart_PathEnd(false, true);
    }

    pathScene->removeItem(pathPtMarkers.back());
    pathScene->removeItem(pathPtMarkers.back()->get_TextLabelRef());

    pathPtMarkers.pop_back();
    numPathMarkers -= 1;
    ui->label_NumPathMarkers->setText(QString::number(numPathMarkers) + " markers");
    update_CurrentPathDistance();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - updates path marker location data
 */
void MainWindow::calibrate_PathMarker(PathPointMarker *mkr) {
    mkr->data->pixelNative = mkr->data->pixelScreen / screenResizeFactor;

    mkr->data->physical =
            worker->imageSegmenter.map_ToTrueCoordinates(mkr->data->pixelNative)
            * worker->imageSegmenter.get_DistancePerPixel();

    update_CurrentPathDistance();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - resets the path markers to unpassed status (readies
 * them for the next operation)
 */
void MainWindow::reset_PathStatusToUnpassed() {
    for (int i = 0; i < static_cast<int>(pathPtMarkers.size()); i++) {
        pathPtMarkers.at(static_cast<unsigned long long>(i))->set_UnpassedStatus();
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - sets the designed path marker to passed status so that
 * the user can no longer modify it until the operation is stopped; is a
 * safety feature
 */
void MainWindow::update_PathMarkerStatusToPassed(int mkrIndex) {
    pathPtMarkers.at(static_cast<unsigned long long>(mkrIndex))->set_PassedStatus();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - calculates the current path distance (called when user adds
 * or moves a path marker
 */
void MainWindow::update_CurrentPathDistance() {
    if (pathPtMarkers.size() <= 1) {return;}

    totalPathDistance = 0.0;
    Point pt1;
    Point pt2;

    // calculate pixel distance over entire path
    for (int i = 1; i < static_cast<int>(pathPtMarkers.size()); i++) {
        pt1 = Point(pathPtMarkers.at(static_cast<unsigned long long>(i))->data->physical);
        pt2 = Point(pathPtMarkers.at(static_cast<unsigned long long>(i-1))->data->physical);
        totalPathDistance += Point::computeEuclideanDist(pt1,pt2);
    }

    ui->label_PathDistance->setText(QString::number(totalPathDistance) + " mm");
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - clears the current drawn path (clears path data and graphics scene display)
 */
void MainWindow::clear_Path() {

    // safety check
    if (!pathScene->isActive() || pathPtMarkers.size() == 0) {return;}

    // prompts user to confirm path clearing
    QMessageBox clearConfirm;
    clearConfirm.setIcon(QMessageBox::Question);
    clearConfirm.setWindowTitle(tr("Clear current path?"));
    clearConfirm.setText(tr("Are you sure you want to clear the current path?"));
    QAbstractButton *yesButton = clearConfirm.addButton(tr("Yes"), QMessageBox::YesRole);
    clearConfirm.addButton(tr("No"), QMessageBox::NoRole);

    clearConfirm.exec();

    if (clearConfirm.clickedButton() == yesButton) {
        ui->graphicsView_Path->items().clear();
        pathPtMarkers.clear();
        pathScene->clear(); // deletes the marker objects on the sceene

        numPathMarkers = 0;
        ui->label_NumPathMarkers->setText(QString::number(numPathMarkers) + " markers");
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - loads a previously drawn path (updates path data and graphics scene display)
 */
void MainWindow::load_Path() {

    // safety checks
    if (!cameraReady || !worker->imageSegmenter.isCalibrated()) {return;}
    if (!pathScene->isActive()) {return;}

    // prompt user to confirm if he/she wants to override any currently drawn path with the loaded path
    if (pathPtMarkers.size() != 0) {
        QMessageBox loadConfirm;
        loadConfirm.setIcon(QMessageBox::Question);
        loadConfirm.setWindowTitle(tr("Override current path?"));
        loadConfirm.setText(tr("Loading a prexisting path will override the current path. Are you sure you want to proceed?"));
        QAbstractButton *yesButton = loadConfirm.addButton(tr("Yes"), QMessageBox::YesRole);
        loadConfirm.addButton(tr("No"), QMessageBox::NoRole);

        loadConfirm.exec();
        if (loadConfirm.clickedButton() != yesButton) {return;}
    }

    // load the path
    loadedPathFilename = QFileDialog::getOpenFileName(this,"Select the .path image.", QDir::currentPath(), "Path File (*.path)");
    if (loadedPathFilename.isNull() || loadedPathFilename.isEmpty()) {return; }
    else {loadedPathFile = new QFile(loadedPathFilename);}

    // clear current path
    if (pathPtMarkers.size() > 0) {clear_Path();}

    // successful reading of path file
    if (loadedPathFile->open(QIODevice::ReadOnly)) {
        QTextStream in(loadedPathFile);
        QStringList line;
        Point loadedPt;

        // safety measure
        if (!pathScene->isActive()) {setup_GraphicsScene();}

        // read through the path file and load in the path markers
        while (!in.atEnd()) {

            line = in.readLine().split(",");
            loadedPt = Point(line.at(0).toDouble(), line.at(1).toDouble());

            // convert physical coordinates to screen coordinates
            loadedPt /= worker->imageSegmenter.get_DistancePerPixel();
            loadedPt = worker->imageSegmenter.unmap_FromTrueCoordinates(loadedPt);
            loadedPt *= screenResizeFactor;

            add_PathMarker(loadedPt);
        }

        ui->statusBar->showMessage(QTime::currentTime().toString() + " - Path loaded!", 3000);
    }
    // failed reading of path file
    else {
        QMessageBox loadFailed;
        loadFailed.setIcon(QMessageBox::Warning);
        loadFailed.setWindowTitle(tr("Read failed for .path file"));
        loadFailed.setText(tr("The selected .path file may be corrupt. Please select another."));
        loadFailed.addButton(QMessageBox::Ok);
        loadFailed.exec();
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - save the currently drawn path into a .path file
 */
void MainWindow::save_Path() {

    // safety checks
    if (!cameraReady || !worker->imageSegmenter.isCalibrated()) {return;}
    if (!pathScene->isActive() || pathPtMarkers.size() == 0) {return;}

    // prompt user to confirm if he/she wants to save the current path
    QMessageBox saveConfirm;
    saveConfirm.setIcon(QMessageBox::Question);
    saveConfirm.setWindowTitle(tr("Save current path?"));
    saveConfirm.setText(tr("Are you sure you want to save the current path into a file?"));
    QAbstractButton *yesButton = saveConfirm.addButton(tr("Yes"), QMessageBox::YesRole);
    saveConfirm.addButton(tr("No"), QMessageBox::NoRole);

    saveConfirm.exec();
    if (saveConfirm.clickedButton() != yesButton) {return;}

    savedPathFilename = QFileDialog::getSaveFileName(this,
                                                     "Select the save location for the current path.",
                                                     "/" + saveFolder+ "/" + QDate::currentDate().toString("MM-dd-yy") + "_Path",
                                                     "Path File (*.path)");
    if (savedPathFilename.isNull() || savedPathFilename.isEmpty()) {return;}

    savedPathFile = new QFile(savedPathFilename);

    // successful saving of path file
    if (savedPathFile->open(QIODevice::WriteOnly)) {
        QTextStream out(savedPathFile);

        for (int i = 0; i < static_cast<int>(pathPtMarkers.size()); i++) {
            out << QString::number(pathPtMarkers.at(static_cast<unsigned long long>(i))->data->physical.x()) +
                   "," +
                   QString::number(pathPtMarkers.at(static_cast<unsigned long long>(i))->data->physical.y()) +
                   "\r\n";
        }

        ui->statusBar->showMessage(QTime::currentTime().toString() + " - Path saved!", 3000);
    }
    // failed saving of path file
    else {
        QMessageBox saveFailed;
        saveFailed.setIcon(QMessageBox::Warning);
        saveFailed.setWindowTitle(tr("Write failed for .path file"));
        saveFailed.setText(tr("The system encountered an error trying to save the current path. Please try again."));
        saveFailed.addButton(QMessageBox::Ok);
        saveFailed.exec();
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - starts or pauses the delivery operation
 */
void MainWindow::startPause_Operation() {

    // start/restart case
    if (!operationInProgress) {
        ui->statusBar->showMessage("Setting up for operation...", 3000);
        setup_Operation();

        write_OperationLogMsg("Starting/Resuming Operation");
        ui->label_OperationStatus->setText("In Progress");
    }
    // pause case
    else {
        emit send_HaltTranslationPrompt();

        // keep track of operation time so far
        elapsedLogTime += static_cast<double>(logTimer->elapsed())/1000;
        logTimer->invalidate();

        operationInProgress = false;
        worker->toggle_segmentFrames(false);

        set_PausedOpWidgetStates();

        write_OperationLogMsg("Paused Operation");
        ui->pushButton_StartPauseOperation->setIcon(QIcon(":/resources/Icons/Play_Icon.png"));
        ui->label_OperationStatus->setText("Paused");
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - stops the delivery operation
 */
void MainWindow::stop_Operation() {
    emit send_HaltTranslationPrompt();

    logTimer->invalidate();
    elapsedLogTime = 0.0;

    // discontinues target vector display
    ui->imageViewer_GL->toggle_DisplayTargetVector(false);

    operationInProgress = false;
    logsReady = false;
    commandCounter = 0;

    update_ProgressUpdatesTab();
    opControl->reset_Data();

    ui->statusBar->showMessage("Operation stopped. Total commands executed: " + QString::number(commandCounter));
    ui->pushButton_StartPauseOperation->setIcon(QIcon(":/resources/Icons/Play_Icon.png"));

    set_StoppedOpWidgetStates();
    reset_PathStatusToUnpassed();
    write_OperationLogMsg("Stopped Operation");

    ui->label_OperationStatus->setText("Stopped");
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - force stops delivery operation; currently our system
 * can easily and safety stop operation through halting of a timer; future iterations
 * of system should implement this function differently
 */
void MainWindow::forceStop_Operation() {
    stop_Operation();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - prepares and execuates a particle translation command; key
 * function for delivery operation
 */
void MainWindow::execute_TranslationCommand() {

    // safety check on motor controllers
    if (!physicsModule->motorsAvailable()) {
        write_OperationLogMsg(ERROR_FORMAT("Motor controllers not connected. Cannot continue translation."));
        forceStop_Operation();
        return;
    }

    commandCounter += 1;
    ui->statusBar->showMessage("Command #: " + QString::number(commandCounter));

    // update current particle location and associated data fields
    currentParticleLoc = worker->imageSegmenter.get_CurrentParticleLoc_PhysicalMapped();
    x_init = currentParticleLoc.x();
    y_init = currentParticleLoc.y();

    bool proceedWithTranslation = opControl->allow_NextTranslation(currentParticleLoc);

    // toggle on target vector display
    if (!ui->imageViewer_GL->isDisplayingTargetVector()) {
        ui->imageViewer_GL->toggle_DisplayTargetVector(true);
    }

    // compute the next translation command
    if (proceedWithTranslation) {

        // feedback feature to supplement translation command
        physicsModule->set_Boost(currentParticleLoc, opControl->get_targetPathPointData().physical);

        // use neural network model
        physicsModule->computeNextCommand_NN(currentParticleLoc,opControl->get_targetPathPointData().physical);

        // use surface fitting model --> comment out the previous line if you want to use this
        //        physicsModule->computeNextCommand_SF(currentParticleLoc,opControl->get_targetPathPt());

        // activating motor controllers
        physicsModule->translateParticle();

        // updating data log
        write_DataLogMsg(this->generate_DataEntry());

        // updating info displayed on progress updates tab
        update_ProgressUpdatesTab();
    }
    else {
        ui->imageViewer_GL->toggle_DisplayTargetVector(false);
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - generates all data fields for an entry to be written into the
 * data log
 */
QStringList MainWindow::generate_DataEntry() {

    // clear out the contents of the previous data log entry
    dataLogEntry.clear();

    // acquire current timestamp
    t_prev = t_current;
    t_current = (static_cast<double>(logTimer->elapsed())/1000) + elapsedLogTime;

    // acquire current particle location coordinates
    currentParticleLoc = worker->imageSegmenter.get_CurrentParticleLoc_PhysicalMapped();
    x_final = currentParticleLoc.x();
    y_final = currentParticleLoc.y();

    // compute distance traveled from the current translation command
    distanceMoved = Point::computeEuclideanDist(currentParticleLoc, Point(x_init,y_init));

    // compute velocity from the current translation command
    velocity = distanceMoved/(t_current - t_prev);

    // acquire the current translation command
    translationCommandArray = physicsModule->get_TranslationCommandInfo();

    // acquire current target path point position
    targetLocation = opControl->get_targetPathPointData().physical;

    // forming the data log entry
    dataLogEntry << QString::number(t_current);
    dataLogEntry << QString::number(x_init);
    dataLogEntry << QString::number(y_init);
    dataLogEntry << QString::number(x_final);
    dataLogEntry << QString::number(y_final);
    dataLogEntry << QString::number(distanceMoved);
    dataLogEntry << QString::number(velocity);
    dataLogEntry << QString::number(translationCommandArray[0]);
    dataLogEntry << QString::number(translationCommandArray[1]);
    dataLogEntry << QString::number(translationCommandArray[2]);
    dataLogEntry << QString::number(translationCommandArray[3]);
    dataLogEntry << QString::number(translationCommandArray[4]);
    dataLogEntry << QString::number(translationCommandArray[5]);
    dataLogEntry << QString::number(translationCommandArray[6]);
    dataLogEntry << QString::number(translationCommandArray[7]);
    dataLogEntry << QString::number(commandCounter);
    dataLogEntry << QString::number(targetLocation.x());
    dataLogEntry << QString::number(targetLocation.y());

    return dataLogEntry;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - updates the fields displayed on the Progress Updates tab
 */
void MainWindow::update_ProgressUpdatesTab() {
    ui->label_ElapsedTime->setText(QString::number(t_current));

    ui->label_PathProgress->setText(QString::number(opControl->get_OperationProgress()));
    ui->label_PathDistanceRemaining->setText(QString::number(opControl->get_RemainingPathDistance()));
    ui->label_ParticleLocation->setText(currentParticleLoc.toString());
    ui->label_TargetLocation->setText(targetLocation.toString());

    ui->label_TranslationCommand->setText("(" +
            QString::number(translationCommandArray[0]) + "," +
            QString::number(translationCommandArray[1]) + "," +
            QString::number(translationCommandArray[2]) + "," +
            QString::number(translationCommandArray[3]) + "," +
            QString::number(translationCommandArray[4]) + "," +
            QString::number(translationCommandArray[5]) + "," +
            QString::number(translationCommandArray[6]) + "," +
            QString::number(translationCommandArray[7]) + ")");
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - resets the fields displayed on the Progress Updates tab
 */
void MainWindow::reset_ProgressUpdatesTab() {
    ui->label_OperationStatus->setText("Not Started");
    ui->label_ElapsedTime->setText("0");

    ui->label_PathProgress->setText("0");
    ui->label_PathDistanceRemaining->setText("0");
    ui->label_ParticleLocation->setText("N/A");
    ui->label_TargetLocation->setText("N/A");

    ui->label_TranslationCommand->setText("N/A");
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - writes an entry to the data log
 */
void MainWindow::write_DataLogMsg(QStringList entry) {

    // no writing is performed if data log cannot be opened
    if (!dataLogFile->open(QIODevice::ReadWrite | QIODevice::Append)) {
        if (operationInProgress) {emit send_StopOpPrompt();}
        qDebug() << "[Error] Could not write to the data log file.";
    }
    else {
        QTextStream outputStream(dataLogFile);
        outputStream << entry.join(",") << "\r\n";
        outputStream.flush();
        dataLogFile->close();
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - writes a line to the operation log
 */
void MainWindow::write_OperationLogMsg(QString msg) {
    qDebug().noquote() << msg;

    // no writing is performed if operation log cannot be opened
    if (!opLogFile->open(QIODevice::ReadWrite | QIODevice::Append)) {
        if (operationInProgress) {emit send_StopOpPrompt();}
        qDebug() << "[Error] Could not write to the operation log file.";
    }
    else {
        opLogEntry = QTime::currentTime().toString() + " >> " + msg;

        QTextStream outputStream(opLogFile);
        outputStream << opLogEntry << "\r\n";
        outputStream.flush();
        opLogFile->close();

        // Operation Log Tab displays the contents of the file
        ui->plainTextEdit_OperationLog->appendPlainText(opLogEntry + "\n");

        // Status Bar displayes the most recent message
        ui->statusBar->showMessage(opLogEntry);
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - drop-down functionality of Drawing Instructions menu in
 * the Path Drawing tab
 */
void MainWindow::on_pushButton_ShowHideDrawingInstructions_clicked() {
    if (ui->textEdit_DrawPathInstructions->isVisible()) {
        ui->textEdit_DrawPathInstructions->hide();
        ui->pushButton_ShowHideDrawingInstructions->setText("▼ Show drawing instructions");
    }
    else {
        ui->textEdit_DrawPathInstructions->show();
        ui->pushButton_ShowHideDrawingInstructions->setText("▲ Hide drawing instructions");
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - drop-down functionality of Path Info menu in
 * the Path Drawing tab
 */
void MainWindow::on_pushButton_ShowHidePathInfo_clicked() {
    if (ui->groupBox_PathInfo->isVisible()) {
        ui->groupBox_PathInfo->hide();
        ui->pushButton_ShowHidePathInfo->setText("▼ Show path info");
    }
    else {
        ui->groupBox_PathInfo->show();
        ui->pushButton_ShowHidePathInfo->setText("▲ Hide path info");
    }
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - // hides/displays the drawn path based on the
 * current GUI tab being viewed
 */
void MainWindow::on_tabWidget_currentChanged(int index) {
    if ((index == 2 || index == 3 || index == 4) && ui->graphicsView_Path->isEnabled()) {
        ui->graphicsView_Path->show();
    }
    else if (ui->graphicsView_Path->isEnabled()){
        ui->graphicsView_Path->hide();
    }
}

//------------------------------------------------------------------------
