/**
 * CS 106B/X Stanford 1-2-3
 * This file implements the Range type that can be used by your spreadsheet.
 *
 * Please do not modify this provided file. Your turned-in files should work
 * with an unmodified version of all provided code files.
 *
 * @author Marty Stepp, Julie Zelenski, Jerry Cain
 * @version 2016/11/29
 * - 16au 106x version (stepp)
 */

#include "range.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include "error.h"

// all functions allowed in a range expression
const Set<std::string> Range::FUNCTION_NAMES {
    "AVERAGE", "MAX", "MEAN", "MEDIAN", "MIN", "PRODUCT", "STDEV", "SUM"
};

Range::Range(int startRow, int startColumn, int endRow, int endColumn) {
    startCellName = toCellName(startRow, startColumn);
    endCellName = toCellName(endRow, endColumn);
    if (!isValidName(startCellName)) {
        error("Range::constructor: invalid start cell name: " + startCellName);
    }
    if (!isValidName(endCellName)) {
        error("Range::constructor: invalid end cell name: " + endCellName);
    }
    if (!isValid()) {
        error("Range::constructor: invalid range: " + toString());
    }
}

Range::Range(const std::string& startCellName, const std::string& endCellName) :
        startCellName(startCellName),
        endCellName(endCellName) {
    if (!isValidName(startCellName)) {
        error("Range::constructor: invalid start cell name: " + startCellName);
    }
    if (!isValidName(endCellName)) {
        error("Range::constructor: invalid end cell name: " + endCellName);
    }
}

Set<std::string> Range::getAllCellNames() const {
    Set<std::string> cellnames;
    int startRow = getStartRow();
    int startCol = getStartColumn();
    int endRow = getEndRow();
    int endCol = getEndColumn();
    for (int row = startRow; row <= endRow; row++) {
        for (int col = startCol; col <= endCol; col++) {
            std::string cellname = toCellName(row, col);
            cellnames.add(cellname);
        }
    }
    return cellnames;
}

std::string Range::getEndCellName() const {
    return endCellName;
}

int Range::getEndColumn() const {
    int row, col;
    toRowColumn(endCellName, row, col);
    return col;
}

int Range::getEndRow() const {
    int row, col;
    toRowColumn(endCellName, row, col);
    return row;
}

std::string Range::getStartCellName() const {
    return startCellName;
}

int Range::getStartColumn() const {
    int row, col;
    toRowColumn(startCellName, row, col);
    return col;
}

int Range::getStartRow() const {
    int row, col;
    toRowColumn(startCellName, row, col);
    return row;
}

bool Range::isKnownFunctionName(const std::string& function) {
    return FUNCTION_NAMES.contains(toUpperCase(function));
}

bool Range::isValid() const {
    int startRow, startCol, endRow, endCol;
    if (!toRowColumn(startCellName, startRow, startCol)
            || !toRowColumn(endCellName, endRow, endCol)) {
        return false;
    }
    return 0 <= startRow && startRow <= endRow
        && 0 <= startCol && startCol <= endCol;
}

bool Range::isValidName(const std::string& cellname) {
    int row, col;
    return toRowColumn(cellname, row, col);
}

std::string Range::toCellName(int row, int column) {
    if (row < 0 || column < 0) {
        error("Range::toCellName: row/column cannot be negative");
    }

    // convert column into a roughly base-26 Excel column name,
    // e.g. 0 -> "A", 1 -> "B", 26 -> "AA", ...
    std::string colStr;
    int col = column + 1;   // 1-based
    while (col-- > 0) {
        colStr = charToString((char) ('A' + (col % 26))) + colStr;
        col /= 26;
    }
    std::string rowStr = integerToString(row + 1);
    return colStr + rowStr;
}

bool Range::toRowColumn(const std::string& cellname, int& row, int& column) {
    int rowTemp = toRow(cellname);
    int colTemp = toColumn(cellname);
    if (rowTemp >= 0 && colTemp >= 0) {
        // fill in reference parameters
        row = rowTemp;
        column = colTemp;
        return true;
    } else {
        return false;
    }
}

int Range::toColumn(const std::string& cellname) {
    // chomp out the row at end and keep only the column
    std::string colStr = trim(toUpperCase(cellname));
    while (!colStr.empty() && !isalpha(colStr[colStr.length() - 1])) {
        colStr.erase(colStr.length() - 1, 1);
    }
    if (colStr.empty() || !isalpha(colStr[0])) {
        return -1;
    }

    // convert alphabetic column letters into a roughly base-26 column index
    int colNum = 0;
    for (int i = 0; i < (int) colStr.length(); i++) {
        char ch = colStr[i];
        if (!isalpha(ch)) {
            return -1;
        } else {
            colNum = colNum * 26 + (ch - 'A' + 1);
        }
    }
    colNum--;   // convert 1-based to 0-based
    return colNum;
}

int Range::toRow(const std::string& cellname) {
    // chomp out the column at start and keep only the row
    std::string rowStr = trim(toUpperCase(cellname));
    while (!rowStr.empty() && !isdigit(rowStr[0])) {
        rowStr.erase(0, 1);
    }
    if (stringIsInteger(rowStr)) {
        // convert 1-based to 0-based
        return stringToInteger(rowStr) - 1;
    } else {
        return -1;
    }
}

std::string Range::toString() const {
    std::ostringstream out;
    out << *this;
    return out.str();
}

std::ostream& operator <<(std::ostream& out, const Range& range) {
    return out << range.getStartCellName() << ":" << range.getEndCellName();
}

double min(const Vector<double>& values) {
    double min = values[0];
    for (int i = 1; i < values.size(); i++) {
        if (values[i] < min) {
            min = values[i];
        }
    }
    return min;
}

double max(const Vector<double>& values) {
    double max = values[0];
    for (int i = 1; i < values.size(); i++) {
        if (values[i] > max) {
            max = values[i];
        }
    }
    return max;
}

double sum(const Vector<double>& values) {
    double sum = 0;
    for (double n : values) {
        sum += n;
    }
    return sum;
}

double product(const Vector<double>& values) {
    double prod = 1;
    for (double n : values) {
        prod *= n;
    }
    return prod;
}

/* This function should be accessible by both name "mean" and "average" */
double average(const Vector<double>& values) {
    return sum(values) / values.size();
}

double median(const Vector<double>& values) {
    Vector<double> clone = values;
    sort(clone.begin(), clone.end());
    int sz = values.size();
    if (sz % 2 == 0) {
        return (clone[sz / 2] + clone[sz / 2 - 1]) / 2;
    } else {
        return clone[sz / 2];
    }
}

double stdev(const Vector<double>& values) {
    double sum = 0;
    double sumsquares = 0;
    for (double n : values) {
        sum += n;
        sumsquares += n * n;
    }
    int sz = values.size();
    return sqrt((sz * sumsquares - sum * sum) / (sz * sz));
}
