/*
 *	Container for Roboclaw classes and particle manipulation.
 *  Handles all translation modeling and interfaces with hardware.
 *
 *	AUTHOR Bassam M.
 *
 */
//------------------------------------------------------------------------

#include <thread>
#include <chrono>
#include <QObject>
#include <QTime>
#include <QDebug>
#include <roboclaw.h>
#include <stdlib.h>
#include <Eigen>

#include "point.h"
#include "macros.h"

#define ROBO_SPEED 115200               // Roboclaw BAUDRATE
#define ROBO_X_ADDRESS 128              // Unique identifier for roboclaw 1
#define ROBO_Y_ADDRESS 129              // Unique identifier for roboclaw 2

//------------------------------------------------------------------------

// Eigen matrix manipulation initialization
using namespace Eigen;

// Shortcut MatrixXd to be a Matrix of type double, Dynamic, Dynamic
typedef Matrix<double, Dynamic, Dynamic> MatrixXd;

class Physics : public QObject
{
        Q_OBJECT
//------------------------------------------------------------------------

private:

    // Roboclaw class variable initialization
    Roboclaw *robo_x;
    Roboclaw *robo_y;

    // COM port names to be used in establishing connection to motor controllers
    // NOTE: different setups may include more port names (greater than COM14)
    std::vector<std::string> portNames = {"\\\\.\\COM0", "\\\\.\\COM1", "\\\\.\\COM2", "\\\\.\\COM3",
                                          "\\\\.\\COM4", "\\\\.\\COM5", "\\\\.\\COM6", "\\\\.\\COM7",
                                          "\\\\.\\COM8", "\\\\.\\COM9", "\\\\.\\COM10", "\\\\.\\COM11",
                                          "\\\\.\\COM12", "\\\\.\\COM13", "\\\\.\\COM14"};

    // Function interface to send command to the roboclaw controlling X-axis manipulation.
    // Parameters are included for the current scale and duration for each channel.
    void moveRoboX(uint8_t current_scale_plusx, int duration_plusx,
                   uint8_t current_scale_minusx, int duration_minusx, bool &isDone);


    // Function interface to send command to the roboclaw controlling X-axis manipulation.
    // Parameters are included for the current scale and duration for each channel.
    void moveRoboY(uint8_t current_scale_plusy, int duration_plusy,
                   uint8_t current_scale_minusy, int duration_minusy, bool &isDone);

    // Saved working COM ports for both roboclaws
    std::string comPortRoboX;
    std::string comPortRoboY;

    // Boolean parameter to check if hardware connection is properly initialized
    bool isInitializedProperly = false;

    // Previous particle location
    Point previousLoc = Point(-1,-1);

    // Boost parameters. Manipulated throughout the physics class.
    QString xDirectioninNeedOfBoost = "";
    QString yDirectioninNeedOfBoost = "";

    // Vector array of all coil locations
    std::vector<Point> coilLocs;

    // Command sequence to be sent to motor controllers.
    // Format is: Current Scale for Roboclaw X channel 1, Current duration for Roboclaw X Channel 1, ...
    // So index 4 would be current scale for Roboclaw Y channel 1 and index 5 is the current duration for roboclaw Y channel 1
    std::vector<int> commandArray = {0,0,0,0,0,0,0,0};

    // Used to normalize input parameters to neural network
    MatrixXd norm(double init_X, double init_Y, double final_X, double final_Y, double dist_to_coil, double mean, double std);

    // The amount of boost in each axis direction
    int boost_X = 0;
    int boost_Y = 0;

    // Functions which generate current scale (hardware command) using trained neural network weights given
    // the inputs needed. Each function has weights specific to a certain coil.
    // Parameters for each function are: Initial X and Y coordinates, Final X and Y coordinates, and euclidean particle
    // distance to each coil location.
    int movePlusX(double init_X, double init_Y, double final_X, double final_Y, double dist_to_PlusX);
    int moveMinusX(double init_X, double init_Y, double final_X, double final_Y, double dist_to_MinusX);
    int moveMinusY(double init_X, double init_Y, double final_X, double final_Y, double dist_to_MinusY);
    int movePlusY(double init_X, double init_Y, double final_X, double final_Y, double dist_to_PlusY);


