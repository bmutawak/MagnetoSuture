/*
 *	Physics source file. Implements all prototypes defined in physics.h
 *
 *
 *	AUTHOR Bassam M.
 *
 */
//------------------------------------------------------------------------

#include "Physics.h"


//------------------------------------------------------------------------
/*
*	Activates X roboclaw depending on current scale and duration
*/
void Physics::moveRoboX(uint8_t current_scale_plusx, int duration_plusx,
                        uint8_t current_scale_minusx, int duration_minusx, bool &isDone) {

    // Checks to see if the plus x coil current scale is nonzero
    if (current_scale_plusx != 0)
    {

        // Sends command to motorcontroller to activate channel 1 with given current scale
        // The ForwardM1 command returns true if the command was successfully send to the motor controllers
        isDone |= robo_x->ForwardM1(ROBO_X_ADDRESS, current_scale_plusx);

        // Force this thread to sleep for the current duration amount
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_plusx));

        // Send another command with a current scale of zero to ensure roboclaw has stopped.
        // If both the initial command and the final stop command returned true, the isDone boolean
        // parameter will be true after this command and false otherwise.
        isDone &= robo_x->ForwardM1(ROBO_X_ADDRESS, 0);
    }
    else if (current_scale_minusx != 0)
    {

        // See above comments. This is the same procedure but if the minus X coil
        // current scale is nonzero.
        isDone |= robo_x->ForwardM2(ROBO_X_ADDRESS, current_scale_minusx);
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_minusx));
        isDone &= robo_x->ForwardM2(ROBO_X_ADDRESS, 0);

    }
    else if (current_scale_plusx == 0 || current_scale_minusx == 0){

        // This portion is executed if all else fails. This is used to initially connect
        // the motor controllers
        isDone |= robo_x->ForwardM2(ROBO_X_ADDRESS, current_scale_plusx);
    }
}

//------------------------------------------------------------------------
/*
*	Activates Y roboclaw depending on current scale and duration.
*   Please see commends above for moveRoboX. The logic is identical.
*/
void Physics::moveRoboY(uint8_t current_scale_plusy, int duration_plusy,
                        uint8_t current_scale_minusy, int duration_minusy, bool &isDone)
{
    if (current_scale_plusy != 0)
    {
        isDone |= robo_y->ForwardM1(ROBO_Y_ADDRESS, current_scale_plusy);
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_plusy));
        isDone &= robo_y->ForwardM1(ROBO_Y_ADDRESS, 0);
    }
    else if (current_scale_minusy != 0)
    {
        isDone |= robo_y->ForwardM2(ROBO_Y_ADDRESS, current_scale_minusy);
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_minusy));
        isDone &= robo_y->ForwardM2(ROBO_Y_ADDRESS, 0);
    }
    else if (current_scale_plusy == 0 || current_scale_minusy == 0){
        isDone |= robo_y->ForwardM2(ROBO_Y_ADDRESS, current_scale_plusy);
    }
}

//------------------------------------------------------------------------
/*
*	Checks to ensure proper communication
*/
bool Physics::motorsAvailable() {

    // Returns true if both motor controllers are successfully connected.
    return robo_x->isAvailable() && robo_x->isAvailable();

}

//------------------------------------------------------------------------


