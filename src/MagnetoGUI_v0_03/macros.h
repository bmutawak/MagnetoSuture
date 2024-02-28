/*
 *	Macros header file.
 *
 *
 *	AUTHOR: Victor Huynh
 *
 */
//------------------------------------------------------------------------

#ifndef MACROS_H
#define MACROS_H

#include <QObject>

// macro to produce a message in an error format (such as for debugging purposes)
#define ERROR_FORMAT(msg) \
    Macro::format_ErrorMessage(this, __LINE__, msg)

//------------------------------------------------------------------------

/*
 * Class Description:
 * Macros is used to store convenience macros and functions for use throughout the code.
 * While only one macro is employed in this iteration of the project, this class can be
 * expanded on for future iterations.
 */

//------------------------------------------------------------------------
class Macro {
//------------------------------------------------------------------------

    // Class Public Functions (non-slots)/Members
public:

    // used to produce a string message in a clear error format
    static QString format_ErrorMessage(QObject *obj, int line, QString msg) {
        return "[Error @ " + QString(obj->metaObject()->className()) + " L." + QString::number(line) + "]: " + msg;
    }

//------------------------------------------------------------------------

};

#endif // MACROS_H
