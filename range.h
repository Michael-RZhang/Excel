/**
 * CS 106B/X Stanford 1-2-3
 * This file declares the Range type that can be used by your spreadsheet.
 *
 * Please do not modify this provided file. Your turned-in files should work
 * with an unmodified version of all provided code files.
 *
 * @author Marty Stepp, Julie Zelenski, Jerry Cain
 * @version 2016/11/29
 * - 16au 106x version (stepp)
 */

#ifndef _range_h
#define _range_h

#include <iostream>
#include "set.h"
#include "vector.h"

/**
 * This struct can be used to identify a range. It represents 0-based
 * start/stop row and column values as locations.
 */
class Range {
public:
    /**
     * Constructs a range enclosing the given start and end cells and all
     * cells between them.  The cells are passed by 0-based row and column
     * indexes.
     */
    Range(int startRow = 0, int startCol = 0, int endRow = 0, int endCol = 0);

    /**
     * Constructs a range enclosing the given start and end cells and all
     * cells between them.  The cells are passed by Excel-style cell names
     * such as "A4" or "B17".
     */
    Range(const std::string& startCellName, const std::string& endCellName);

    /**
     * Returns a set containing the names of all cells in this range.
     * For example, if the range is B3:C5, returns the set containing
     * {"B3", "B4", "B5", "C3", "C4", "C5"}.
     */
    Set<std::string> getAllCellNames() const;

    /**
     * Returns the ending cell in the range as an Excel-style cell name
     * such as "A7". Note that you can just access the end row/column
     * directly using the member variables, but this is provided for
     * convenience in case you want the cell's name rather than the 0-based
     * indexes.
     */
    std::string getEndCellName() const;

    /**
     * Returns the 0-based column of the end of this range.
     * For example, if the range is C5:F7, returns 5
     * (which is the 0-based number for column F from "F7", the sixth column).
     */
    int getEndColumn() const;

    /**
     * Returns the 0-based row of the end of this range.
     * For example, if the range is C5:F7, returns 6
     * (which is the 1-based row 7 from "F7" minus 1 to make it 0-based).
     */
    int getEndRow() const;

    /**
     * Returns the starting cell in the range as an Excel-style cell name
     * such as "A4". Note that you can just access the start row/column
     * directly using the member variables, but this is provided for
     * convenience in case you want the cell's name rather than the 0-based
     * indexes.
     */
    std::string getStartCellName() const;

    /**
     * Returns the 0-based column of the start of this range.
     * For example, if the range is C5:F7, returns 2
     * (which is the 0-based number for column C from "C5", the third column).
     */
    int getStartColumn() const;

    /**
     * Returns the 0-based row of the end of this range.
     * For example, if the range is C5:F7, returns 4
     * (which is the 1-based row 5 from "C5" minus 1 to make it 0-based).
     */
    int getStartRow() const;

    /**
     * Returns true if the given function name is one of the known names in
     * the FUNCTION_NAMES set declared in this class.
     */
    static bool isKnownFunctionName(const std::string& function);

    /**
     * Returns true if the given name is a valid Excel-style name for a cell.
     * For example, "A17" or "BZF45" are valid cell names.
     */
    static bool isValidName(const std::string& cellname);

    /**
     * Converts the given 0-based row and columns into an Excel-style cell name.
     */
    static std::string toCellName(int row, int column);

    /**
     * Converts the given Excel-style cell name into a 0-based row and column,
     * setting both of the given integer output parameters by reference.
     * For example, passing "C7" sets row to 6 (7-1) and column to 2 ('A' + 2 -> 'C').
     * Returns true if successful and false if not.
     */
    static bool toRowColumn(const std::string& cellname, int& row, int& column);

    /**
     * Extracts the alphabetic column name from an Excel-style cell name such as "C7"
     * and converts it into a 0-based column index such as 2 for 'C'.
     * If the given string is not properly formatted, returns -1.
     */
    static int toColumn(const std::string& cellname);

    /**
     * Extracts the 1-based row number from an Excel-style cell name such as "C7"
     * and converts it into a 0-based row index such as 6.
     * If the given string is not properly formatted, returns -1.
     */
    static int toRow(const std::string& cellname);

    /**
     * Returns a string representation of this range such as "A1:B7".
     */
    std::string toString() const;

private:
    // set of all known function names, in uppercase (such as "SUM" and "AVERAGE")
    static const Set<std::string> FUNCTION_NAMES;

    // Excel-style cell names of start/end of this range (e.g. "A5" or "C7")
    std::string startCellName;
    std::string endCellName;

    /**
     * Returns true if the start row/col come before the end row/col
     * and all are non-negative.
     */
    bool isValid() const;
};

std::ostream& operator <<(std::ostream& out, const Range& range);

/**
 * Implementation of built-in functions supported in cell formulas. Each
 * takes a vector of double values and returns the computed result.
 */
double average(const Vector<double>& values);
double sum(const Vector<double>& values);
double product(const Vector<double>& values);
double max(const Vector<double>& values);
double min(const Vector<double>& values);
double median(const Vector<double>& values);
double stdev(const Vector<double>& values);

#endif // _range_h