/*
 * Physics constructer given no COM port names. This loops through all
 * available ports and sends commands. The motor controllers are initialized
 * if a successful command was sent.
*/
Physics::Physics() {

    int namesVectorLength = static_cast<int>(this->portNames.size());
    std::string currentPort;

    // Boolean parameters to be used if eighter roboclaw is set.
    bool xIsSet = false;
    bool yIsSet = false;

    bool canMove = false;

    // Loops through each index of the COM port names vector.
    for (int i = 0; i < namesVectorLength; i++) {

        // Sets the current port to the be current inndex
        currentPort = portNames.at(static_cast<unsigned long long>(i));

        // If the X roboclaw is not yet set
        if (!xIsSet) {

            // Try catch for roboclaw setup. This throws an error if no device is connected
            // to the COM port it is trying to set up.
            try {

                // Initialize robo X with the current com port and Baud Rate
                this->robo_x = new Roboclaw(currentPort, ROBO_SPEED);

                // A test command. If canMove is true after this command, then the
                // Roboclaw X is connected to the current COM port and is good to go.
                this->moveRoboX(0,0,0,0, canMove);

                // If command was sent successfully
                if (canMove) {

                    // Set X boolean to be true
                    xIsSet = true;

                    // Reset the canMove boolean
                    canMove = false;

                    // Set the com port for robo X to be the current port
                    this->comPortRoboX = currentPort;

                    // Continue to the next iteration of the loop
                    continue;
                }
                // Catch an error resulting from incorrect COM port
            } catch (std::exception e) {}
        }

        // Same logic for Y.
        if (!yIsSet) {
            try {

                // Try to set the Y roboclaw
                this->robo_y = new Roboclaw(currentPort, ROBO_SPEED);

                // Send a test command
                this->moveRoboY(0,0,0,0,canMove);

                // If command was successful
                if (canMove) {

                    // Set the Y motor controller
                    yIsSet = true;
                    canMove = false;
                    this->comPortRoboY = currentPort;

                    // continue to next iteration
                    continue;
                }
            } catch (std::exception e) {}
        }

    }


    // Initialize neural network weights and bias for each coil

    //Weights for Plus X
    W_I_XPlus << -10.0072329209466, 6.45961981517493, 6.00950327985644, 5.99713897195693, -2.96668932147861,
        31.9783667308638, -20.5550488832418, -48.3028921016451, 20.3941040224823, -16.2661185707276,
        - 19.9268238559750,	0.213538971710742,	16.5277403449790, 0.381798806637307, -0.394249007691107,
        - 11.1886848742689, -17.6343967857119,	6.78246245563282, -7.89701857279568,	10.2804772878987,
        98.2077941548112,	111.665359478789, -34.5985782373730, -117.841853723082, -10.6408815189047,
        - 29.1035387607019, -6.97997327030342, -23.4500329495352, -9.63062781977193,	53.5774683633284,
        - 3.60393086248888,	10.5037734933914, -33.9959081704081, -11.8861542339878, -50.4471905960175,
        - 6.30037568443597, -2.63317467283980,	4.92120635281714,	2.78828487258481, -3.62265203361961,
        17.1205444120117, -2.92964240311554,	6.26270954710731,	3.82606753788165,	18.7434771678560,
        64.3540209647780, -38.2520434459882,	2.02601606618781,	10.5275626633884,	63.3194850577598;
    W_HO_XPlus << -4.47372110640185, -7.58799950280784, 13.3695849061267, -12.7340846842078, -8.66503516945277, 35.0358521114079, -7.25208250979585, -13.3644095443510, 10.9398381660345, 3.90039171763157;
    B_IH_XPlus<< -6.59047344633670,
            20.6911421987972,
            -2.18141122961065,
            -2.89217820536681,
            71.8020301076051,
            18.3152992947197,
            72.5709736960152,
            3.52656864572941,
            -25.6401970676584,
            -74.9152245144038;
    B_HO_XPlus << 8.18620914518664;


    //Weights for Minus X
    W_I_XMinus << -0.104567263820039, -1.67201600247932, 0.0956241175170325, -3.57233018138466, 5.06627697978426,
        -0.0252595442203610, 6.74613628979724, 3.66721357401703, 4.59133818253040, -3.87142913342108,
        -2.97750636266090, -0.109420236935314,	12.5532252666635,	2.03577742721907, -12.5817006060229,
        -14.4882140102667, -9.53251512163983,	25.9808609122040,	9.54459492590719, -10.4143380092625,
        -64.5192719930261, -56.8102828122074,	40.2208891085252,	39.5650386681443, -79.6435275797638,
        -2.22815068593308, -0.170131439525163,	12.5748342392406, -5.63645990353431, -7.78528861267621,
        -13.8118932471227,	6.17019958695816,	6.75254229628310, -5.99235013219578,	4.35311110534989,
        -0.445432824636306, 6.01791064835767, -3.33789090212738,	7.72218667948090, -17.2397591318276,
        0.941621800688702,	9.46241987071681,	0.0304155147431984,	9.96042122308498, -32.6895279544472,
        -12.8375798294969,	8.48517582365008,	7.89518945680643, -8.57036826659216,	2.79593857352266;
    W_HO_XMinus << 7.68535050255491, -3.30575998668349, -12.8943719848503, -16.5287397317440, -0.408825695631219, 16.1392526433930, -57.6457303364764, -15.4232937884118, 17.7753760724261, 42.8408314019056;
    B_IH_XMinus << 9.65811724965417,
        -0.447630813611959,
        -2.04161967949849,
        21.6332667291042,
        86.7116392780810,
        -5.74944021762798,
        -8.44980945732208,
        -5.45937913597017,
        -15.4112605734887,
        -4.36927500446279;
    B_HO_XMinus << 9.10689246604076;

    // Weights for Minus Y
    W_I_YMinus << -1.53671532511793, 2.84563096966466, -0.164708133759789, 1.21939103714500, 9.15439718004281,
        4.70099857643987,	6.20633650702055, -5.14094512881679, - 5.28228627547089,	0.584182836423060,
        0.538618037915664, -1.61925515000016,	4.56166942871436,	8.58506414848194, -7.38067282423193,
        - 3.06224055114101, -21.3330451344241,	1.81681089191324,	3.24868076343525,	13.0664440996960,
        13.1160182330450,	1.05502203505818, -14.2668453471173, -3.09320644748330, -1.90173337945477,
        - 5.60962330311280, -10.4449702975363,	8.14530194565485,	3.23343812014166,	4.50932731533645,
        0.511222826941135,	1.49163380368437, -0.686828115828473, -3.42157319789922,	4.52406221020561,
        0.710638858586224, -23.5914927967346, -0.516576730482275,	20.9586700545040,	3.60905034366893,
        2.37890513735993, -10.6040429182623, 4.84021042935092, -10.3146900307570, 10.1407117355226,
        1.00910626906626,	0.186356950322858,	0.809415913652285, -0.755919698727421,	0.438530516329789;
    W_HO_YMinus << 8.32302537743211, 17.5182082657585, 2.13698527045327, -18.9266387696311, -6.07400567020675, 0.190083951986220, 4.38779360011961, -12.1647798225087, -0.412154283058505, 9.81280515360629;
    B_IH_YMinus << 1.03007161719977,
        -1.79143532756681,
        -6.54189208917923,
        -27.5774189266603,
        -0.753252934184779,
        -8.61701101049892,
        8.92951189091131,
        -6.37257244879880,
        7.86398489897500,
        9.10313931404435;
    B_HO_YMinus << 14.7466728096038;

    //Weights for Plus Y
    W_I_YPlus << -3.69815671518051, 2.96616622177505, -1.10070059273999, 3.29336179619065, 13.8847070800275,
        - 1.85589936495146,	61.9631256728095,	2.45308488244810, -60.5523990148704, -1.05648954500539,
        2.90665419432043, -10.1958489577114, -0.540045180378358, -2.18765646157206,	16.7537805056013,
        - 16.0844479413510,	0.313760966556854, -12.3834370495581, -14.0779358536358,	11.0900237310057,
        - 4.11789729188376, -0.173525104391563, -7.61757607230936, -5.56299450689437,	4.85956335400442,
        1.22561531902346, -7.40233379523651, -0.569849468015292,	4.30182581233177, -5.31797672874703,
        9.15102176799354, -26.2582650202954, -9.34883124331713,	18.2953764726282, -0.291835426913627,
        1.34320756848034, -7.97201444829793, -4.44006637114823,	7.20380837228370,	23.0513083494877,
        0.731824281825986, -0.399190952014102,	2.04458001301020, - 1.71747484883555,	2.30029336068137,
        - 13.9689101681997,	23.6059727142377, -11.5786738976342,	34.3349320468021,	28.8593355800426;
    W_HO_YPlus << 18.9018052016238, -14.4606734119977,	1.42221914167345, -2.71999341980663,	3.30157387099858, -19.0380944996092,	18.0705074972269, -1.33867918301334, -0.517245961794046, -2.89443600728884;
    B_IH_YPlus << 22.0072085991892,
        5.17244915738741,
        10.9808556287992,
        - 13.3584119697046,
        2.43063379042153,
        5.35983858509506,
        - 5.92252816034240,
        20.8729398264778,
        7.92334600987591,
        - 29.2202354004244;
    B_HO_YPlus << 4.09488038895835;

}

