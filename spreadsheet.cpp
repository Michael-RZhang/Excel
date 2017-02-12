
#include "spreadsheet.h"
#include "view.h"
#include "parser.h"
#include "error.h"

using namespace std;

Spreadsheet::Spreadsheet(View* view) {
    // constructor
    // ErrorException by calling the error function.
    if (view == nullptr) {
        error("view is a nullptr");
    }
    this->view = view;
}

Spreadsheet::~Spreadsheet() {
    // destructor
    clear();
}

bool Spreadsheet::cellIsFormula(const string& cellname) const {
    // check if the cell is a formula
    if (cellGraph.containsVertex(cellname)) {
        if (cellGraph.getVertex(cellname)->data == nullptr) return false;
        else return cellGraph.getVertex(cellname)->data->isFormula();
    }
    return false;
}

void Spreadsheet::clear() {
    // iterate thru the vertex to free the data memory
    for (VertexV<Expression*>* vertex : cellGraph.getVertexSet()) {
        delete vertex->data;
    }
    // delete the vertex
    cellGraph.clear();
    view->clearCells();
}

void Spreadsheet::fillFromRange(const Range& range, Vector<double>& values) {
    // loop over the range and put all the values in the vector
    int startRow = range.getStartRow();
    int startCol = range.getStartColumn();
    int endRow = range.getEndRow();
    int endCol = range.getEndColumn();
    for (int i = startCol; i <= endCol; i++ ) {
        for (int j = startRow; j <= endRow; j++) {
            string cellname = Range::toCellName(j, i);
            // get the value and add to the vector
            if (cellGraph.getVertex(cellname)->data == nullptr) {
                    values.add(0);
            }
            else values.add(cellGraph.getVertex(cellname)->data->getValue());

        }
    }
}

double Spreadsheet::getCellCalculatedValue(const string& cellname) const {
    // return the calculated value
    if (cellGraph.containsVertex(cellname)) {
        // if nothing in where return 0
        if (cellGraph.getVertex(cellname)->data == nullptr) return 0.0;
        else return cellGraph.getVertex(cellname)->data->getValue();
    }
    return 0;
}

string Spreadsheet::getCellRawText(const string& cellname) const {
    // return the raw text
    if (cellGraph.containsVertex(cellname)) {
        if (cellGraph.getVertex(cellname)->data == nullptr) return "";
        else return cellGraph.getVertex(cellname)->data->getRawText();
    }
    return "";
}

void Spreadsheet::load(istream& infile) {

    // clear the old memory
    clear();
    // read in the file
    while (!infile.fail()) {
        string cellname, rawText;
        infile >> cellname;
        getline(infile, rawText);
        if (infile.fail()) break;
        setCell(cellname, rawText);
    }
}

void Spreadsheet::save(ostream& outfile) const {

    // traverse the vertices and output them into the format
    for (VertexV<Expression*>* vertex : cellGraph.getVertexSet()) {
        if (vertex->data != nullptr) {
            outfile << vertex->name
                    << " " << vertex->data->getRawText()
                    << endl;
        }

    }
}

void Spreadsheet::setCell(const string& cellname, const string& rawText) {

    Expression* exp;
    // parse rawText into expression object
    try{
        exp = Parser::parseExpression(rawText);
    } catch(exception ex) {
        error("invalid input:" + rawText);
    }

    // check if the cell exists or not, if not add it
    if (!cellGraph.containsVertex(cellname)) {
        cellGraph.addVertex(cellname);
    }

    // make sure not create a circle
    if (checkCircle(exp, cellname)) {
        error("circular reference");
    }

    // first remove all out-bound, old edges
    removeEdge(cellname);
    // add edges and evaluate the cell
    setCellHelper(exp, cellname);
    cellGraph.getVertex(cellname)->data = exp;
    cellGraph.getVertex(cellname)->data->eval(*this);

    // update the cells dependent on it and display them
    updateCellHelper(cellname);

    // display
    display(cellname);
}

