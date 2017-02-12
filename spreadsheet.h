
#ifndef _spreadsheet_h
#define _spreadsheet_h

#include <iostream>
#include <string>
#include "range.h"
#include "vector.h"
#include "view.h"
#include "basicgraph.h"
#include "expression.h"
using namespace std;

class Spreadsheet {
public:
    Spreadsheet(View* view);
    ~Spreadsheet();

    bool cellIsFormula(const string& cellname) const;
    void clear();
    void fillFromRange(const Range& range, Vector<double>& values);
    double getCellCalculatedValue(const string& cellname) const;
    string getCellRawText(const string& cellname) const;
    void load(istream& infile);
    void save(ostream& outfile) const;
    void setCell(const string& cellname, const string& rawText);

private:

    BasicGraphV<Expression*> cellGraph;
    View* view;
    void setCellHelper(Expression*& exp, const string& cellname);
    void updateCellHelper(const string& cellname);
    void removeEdge(const string& cellname);
    bool checkCircle(Expression*& exp, const string& cellname);
    void display(const string& cellname);

};

#endif // _spreadsheet_h