//------------------------------------------------------------------------
/*
*	Physics onstructor. Takes in the COM_PORT names as Strings.
*/
Physics::Physics(std::string com_x, std::string com_y)
{

    // Sets the COM port names to be in correct format
    com_x = "\\\\.\\" + com_x;
    com_y = "\\\\.\\" + com_y;


    // Try to initialize motor controllers based on given com ports
    try {
        this->robo_x = new Roboclaw(com_x, ROBO_SPEED);
        this->robo_y = new Roboclaw(com_y, ROBO_SPEED);
        isInitializedProperly = true;
    } catch (std::exception e) {
        emit send_OperationLogMsg(ERROR_FORMAT("Cannot open motor controllers"));
    }

    // Initialize neural network weights and bias for each coil

    //Weights for Plus X
    W_I_XPlus << -10.0072329209466, 6.45961981517493, 6.00950327985644, 5.99713897195693, -2.96668932147861,
        31.9783667308638, -20.5550488832418, -48.3028921016451, 20.3941040224823, -16.2661185707276,
        - 19.9268238559750,	0.213538971710742,	16.5277403449790, 0.381798806637307, -0.394249007691107,
        - 11.1886848742689, -17.6343967857119,	6.78246245563282, -7.89701857279568,	10.2804772878987,
        98.2077941548112,	111.665359478789, -34.5985782373730, -117.841853723082, -10.6408815189047,
        - 29.1035387607019, -6.97997327030342, -23.4500329495352, -9.63062781977193,	53.5774683633284,
        - 3.60393086248888,	10.5037734933914, -33.9959081704081, -11.8861542339878, -50.4471905960175,
        - 6.30037568443597, -2.63317467283980,	4.92120635281714,	2.78828487258481, -3.62265203361961,
        17.1205444120117, -2.92964240311554,	6.26270954710731,	3.82606753788165,	18.7434771678560,
        64.3540209647780, -38.2520434459882,	2.02601606618781,	10.5275626633884,	63.3194850577598;
    W_HO_XPlus << -4.47372110640185, -7.58799950280784, 13.3695849061267, -12.7340846842078, -8.66503516945277, 35.0358521114079, -7.25208250979585, -13.3644095443510, 10.9398381660345, 3.90039171763157;
    B_IH_XPlus<< -6.59047344633670,
            20.6911421987972,
            -2.18141122961065,
            -2.89217820536681,
            71.8020301076051,
            18.3152992947197,
            72.5709736960152,
            3.52656864572941,
            -25.6401970676584,
            -74.9152245144038;
    B_HO_XPlus << 8.18620914518664;


    //Weights for Minus X
    W_I_XMinus << -0.104567263820039, -1.67201600247932, 0.0956241175170325, -3.57233018138466, 5.06627697978426,
        -0.0252595442203610, 6.74613628979724, 3.66721357401703, 4.59133818253040, -3.87142913342108,
        -2.97750636266090, -0.109420236935314,	12.5532252666635,	2.03577742721907, -12.5817006060229,
        -14.4882140102667, -9.53251512163983,	25.9808609122040,	9.54459492590719, -10.4143380092625,
        -64.5192719930261, -56.8102828122074,	40.2208891085252,	39.5650386681443, -79.6435275797638,
        -2.22815068593308, -0.170131439525163,	12.5748342392406, -5.63645990353431, -7.78528861267621,
        -13.8118932471227,	6.17019958695816,	6.75254229628310, -5.99235013219578,	4.35311110534989,
        -0.445432824636306, 6.01791064835767, -3.33789090212738,	7.72218667948090, -17.2397591318276,
        0.941621800688702,	9.46241987071681,	0.0304155147431984,	9.96042122308498, -32.6895279544472,
        -12.8375798294969,	8.48517582365008,	7.89518945680643, -8.57036826659216,	2.79593857352266;
    W_HO_XMinus << 7.68535050255491, -3.30575998668349, -12.8943719848503, -16.5287397317440, -0.408825695631219, 16.1392526433930, -57.6457303364764, -15.4232937884118, 17.7753760724261, 42.8408314019056;
    B_IH_XMinus << 9.65811724965417,
        -0.447630813611959,
        -2.04161967949849,
        21.6332667291042,
        86.7116392780810,
        -5.74944021762798,
        -8.44980945732208,
        -5.45937913597017,
        -15.4112605734887,
        -4.36927500446279;
    B_HO_XMinus << 9.10689246604076;

    // Weights for Minus Y
    W_I_YMinus << -1.53671532511793, 2.84563096966466, -0.164708133759789, 1.21939103714500, 9.15439718004281,
        4.70099857643987,	6.20633650702055, -5.14094512881679, - 5.28228627547089,	0.584182836423060,
        0.538618037915664, -1.61925515000016,	4.56166942871436,	8.58506414848194, -7.38067282423193,
        - 3.06224055114101, -21.3330451344241,	1.81681089191324,	3.24868076343525,	13.0664440996960,
        13.1160182330450,	1.05502203505818, -14.2668453471173, -3.09320644748330, -1.90173337945477,
        - 5.60962330311280, -10.4449702975363,	8.14530194565485,	3.23343812014166,	4.50932731533645,
        0.511222826941135,	1.49163380368437, -0.686828115828473, -3.42157319789922,	4.52406221020561,
        0.710638858586224, -23.5914927967346, -0.516576730482275,	20.9586700545040,	3.60905034366893,
        2.37890513735993, -10.6040429182623, 4.84021042935092, -10.3146900307570, 10.1407117355226,
        1.00910626906626,	0.186356950322858,	0.809415913652285, -0.755919698727421,	0.438530516329789;
    W_HO_YMinus << 8.32302537743211, 17.5182082657585, 2.13698527045327, -18.9266387696311, -6.07400567020675, 0.190083951986220, 4.38779360011961, -12.1647798225087, -0.412154283058505, 9.81280515360629;
    B_IH_YMinus << 1.03007161719977,
        -1.79143532756681,
        -6.54189208917923,
        -27.5774189266603,
        -0.753252934184779,
        -8.61701101049892,
        8.92951189091131,
        -6.37257244879880,
        7.86398489897500,
        9.10313931404435;
    B_HO_YMinus << 14.7466728096038;

    //Weights for Plus Y
    W_I_YPlus << -3.69815671518051, 2.96616622177505, -1.10070059273999, 3.29336179619065, 13.8847070800275,
        - 1.85589936495146,	61.9631256728095,	2.45308488244810, -60.5523990148704, -1.05648954500539,
        2.90665419432043, -10.1958489577114, -0.540045180378358, -2.18765646157206,	16.7537805056013,
        - 16.0844479413510,	0.313760966556854, -12.3834370495581, -14.0779358536358,	11.0900237310057,
        - 4.11789729188376, -0.173525104391563, -7.61757607230936, -5.56299450689437,	4.85956335400442,
        1.22561531902346, -7.40233379523651, -0.569849468015292,	4.30182581233177, -5.31797672874703,
        9.15102176799354, -26.2582650202954, -9.34883124331713,	18.2953764726282, -0.291835426913627,
        1.34320756848034, -7.97201444829793, -4.44006637114823,	7.20380837228370,	23.0513083494877,
        0.731824281825986, -0.399190952014102,	2.04458001301020, - 1.71747484883555,	2.30029336068137,
        - 13.9689101681997,	23.6059727142377, -11.5786738976342,	34.3349320468021,	28.8593355800426;
    W_HO_YPlus << 18.9018052016238, -14.4606734119977,	1.42221914167345, -2.71999341980663,	3.30157387099858, -19.0380944996092,	18.0705074972269, -1.33867918301334, -0.517245961794046, -2.89443600728884;
    B_IH_YPlus << 22.0072085991892,
        5.17244915738741,
        10.9808556287992,
        - 13.3584119697046,
        2.43063379042153,
        5.35983858509506,
        - 5.92252816034240,
        20.8729398264778,
        7.92334600987591,
        - 29.2202354004244;
    B_HO_YPlus << 4.09488038895835;

}