    // Mean and standard deviation input parameters for each coil to be used for Z-score normalization.
    double mean_XPlus = 4.56106731428571;
    double std_XPlus = 14.6950071736071;

    double mean_XMinus = 9.47650655999994;
    double std_XMinus = 13.6815392879361;

    double mean_YMinus = 7.67926666666665;
    double std_YMinus = 12.6400463717554;

    double mean_YPlus = 4.36931255999999;
    double std_YPlus = 15.8091511002263;

    // Matrix initialization for each coil used to calculate neural network output.

    MatrixXd W_I_XPlus = MatrixXd(10,5);                    // Input weight matrix
    MatrixXd W_HO_XPlus = MatrixXd(1,10);                   // Output weight matrix
    MatrixXd B_IH_XPlus = MatrixXd(10,1);                   // Input bias
    MatrixXd B_HO_XPlus = MatrixXd(1,1);                    // Output bias

    MatrixXd W_I_XMinus = MatrixXd(10,5);
    MatrixXd W_HO_XMinus = MatrixXd(1,10);
    MatrixXd B_IH_XMinus = MatrixXd(10,1);
    MatrixXd B_HO_XMinus = MatrixXd(1,1);

    MatrixXd W_I_YPlus = MatrixXd(10,5);
    MatrixXd W_HO_YPlus = MatrixXd(1,10);
    MatrixXd B_IH_YPlus = MatrixXd(10,1);
    MatrixXd B_HO_YPlus = MatrixXd(1,1);

    MatrixXd W_I_YMinus = MatrixXd(10,5);
    MatrixXd W_HO_YMinus = MatrixXd(1,10);
    MatrixXd B_IH_YMinus = MatrixXd(10,1);
    MatrixXd B_HO_YMinus = MatrixXd(1,1);

//------------------------------------------------------------------------

public:

    // Physics constructor.
    Physics();

    // Constructor that takes in com port names
    Physics(std::string com_x,std::string com_y);

    // Destructor
    ~Physics() {
        qDebug() << "Physics is destructed";
        delete robo_x;
        delete robo_y;
    }
//------------------------------------------------------------------------

signals:

    // Function to send information to operation log
    void send_OperationLogMsg(QString msg);

//------------------------------------------------------------------------

public slots:

    // Checks if motor controllers are connected properly
    bool motorsAvailable();

    // Functions to compute next command given current and desired particle locations
    // using Neural Network (NN) or Surface Fitting (SF)
    void computeNextCommand_SF(Point currentLoc, Point nextLoc);
    void computeNextCommand_NN(Point currentLoc, Point nextLoc);

    // Send hardware commands to motor controllers from command array class variable
    bool translateParticle();

    // Send hardware commands to motor controllers given inputs
    bool translateParticle(uint8_t current_scale_plusx, int duration_plusx,
                           uint8_t current_scale_minusx, int duration_minusx,
                           uint8_t current_scale_plusy, int duration_plusy,
                           uint8_t current_scale_minusy, int duration_minusy);

    // Store variables to be used for data collections
    bool incrementDataCollection(double object_Px, double object_Py, double desiredX, double desiredY,
                                 double distFrom_PlusX, double distFrom_MinusX, double distFrom_PlusY, double distFrom_MinusY);

    // Set coil locations
    void setCoilLocs(std::vector<Point> inputCoilLocs) {
        this->coilLocs = inputCoilLocs;
    }

    // Set the boost given current and target particle location
    void set_Boost(Point currentLoc, Point targetLoc);

    // Return com port names for each roboclaw
    QString getComPortXName();
    QString getComPortYName();

    // Return command array
    std::vector<int> get_TranslationCommandInfo();

//------------------------------------------------------------------------

};
