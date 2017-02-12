/**
 * CS 106B/X Stanford 1-2-3
 * This file declares a view type that can be used for updating the GUI.
 *
 * Please do not modify this provided file. Your turned-in files should work
 * with an unmodified version of all provided code files.
 *
 * @author Marty Stepp, Julie Zelenski, Jerry Cain
 * @version 2016/11/29
 * - 16au 106x version (stepp)
 */

#ifndef _view_h
#define _view_h

#include <string>

/**
 * This pure virtual base class ("interface", in Java parlance) is used as a
 * parent type for classes that want to be notified when a spreadsheet updates.
 */
class View {
public:
    virtual void clearCells() = 0;
    virtual void displayCell(int row, int column, const std::string& text) = 0;
    virtual void displayCell(const std::string& cellname, const std::string& text) = 0;
};

#endif // _view_h