//------------------------------------------------------------------------

// Compute hardware commands given current and desired particle locations
// based on a Surface Fitting approach
void Physics::computeNextCommand_SF(Point currentLoc, Point nextLoc) {

    // Calculates the distance to travel in each axis
    double xDistanceToTravel = nextLoc.x() - currentLoc.x();
    double yDistanceToTravel = nextLoc.y() - currentLoc.y();
    double distToCoil;

    // Initializes the current scales and durations to all be Zero
    double currentScaleXPlus = 0;
    double currentScaleXMinus = 0;
    double currentScaleYPlus = 0;
    double currentScaleYMinus = 0;

    double currentDurationXPlus = 0;
    double currentDurationXMinus = 0;
    double currentDurationYPlus = 0;
    double currentDurationYMinus = 0;

    // Set the minimum current scale value. This is required as the surface fitting model may
    // return negative current scales, which are not acceptable.
    double minCurrentScale = 7.0;

    // If the X-axis distance to travel is greater than zero (so we need to activate the Plux X coil)
    if (xDistanceToTravel > 0) {

        // Compute the particle's distance to the plus X coil
        distToCoil = Point::computeEuclideanDist(currentLoc, coilLocs[0]);

        // Use the equation returned from 3D surface fitting to calculate current scale
        currentScaleXPlus = 816.3325 - 105.8183*xDistanceToTravel - 82.3793*distToCoil + 4.8661*std::pow(xDistanceToTravel,2) +
                7.33645*xDistanceToTravel*distToCoil + 2.6562*std::pow(distToCoil, 2) - 0.2251*std::pow(xDistanceToTravel,2)*distToCoil -
                0.1055*xDistanceToTravel*std::pow(distToCoil, 2) - 0.0268*std::pow(distToCoil, 3);

        // Make sure it is greater than the minimum current scale
        currentScaleXPlus = std::max(currentScaleXPlus, minCurrentScale);

        // Set current duration to be 100 ms
        currentDurationXPlus = 100;

        // If the X distance to travel is less than zero (so we need to activate the Minus X coil)
    } else if (xDistanceToTravel < 0) {

        // Make the distance to travel a positive value (required for equation)
        xDistanceToTravel = std::abs(xDistanceToTravel);

        // Compute distance to minus X coil
        distToCoil = Point::computeEuclideanDist(currentLoc, coilLocs[1]);

        // Use surface fitting equation for minus X coil to calculate current scale
        currentScaleXMinus = 1630.9 - 288.0743*xDistanceToTravel - 157.4598*distToCoil + 14.2526*std::pow(xDistanceToTravel,2) +
                18.9574*xDistanceToTravel*distToCoil + 4.8963*std::pow(distToCoil, 2) - 0.6053*std::pow(xDistanceToTravel,2)*distToCoil -
                0.2767*xDistanceToTravel*std::pow(distToCoil, 2) - 0.0486*std::pow(distToCoil, 3);

        // Ensure that this is greater than the minimum
        currentScaleXMinus = std::max(currentScaleXMinus, minCurrentScale);

        // Set current duration to be 100 ms
        currentDurationXMinus = 100;
    }

    // If Y-axis distance to travel is greater than zero (we need to activate the Plus Y coil)
    if (yDistanceToTravel > 0) {

        // Compute distance to coil
        distToCoil = Point::computeEuclideanDist(currentLoc, coilLocs[2]);

        // Compute current scale using surface fitting equation
        currentScaleYPlus = 1930.5 - 296.6075*yDistanceToTravel - 190.1963*distToCoil + 13.8199*std::pow(yDistanceToTravel,2) +
                19.5185*yDistanceToTravel*distToCoil + 6.0726*std::pow(distToCoil, 2) - 0.585*std::pow(yDistanceToTravel,2)*distToCoil -
                0.2850*yDistanceToTravel*std::pow(distToCoil, 2) - 0.0624*std::pow(distToCoil, 3);

        // Ensure this is greater than minimum
        currentScaleYPlus = std::max(currentScaleYPlus, minCurrentScale);

        // Set current duration to be 100 ms
        currentDurationYPlus = 100;

        // If Y-axis distance to travel is less than zero (so we need to activate the minus Y coil)
    } else if (yDistanceToTravel < 0) {

        // Make distance to travel positive
        yDistanceToTravel = std::abs(yDistanceToTravel);

        // Compute distance to coil
        distToCoil = Point::computeEuclideanDist(currentLoc, coilLocs[3]);

        // Calculate current scale using surface fitting equation
        currentScaleYMinus = 720.8288 - 70.6802*yDistanceToTravel - 73.2834*distToCoil + 4.3349*std::pow(yDistanceToTravel,2) +
                4.7419*yDistanceToTravel*distToCoil + 2.3785*std::pow(distToCoil, 2) - 0.2030*std::pow(yDistanceToTravel,2)*distToCoil -
                0.0578*yDistanceToTravel*std::pow(distToCoil, 2) - 0.0241*std::pow(distToCoil, 3);

        // Set to be atleast the minimum
        currentScaleYMinus = std::max(currentScaleYMinus, minCurrentScale);

        // Set current duration
        currentDurationYMinus = 100;
    }

    // Set the hardware command array based on all the above logic.
    // The format of the array is:
    //                    Current Scale Plus X Coil, Current Duration Plus X Coil
    //                    Current Scale Minus X Coil, Current Duration Minus X Coil
    //                    Current Scale Plus Y Coil, Current Duration Plus Y Coil
    //                    Current Scale Minus Y Coil, Current Duration Minus Y Coil
    this->commandArray = {static_cast<uint8_t>(currentScaleXPlus), static_cast<int>(currentDurationXPlus),
                          static_cast<uint8_t>(currentScaleXMinus), static_cast<int>(currentDurationXMinus),
                          static_cast<uint8_t>(currentScaleYPlus), static_cast<int>(currentDurationYPlus),
                          static_cast<uint8_t>(currentScaleYMinus), static_cast<int>(currentDurationYMinus)};
}