void Spreadsheet::setCellHelper(Expression*& exp, const string& cellname) {

    // find all its dependency and add edges
    // not add duplicate edges
    if (exp->getType() == COMPOUND) {
        // like "=A1+B2", just keep traversing down
        Expression* left = (Expression*) exp->getLeft();
        Expression* right = (Expression*)  exp->getRight();
        setCellHelper(left, cellname);
        setCellHelper(right, cellname);
    } else if (exp->getType() == RANGE) {
        // like "=SUM(C3:C8)", stop going down and add edges
        Range range = exp->getRange();
        int startRow = range.getStartRow();
        int startCol = range.getStartColumn();
        int endRow = range.getEndRow();
        int endCol = range.getEndColumn();
        // loop over all the cells involved
        for (int i = startCol; i <= endCol; i++ ) {
            for (int j = startRow; j <= endRow; j++) {
                string newcellname = Range::toCellName(j, i);

                if (!cellGraph.containsVertex(newcellname)) {
                    cellGraph.addVertex(newcellname);
                }
                if (!cellGraph.containsEdge(cellname, newcellname)) {
                    cellGraph.addEdge(cellname, newcellname);
                }
            }
        }

    } else if (exp->getType() == IDENTIFIER) {
        // identifier like "=A1", add an edge
        string newcellname = exp->toString();
        if (!cellGraph.containsVertex(newcellname)){
            cellGraph.addVertex(newcellname);
        }
        if (!cellGraph.containsEdge(cellname, newcellname)) {
            cellGraph.addEdge(cellname, newcellname);
        }
    }
}

void Spreadsheet::updateCellHelper(const string& cellname) {
    // update the dependent cells
    for (VertexV<Expression*>* invNeighbor : cellGraph.getInverseNeighbors(cellname)) {
        string newcellname = invNeighbor->name;
        invNeighbor->data->eval(*this);
        // display
        display(newcellname);
        updateCellHelper(newcellname); 
    }

    // itself
    cellGraph.getVertex(cellname)->data->eval(*this);
    // display
    display(cellname);
}

void Spreadsheet::removeEdge(const string& cellname) {
    // remove all the existing, out-bound edges since the rawtext changes
    for (VertexV<Expression*>* neighbor : cellGraph.getNeighbors(cellname)) {
        Expression* exp = neighbor->data;
        cellGraph.removeEdge(cellGraph.getVertex(cellname), neighbor);
    }
}

bool Spreadsheet::checkCircle(Expression*& exp, const string& cellname){
    // return true if there is a circle.
    if (exp->getType() == COMPOUND) {
        // like "=A1+B2", just keep traversing down
        Expression* left = (Expression*)  exp->getLeft();
        Expression* right = (Expression*)  exp->getRight();
        return checkCircle(left, cellname) || checkCircle(right, cellname);
    } else if (exp->getType() == RANGE) {
        // like "=SUM(C3:C8)", loop over each and do the recursion
        Range range = exp->getRange();
        int startRow = range.getStartRow();
        int startCol = range.getStartColumn();
        int endRow = range.getEndRow();
        int endCol = range.getEndColumn();
        // loop over all the cells involved
        for (int i = startCol; i <= endCol; i++ ) {
            for (int j = startRow; j <= endRow; j++) {
                string newcellname = Range::toCellName(j, i);
                if (cellname == newcellname) return true;
                // go down more layers of cells
                else if ( cellGraph.containsVertex(newcellname) &&
                    cellGraph.getVertex(newcellname)->data != nullptr &&
                    checkCircle(cellGraph.getVertex(newcellname)->data, cellname)) {
                    return true;
                }
            }
        }
    } else if (exp->getType() == IDENTIFIER) {
        // identifier like "=A1", add an edge
        string newcellname = exp->toString();
        if (newcellname == cellname) return true;
        else if ( cellGraph.containsVertex(newcellname) &&
            cellGraph.getVertex(newcellname)->data != nullptr &&
            checkCircle(cellGraph.getVertex(newcellname)->data, cellname)) {
            return true;
        }
    }
    return false;
}

void Spreadsheet::display(const string& cellname) {
    if (!cellGraph.getVertex(cellname)->data->isFormula()) {
        // if not formula, display rawText
        if ( cellGraph.getVertex(cellname)->data->getType() == TEXTSTRING) {
            // if it is textstring, isformula is not good enough for "=1" case
            view->displayCell(cellname, cellGraph.getVertex(cellname)->data->getRawText());
        }
        else {
            view->displayCell(cellname, realToString(getCellCalculatedValue(cellname)));
        }
    }
    else {
        // If is formula, display the value
        double displayValue = cellGraph.getVertex(cellname)->data->getValue();
        view->displayCell(cellname, realToString(displayValue));
    }
}
