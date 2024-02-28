/*
 *	Dialog_SettingsWindow source file.
 *
 *
 *	AUTHOR: Victor Huynh
 *
 */
//------------------------------------------------------------------------

#include "dialog_settingswindow.h"
#include "ui_dialog_settingswindow.h"

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - constructor; sets initial widget states, certain window
 * properties, and establishes window connections
 */
Dialog_SettingsWindow::Dialog_SettingsWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_SettingsWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Settings");

    // make settings window of fixed size (make it not resizable)
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setSizeGripEnabled(false);

    this->establish_Connections();
    this->set_InitialWidgetStates();   
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - destructor
 */
Dialog_SettingsWindow::~Dialog_SettingsWindow()
{
    delete ui;
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - establishes all Dialog_SettingsWindow connections within the
 * class
 */
void Dialog_SettingsWindow::establish_Connections() {

    // image processing-related connections
    connect(ui->pushButton_CalibrateSystem, SIGNAL(clicked()), this, SLOT(initiate_SystemCalibration()));
    connect(ui->pushButton_Preview, SIGNAL(clicked()), this, SLOT(preview_ParticleDetection()));

    // general settings connections
    connect(ui->pushButton_ApplySettings, SIGNAL(clicked()), this, SLOT(apply_Settings()));
    connect(ui->pushButton_CancelAndExit, SIGNAL(clicked()), this, SLOT(close()));
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - defines window behavior upon any keypress
 * (which in this case is to ignore keypresses for safety reasons)
 */
void Dialog_SettingsWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key()) {return;}
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - sets initial widget states; called within constructor
 */
void Dialog_SettingsWindow::set_InitialWidgetStates() {
    ui->pushButton_Preview->setEnabled(false);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - sets widget states right before a delivery operation
 * is started; majority of widgets are disabled
 */
void Dialog_SettingsWindow::set_InOpWidgetStates() {
    ui->pushButton_CalibrateSystem->setEnabled(false);
    ui->pushButton_Preview->setEnabled(false);
    ui->pushButton_ApplySettings->setEnabled(false);
    ui->doubleSpinBox_CommandFreq->setEnabled(false);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - sets widget states for when a delivery operation
 * is paused
 */
void Dialog_SettingsWindow::set_PausedOpWidgetStates() {
    ui->pushButton_CalibrateSystem->setEnabled(false);

    ui->pushButton_Preview->setEnabled(true);
    ui->pushButton_ApplySettings->setEnabled(true);
    ui->doubleSpinBox_CommandFreq->setEnabled(true);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - sets widget states for when a delivery operation
 * is stopped
 */
void Dialog_SettingsWindow::set_StoppedOpWidgetStates() {
    ui->pushButton_CalibrateSystem->setEnabled(true);
    ui->pushButton_Preview->setEnabled(true);
    ui->pushButton_ApplySettings->setEnabled(true);
    ui->doubleSpinBox_CommandFreq->setEnabled(true);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - sets up initial values for the window's
 * particle detection spinboxes and sldiers
 */
void Dialog_SettingsWindow::setup_InitialParticleDetectionParams(int initialFilterThreshold,
                                                                 int initialMinSize,
                                                                 int initialMaxSize) {

    // spinbox setup
    ui->spinBox_FilterThreshold->setValue(initialFilterThreshold);
    ui->spinBox_MinParticleSize->setValue(initialMinSize);
    ui->spinBox_MaxParticleSize->setValue(initialMaxSize);

    // slider setup
    ui->horizontalSlider_FilterThreshold->setValue(initialFilterThreshold);
    ui->horizontalSlider_MinParticleSize->setValue(initialMinSize);
    ui->horizontalSlider_MaxParticleSize->setValue(initialMaxSize);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - initiates system calibration process
 */
void Dialog_SettingsWindow::initiate_SystemCalibration() {

    // for safety
    ui->pushButton_Preview->setEnabled(false);

    emit send_CalibrationPrompt();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - initiates particle detection using set parameters
 */
void Dialog_SettingsWindow::preview_ParticleDetection() {

    // sends updated particle detection parameters
    emit send_ParticleDetectionParams(ui->spinBox_FilterThreshold->value(),
                             ui->spinBox_MinParticleSize->value(),
                             ui->spinBox_MaxParticleSize->value());

    // updates window content
    ui->label_LastUpdatePreview->setText(QTime::currentTime().toString("hh:mm::ss"));
    ui->label_FilterThresholdPreview->setText(QString::number(ui->spinBox_FilterThreshold->value()));
    ui->label_MinParticleSizePreview->setText(QString::number(ui->spinBox_MinParticleSize->value()));
    ui->label_MaxParticleSizePreview->setText(QString::number(ui->spinBox_MaxParticleSize->value()));

    // initiates particle detection
    emit send_PreviewParticleDetectionPrompt();
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - enables the button that initiates particle detection
 */
void Dialog_SettingsWindow::enable_ParticleDetectionPreview() {
    ui->pushButton_Preview->setEnabled(true);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - applies settings to the program
 */
void Dialog_SettingsWindow::apply_Settings() {

    // redundant to send this signal, but is just for safety
    emit send_ParticleDetectionParams(ui->spinBox_FilterThreshold->value(),
                             ui->spinBox_MinParticleSize->value(),
                             ui->spinBox_MaxParticleSize->value());

    // update hardware timer timeout
    emit send_HardwareCommandTimeout(static_cast<int>(1.0/ui->doubleSpinBox_CommandFreq->value()*1000));

    // update whether or not to display system axes and bounding box
    emit send_DisplaySystemAxesPrompt(ui->checkBox_DisplaySystemAxes->isChecked());

    // for user convenience, note the last time that the settings were applied
    ui->label_ChangesApplied->setText("Last Applied -> " + QTime::currentTime().toString("hh:mm::ss"));
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - triggered by value update of filter threshold slider;
 * updates the filter threshold spinbox value
 */
void Dialog_SettingsWindow::on_horizontalSlider_FilterThreshold_valueChanged(int value) {
    ui->spinBox_FilterThreshold->setValue(value);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - triggered by value update of minimum particle size slider;
 * updates the minimum particle size spinbox value
 */
void Dialog_SettingsWindow::on_horizontalSlider_MinParticleSize_valueChanged(int value) {

    // ensures that the minimum particle size is no greater than the maximum particle size
    if (ui->horizontalSlider_MinParticleSize->value() >= ui->horizontalSlider_MaxParticleSize->value()) {
        ui->horizontalSlider_MaxParticleSize->setValue(value+1);
    }
    ui->spinBox_MinParticleSize->setValue(value);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - triggered by value update of maximum particle size slider;
 * updates the maximum particle size spinbox value
 */
void Dialog_SettingsWindow::on_horizontalSlider_MaxParticleSize_valueChanged(int value) {

    // ensures that the maximum particle size is no lesser than the maximum particle size
    if (ui->horizontalSlider_MaxParticleSize->value() <= ui->horizontalSlider_MinParticleSize->value()) {
        ui->horizontalSlider_MinParticleSize->setValue(value-1);
    }
    ui->spinBox_MaxParticleSize->setValue(value);
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - triggered by value update of filter threshold spinbox;
 * updates the filter threshold slider value
 */
void Dialog_SettingsWindow::on_spinBox_FilterThreshold_editingFinished() {
    ui->horizontalSlider_FilterThreshold->setValue(ui->spinBox_FilterThreshold->value());
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - triggered by value update of minimum particle size spinbox;
 * updates the minimum particle size slider value
 */
void Dialog_SettingsWindow::on_spinBox_MinParticleSize_editingFinished() {

    // ensures that the minimum particle size is no greater than the maximum particle size
    if (ui->spinBox_MinParticleSize->value() >= ui->spinBox_MaxParticleSize->value()) {
        ui->spinBox_MinParticleSize->setValue(ui->spinBox_MaxParticleSize->value()-1);
    }
    ui->horizontalSlider_MinParticleSize->setValue(ui->spinBox_MinParticleSize->value());
}

//------------------------------------------------------------------------

/* FUNCTION DESCRIPTION - triggered by value update of maximum particle size spinbox;
 * updates the maximum particle size slider value
 */
void Dialog_SettingsWindow::on_spinBox_MaxParticleSize_editingFinished() {

    // ensures that the maximum particle size is no lesser than the maximum particle size
    if (ui->spinBox_MaxParticleSize->value() <= ui->spinBox_MinParticleSize->value()) {
        ui->spinBox_MaxParticleSize->setValue(ui->spinBox_MinParticleSize->value()+1);
    }
    ui->horizontalSlider_MaxParticleSize->setValue(ui->spinBox_MaxParticleSize->value());
}

//------------------------------------------------------------------------