//------------------------------------------------------------------------

/*
 * Function to send hardware commands based on stored command array.
 * Returns true if command was successfully sent
 *
*/
bool Physics::translateParticle() {
    return this->translateParticle(static_cast<unsigned char>(commandArray[0]),
                            commandArray[1],
                            static_cast<unsigned char>(commandArray[2]),
                            commandArray[3],
                            static_cast<unsigned char>(commandArray[4]),
                            commandArray[5],
                            static_cast<unsigned char>(commandArray[6]),
                            commandArray[7]);
}

//------------------------------------------------------------------------
/*
*	Moves in both X and Y directions given current scale and duration for each. Returns true for complete operation.
*/
bool Physics::translateParticle(uint8_t current_scale_plusx, int duration_plusx,
                                uint8_t current_scale_minusx, int duration_minusx,
                                uint8_t current_scale_plusy, int duration_plusy,
                                uint8_t current_scale_minusy, int duration_minusy)
{
    // Check for proper communication
    if (!motorsAvailable()) { return false; }

    bool movedX = false;
    bool movedY = false;

    // Creates seperate CPU threads for each execution. This is important as there are busy waits in each execution,
    // and for simulataneous execution this is isolated to seperate threads
    std::thread moveinX(&Physics::moveRoboX, this, current_scale_plusx, duration_plusx, current_scale_minusx, duration_minusx,  std::ref(movedX));
    std::thread moveinY(&Physics::moveRoboY, this, current_scale_plusy, duration_plusy, current_scale_minusy, duration_minusy, std::ref(movedY));


    // Join the threads with the main thread. This is needed to return the movedX and movedY booleans passed to each thread
    moveinX.join();
    moveinY.join();

    // Delete each created thread
    moveinX.~thread();
    moveinY.~thread();

    // True if both were successful, false otherwise
    return movedX && movedY;
}

