/**
 * CS 106B/X Stanford 1-2-3
 * This file implements the expression.h interface.
 *
 * Please do not modify this provided file. Your turned-in files should work
 * with an unmodified version of all provided code files.
 *
 * @author Marty Stepp, Julie Zelenski, Jerry Cain
 * @version 2016/12/02
 * - made value default to 0.0 on Expression construction
 * @version 2016/11/29
 * - 16au 106x version (stepp)
 */

#include "expression.h"
#include "error.h"
#include "strlib.h"
#include "spreadsheet.h"

/**
 * Implementation notes: Expression
 * --------------------------------
 * The Expression class itself implements only those methods that
 * are not designated as pure virtual.
 */
Expression::Expression()
        : rawText(""),
          value(0.0) {
    /* Empty */
}

Expression::~Expression() {
    /* Empty */
}

std::string Expression::getRawText() const {
    return rawText;
}

double Expression::getValue() const {
    return value;
}

bool Expression::isFormula() const {
    ExpressionType type = getType();
    return type == IDENTIFIER || type == COMPOUND || type == RANGE;
}

void Expression::setRawText(const std::string& rawText) {
    this->rawText = rawText;
}

void Expression::setValue(double value) {
    this->value = value;
}

std::string Expression::getFunction() const {
    error("Expression::getFunction: called on a non-Range expression object");
    return "";
}

const Expression* Expression::getLeft() const {
    error("Expression::getLeft: called on a non-Compound expression object");
    return nullptr;
}

std::string Expression::getOperator() const {
    error("Expression::getOperator: called on a non-Compound expression object");
    return "";
}

Range Expression::getRange() const {
    error("Expression::getRange: called on a non-Range expression object");
    return Range();
}

const Expression* Expression::getRight() const {
    error("Expression::getRight: called on a non-Compound expression object");
    return nullptr;
}

/**
 * Implementation notes: CompoundExp
 * ---------------------------------
 * The implementation of eval for CompoundExp evaluates the left and right
 * subexpressions recursively and then applies the operator.  Assignment is
 * treated as a special case because it does not evaluate the left operand.
 * Invariant: lhs != nullptr && rhs != nullptr
 */
Set<std::string> CompoundExp::KNOWN_OPERATORS {"+", "-", "*", "/"};

CompoundExp::CompoundExp(const std::string& op, Expression* lhs, Expression* rhs) {
    this->op = op;
    if (!lhs) {
        error("CompoundExp::constructor: null left sub-expression");
    }
    if (!rhs) {
        error("CompoundExp::constructor: null right sub-expression");
    }
    this->lhs = lhs;
    this->rhs = rhs;
}

CompoundExp::~CompoundExp() {
    delete lhs;
    delete rhs;
}

double CompoundExp::eval(Spreadsheet& model) {
    if (!KNOWN_OPERATORS.contains(op)) {
        error("Illegal operator in expression: " + op);
    }
    double right = rhs->eval(model);
    double left = lhs->eval(model);
    double result = 0.0;
    if (op == "+") {
        result = left + right;
    } else if (op == "-") {
        result = left - right;
    } else if (op == "*") {
        result = left * right;
    } else if (op == "/") {
        result = left / right; // divide by 0.0 gives +/- INF
    }
    setValue(result);
    return result;
}

const Expression* CompoundExp::getLeft() const {
    return lhs;
}

std::string CompoundExp::getOperator() const {
    return op;
}

const Expression* CompoundExp::getRight() const {
    return rhs;
}

ExpressionType CompoundExp::getType() const {
    return COMPOUND;
}

std::string CompoundExp::toString() const {
    return '(' + lhs->toString() + ' ' + op + ' ' + rhs->toString() + ')';
}

/**
 * Implementation notes: DoubleExp
 * -------------------------------
 * The DoubleExp subclass represents a numeric constant.
 * The eval method simply returns that value.
 */
DoubleExp::DoubleExp(double value) {
    setValue(value);
}

double DoubleExp::eval(Spreadsheet& /* model */) {
    return getValue();
}

ExpressionType DoubleExp::getType() const {
    return DOUBLE;
}

std::string DoubleExp::toString() const {
    return realToString(value);
}

/**
 * Implementation notes: IdentifierExp
 * -----------------------------------
 * The IdentifierExp subclass represents a variable name.
 * The implementation of eval looks up that name in the evaluation context
 * using your spreadsheet to get its value.
 */
IdentifierExp::IdentifierExp(const std::string& name) {
    this->name = name;
}

double IdentifierExp::eval(Spreadsheet& model) {
    if (!Range::isValidName(name)) {
        error(name + " is not valid cell name.");
    }
    double result = model.getCellCalculatedValue(name);
    setValue(result);
    return result;
}

ExpressionType IdentifierExp::getType() const {
    return IDENTIFIER;
}

std::string IdentifierExp::toString() const {
    return name;
}

/**
 * Implementation notes: RangeExp
 * ------------------------------
 * None.
 */
RangeExp::RangeExp(const std::string& function, Range cells) {
    this->function = toUpperCase(trim(function));
    this->cells = cells;
}

double RangeExp::eval(Spreadsheet& model) {
    if (!Range::isKnownFunctionName(this->function)) {
        error("Unknown function name: " + function);
    }
    Vector<double> valuesInRange;
    model.fillFromRange(cells, valuesInRange);
    double result = 0.0;
    if (function == "AVERAGE" || function == "MEAN") {
        result = average(valuesInRange);
    } else if (function == "SUM") {
        result = sum(valuesInRange);
    } else if (function == "PRODUCT") {
        result = product(valuesInRange);
    } else if (function == "MAX") {
        result = max(valuesInRange);
    } else if (function == "MIN") {
        result = min(valuesInRange);
    } else if (function == "MEDIAN") {
        result = median(valuesInRange);
    } else if (function == "STDEV") {
        result = stdev(valuesInRange);
    } else {
        error("Unknown function name: " + function);
    }
    setValue(result);
    return result;
}

std::string RangeExp::getFunction() const{
    return function;
}

Range RangeExp::getRange() const{
    return cells;
}

ExpressionType RangeExp::getType() const {
    return RANGE;
}

std::string RangeExp::toString() const {
    return function + "(" + cells.toString() + ")";
}

/**
 * Implementation notes: TextStringExp
 * -----------------------------------
 * The IdentifierExp subclass represents a text string constant.  The
 * implementation of eval simply returns 0.0.
 */
TextStringExp::TextStringExp(const std::string& str) {
    this->str = str;
    setValue(0.0);
}

double TextStringExp::eval(Spreadsheet& /* model */) {
    return 0.0;
}

std::string TextStringExp::toString() const {
    return str;
}

ExpressionType TextStringExp::getType() const {
    return TEXTSTRING;
}

/**
 * Implementation notes: EvaluationContext
 * ---------------------------------------
 * The methods in the EvaluationContext class simply call the appropriate
 * method on the map used to represent the symbol table.
 */
void EvaluationContext::setValue(const std::string& var, double value) {
    symbolTable.put(var, value);
}

double EvaluationContext::getValue(const std::string& var) const {
    return symbolTable.get(var);
}

bool EvaluationContext::isDefined(const std::string& var) const {
    return symbolTable.containsKey(var);
}

std::ostream& operator <<(std::ostream& out, const Expression& expr) {
    out << expr.toString();
    return out;
}