//------------------------------------------------------------------------
/*
*	Automatic data collection function. Translates particle based on data collection procedure.
*	Used only for data collection
*/
bool Physics::incrementDataCollection(double object_Px, double object_Py, double desiredX, double desiredY,
                                      double distFrom_PlusX, double distFrom_MinusX, double distFrom_PlusY, double distFrom_MinusY)
{
    return false;
}

//------------------------------------------------------------------------

/*
 * Compute requred hardware commands given current and next particle locations
 * based on a Neural Network modeling approach
 *
*/
void Physics::computeNextCommand_NN(Point currentLoc, Point nextLoc) {


    // Compute the distance to travel in each axis
    double xDistanceToTravel = nextLoc.x() - currentLoc.x();
    double yDistanceToTravel = nextLoc.y()  - currentLoc.y();


    // set the initial current scales and durations to be zero
    int currentScaleXPlus = 0;
    int currentScaleXMinus = 0;
    int currentScaleYPlus = 0;
    int currentScaleYMinus = 0;

    int currentDurationXPlus = 0;
    int currentDurationXMinus = 0;
    int currentDurationYPlus = 0;
    int currentDurationYMinus = 0;

    // Initial distance to coil to be zero, and the minimum current scale.
    double distToCoil = 0;
    int minCurrentScale = 7;

    // If we need to travel in the plus X direction...
    if (xDistanceToTravel > 0) {

        // Compute distance to coil
        distToCoil = Point::computeEuclideanDist(currentLoc, coilLocs[0]);

        // Call the NN function to move in plus X
        currentScaleXPlus = std::max(this->movePlusX(currentLoc.x(), currentLoc.y(), nextLoc.x(), nextLoc.y(), distToCoil), minCurrentScale);

        // If there is a need for a boost, then add boost
        if (xDirectioninNeedOfBoost == "+X") {
            currentScaleXPlus += boost_X;
        }

        // Only set current duration to be nonzero if we need to activate that coil
        currentDurationXPlus = 100;
    }
    // Repeat for Minus X Coil if we need to travel in that direction instead
    else if (xDistanceToTravel < 0) {

        // Make sure distance to travel is positive (NN only takes positive inputs)
        xDistanceToTravel = std::abs(xDistanceToTravel);
        distToCoil = Point::computeEuclideanDist(currentLoc, coilLocs[1]);

        currentScaleXMinus = std::max(this->moveMinusX(currentLoc.x(), currentLoc.y(), nextLoc.x(), nextLoc.y(), distToCoil), minCurrentScale);



        if (xDirectioninNeedOfBoost == "-X") {
            currentScaleXMinus += boost_X;
        }

        currentDurationXMinus = 100;
    }

    // Repeat for Plus Y coil
    if (yDistanceToTravel > 0) {

        // Calculate distance to plus Y coil
        distToCoil = Point::computeEuclideanDist(currentLoc, coilLocs[2]);

        // Use NN function
        currentScaleYPlus = std::max(this->movePlusY(currentLoc.x(), currentLoc.y(), nextLoc.x(), nextLoc.y(), distToCoil), minCurrentScale);

        // Add boost
        if (yDirectioninNeedOfBoost == "+Y") {
            currentScaleYPlus += boost_Y;
        }

        currentDurationYPlus = 100;

    }
    // Repeat for Minus Y coil if we need to travel in that direction
    else if (yDistanceToTravel < 0) {

        // Make sure Distance to travel is positive
        yDistanceToTravel = std::abs(yDistanceToTravel);

        // Compute distance to coil
        distToCoil = Point::computeEuclideanDist(currentLoc, coilLocs[3]);

        // Use NN Function
        currentScaleYMinus = std::max(this->moveMinusY(currentLoc.x(), currentLoc.y(), nextLoc.x(), nextLoc.y(), distToCoil), minCurrentScale);


        // Add boost if needed
        if (yDirectioninNeedOfBoost == "-Y") {
            currentScaleYMinus += boost_Y;
        }

        currentDurationYMinus = 100;
    }

    // Configure command array
    this->commandArray = {currentScaleXPlus, currentDurationXPlus, currentScaleXMinus, currentDurationXMinus, currentScaleYPlus, currentDurationYPlus, currentScaleYMinus, currentDurationYMinus};


}

//------------------------------------------------------------------------

// Return a 5x1 normalized input matrix based on standard deviation and mean of collected data
MatrixXd Physics::norm(double init_X, double init_Y, double final_X, double final_Y, double dist_to_coil, double mean, double std) {

    MatrixXd norm(5, 1);
    norm << (init_X - mean) / std,
            (init_Y - mean) / std,
            (final_X - mean) / std,
            (final_Y - mean) / std,
            (dist_to_coil - mean) / std;

    return norm;
}

//------------------------------------------------------------------------

// Given initial inputs, this function normalizes them and computes the
// required current scale for the plus X coil using neural network weights
int Physics::movePlusX(double init_X, double init_Y, double final_X, double final_Y, double dist_to_PlusX) {

    // Normalize
    MatrixXd input(5, 1);
    input = this->norm(init_X, init_Y, final_X, final_Y, dist_to_PlusX, mean_XPlus, std_XPlus);

    // Use neural network weights to compute output.
    // in our case it's output = B_HO + W_HO * tansig(B_IH + W_I * input)
    MatrixXd O_H = (W_I_XPlus * input) + B_IH_XPlus;
    O_H = O_H.array().tanh();

    MatrixXd output = (W_HO_XPlus * O_H) + B_HO_XPlus;
    return static_cast<int>(output(0, 0));
}

//------------------------------------------------------------------------

// Given initial inputs, this function normalizes them and computes the
// required current scale for the minus X coiul using neural network weights
int Physics::moveMinusX(double init_X, double init_Y, double final_X, double final_Y, double dist_to_PlusX) {

    // Normalize
    MatrixXd input(5, 1);
    input = this->norm(init_X, init_Y, final_X, final_Y, dist_to_PlusX, mean_XMinus, std_XMinus);

    // Output = B_HO + W_HO * tansig(B_IH + W_I * input)
    MatrixXd O_H = (W_I_XMinus * input) + B_IH_XMinus;
    O_H = O_H.array().tanh();

    MatrixXd output = (W_HO_XMinus * O_H) + B_HO_XMinus;
    return static_cast<int>(output(0, 0));
}

//------------------------------------------------------------------------

// Given initial inputs, this function normalizes them and computes the
// required current scale for the minus Y Coil using neural network weights
int Physics::moveMinusY(double init_X, double init_Y, double final_X, double final_Y, double dist_to_PlusX) {

    // Normalize
    MatrixXd input(5, 1);
    input = this->norm(init_X, init_Y, final_X, final_Y, dist_to_PlusX, mean_YMinus, std_YMinus);

    // Output = B_HO + W_HO * tansig(B_IH + W_I * input)
    MatrixXd O_H = (W_I_YMinus * input) + B_IH_YMinus;
    O_H = O_H.array().tanh();

    MatrixXd output = (W_HO_YMinus * O_H) + B_HO_YMinus;
    return static_cast<int>(output(0, 0));
}

//------------------------------------------------------------------------


// Given initial inputs, this function normalizes them and computes the
// required current scale for the Plus X Coil using neural network weights
int Physics::movePlusY(double init_X, double init_Y, double final_X, double final_Y, double dist_to_PlusX) {

    // Normalize
    MatrixXd input(5, 1);
    input = this->norm(init_X, init_Y, final_X, final_Y, dist_to_PlusX, mean_YPlus, std_YPlus);


    // Output = B_HO + W_HO * tansig(B_IH + W_I * input)
    MatrixXd O_H = (W_I_YPlus * input) + B_IH_YPlus;
    O_H = O_H.array().tanh();

    MatrixXd output = (W_HO_YPlus * O_H) + B_HO_YPlus;
    return static_cast<int>(output(0, 0));
}

//------------------------------------------------------------------------

// Sets boost in both axis directions based on how much object
// has moved (or not moved)
void Physics::set_Boost(Point currentLoc, Point targetLoc) {

    // initial case - start of operation
    if (previousLoc == Point(-1,-1)) {
        previousLoc = currentLoc;
        boost_X = 0;
        boost_Y = 0;
        return;
    }

    // Compute distance travelled in both axis
    double distanceTraveled_inX = abs(previousLoc.y() - currentLoc.y());
    double distanceTraveled_inY = abs(previousLoc.x() - currentLoc.x());

    // Compute required distance to travel in both axis
    double distanceToTravel_inX = targetLoc.x() - currentLoc.x();
    double distanceToTravel_inY = targetLoc.y() - currentLoc.y();

    // Choosing which directions to boost
    if (distanceToTravel_inX > 0) {xDirectioninNeedOfBoost = "+X";}
    else if (distanceToTravel_inX < 0) {xDirectioninNeedOfBoost = "-X";}

    if (distanceToTravel_inY > 0) {yDirectioninNeedOfBoost = "+Y";}
    else if (distanceToTravel_inY < 0) {yDirectioninNeedOfBoost = "-Y";}


    // Set boost in either axis if we need to travel more than 1.5 mm
    // in that direction and we have only travelled 0.2 mm previously
    if (abs(distanceToTravel_inX) >= 1.5 && distanceTraveled_inX <= 0.2) {

        // Add 3 to next command's current scale
        boost_X += 3;
    }
    else {
        boost_X = 0;
        xDirectioninNeedOfBoost = "";
    }

    // Repeat for the Y direction boost
    if (abs(distanceToTravel_inY) >= 1.5 && distanceTraveled_inY <= 0.2) {
        boost_Y += 3;
    }
    else {
        boost_Y = 0;
        yDirectioninNeedOfBoost = "";
    }

    // if the particle is more or less completely still, add boost to
    // both axis directions
    if (distanceTraveled_inX <= 0.1 && distanceToTravel_inX >= 0.5 &&
            distanceTraveled_inY <= 0.1 && distanceToTravel_inY >= 0.5) {
        boost_X = 3;
        boost_Y = 3;
    }

//    qDebug() << "Moved in X: " << distanceTraveled_inX;
//    qDebug() << "Moved in Y: " << distanceTraveled_inY;
//    qDebug() << "Boost X = " << boost_X;
//    qDebug() << "Boost Y = " << boost_Y;

    previousLoc = currentLoc;
}

//------------------------------------------------------------------------

// Return the comport name for X-axis motor controller
QString Physics::getComPortXName() {
    if (motorsAvailable()) {
        return QString::fromStdString(this->comPortRoboX).remove("\\\\.\\");
    }
    else {
        return "Err N/A";
    }
}

//------------------------------------------------------------------------

// Return the comport name for Y-axis motor controller
QString Physics::getComPortYName() {
    if (motorsAvailable()) {
        return QString::fromStdString(this->comPortRoboY).remove("\\\\.\\");
    }
    else {
        return "Err N/A";
    }
}

//------------------------------------------------------------------------

// Return the current command Array
std::vector<int> Physics::get_TranslationCommandInfo() {
    return this->commandArray;
}
