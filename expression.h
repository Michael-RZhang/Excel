/**
 * CS 106B/X Stanford 1-2-3
 * This interface defines a class hierarchy for arithmetic expressions.
 *
 * Please do not modify this provided file. Your turned-in files should work
 * with an unmodified version of all provided code files.
 *
 * @author Marty Stepp, Julie Zelenski, Jerry Cain
 * @version 2016/11/29
 * - 16au 106x version (stepp)
 */

#ifndef _expression_h
#define _expression_h

#include <string>
#include "map.h"
#include "set.h"
#include "range.h"
#include "tokenscanner.h"

// forward declarations
class EvaluationContext;
class Parser;
class Spreadsheet;
class Expression;
class CompoundExp;
class DoubleExp;
class IdentifierExp;
class RangeExp;
class TextStringExp;

/**
 * This enumerated type is used to differentiate the five different
 * expression types.
 */
enum ExpressionType {COMPOUND, DOUBLE, IDENTIFIER, RANGE, TEXTSTRING};

/**
 * This parent class is used to represent a node in an expression tree.
 * Expression itself is an "abstract" class, which means that there are
 * never any objects whose actual type is Expression.  All objects are
 * instead created using one of five concrete subclasses:
 *
 *  1. DoubleExp     -- a numeric constant                  (such as 3.14 or 42)
 *  2. TextStringExp -- a text string constant              (such as "hello")
 *  3. IdentifierExp -- a string representing an identifier (such as "A5")
 *  4. CompoundExp   -- two expressions combined by an operator  (such as "B1+A2")
 *  5. RangeExp      -- a range of cells whose values are aggregated by a function (such as "SUM(B2:B5)")
 *
 * The Expression class defines the interface common to all expressions;
 * each subclass provides its own implementation of the common interface.
 */
class Expression {
public:
    /**
     * Specifies the constructor for the base Expression class.  Each subclass
     * constructor will define its own constructor.
     */
    Expression();

    /**
     * The destructor deallocates the storage for this expression.  This method
     * must be declared virtual to ensure that the correct subclass destructor
     * is called when deleting an expression.
     */
    virtual ~Expression();

    /**
     * Evaluates this expression and returns its value in the context of
     * the specified EvaluationContext object.
     * Also caches the value internally so that subsequent calls of getValue()
     * will return it without recalculating the value.
     */
    virtual double eval(Spreadsheet& model) = 0;

    /**
     * Returns the raw text that was passed in to parse this expression,
     * such as "=A2+SUM(B1:B6)*5".
     * This value will be an empty string unless this expression is the root
     * of the overall expression tree.
     */
    virtual std::string getRawText() const;

    /**
     * Returns the type of the expression, which must be one of the constants
     * COMPOUND, DOUBLE, RANGE, IDENTIFIER, or TEXTSTRING.
     */
    virtual ExpressionType getType() const = 0;

    /**
     * Returns the calculated value of this expression *without* re-calculating it.
     * If you have never called eval() on this expression before, this function
     * will return 0.0.
     * If you call eval(), the value calculated by eval will be saved in this
     * expression and returned from future calls to getValue.
     */
    virtual double getValue() const;

    /**
     * Returns true if this expression represents a formula that must be calculated.
     * A formula can be a reference to another cell (such as "=A2") or a more
     * complex expression involving operators (such as "=A2+B1-SUM(C4:C7)").
     */
    virtual bool isFormula() const;

    /**
     * Returns a string representation of this expression.
     */
    virtual std::string toString() const = 0;

    // Begin subclass-specific member functions; these are not well-defined
    // in the parent class and honestly shouldn't be here, but we include
    // them as empty 'stub' versions so that the student does not need to
    // perform pointer type-casting in order to call subclass behavior.
    // This is because inheritance has not been well covered by the time
    // the assignment is released this quarter.

    /**
     * Returns the name of the function being called in a range expression,
     * such as "AVERAGE".
     * If this expression is not a range expression, throws an ErrorException.
     */
    virtual std::string getFunction() const;

    /**
     * Returns the left-hand-side subexpression of a compound expression.
     * If this expression is not a compound expression, throws an ErrorException.
     */
    virtual const Expression* getLeft() const;

    /**
     * Returns the operator used in a compound expression, such as "+".
     * If this expression is not a compound expression, throws an ErrorException.
     */
    virtual std::string getOperator() const;

    /**
     * Returns the range of cells being referenced in a range expression,
     * such as B2:B17.
     * If this expression is not a range expression, throws an ErrorException.
     */
    virtual Range getRange() const;

    /**
     * Returns the left-hand-side subexpression of a compound expression.
     * If this expression is not a compound expression, throws an ErrorException.
     */
    virtual const Expression* getRight() const;

private:
    std::string rawText;
    double value;

    /**
     * Sets the raw text stored in this expression.
     * Called by Parser.
     */
    virtual void setRawText(const std::string& rawText);

    /**
     * Sets the value stored in this expression.
     * Called internally by eval().
     */
    virtual void setValue(double value);

    // allows the subclasses to call setValue, but not other client code
    friend class CompoundExp;
    friend class DoubleExp;
    friend class IdentifierExp;
    friend class RangeExp;
    friend class TextStringExp;
    friend class Parser;
};


/**
 * This subclass represents a compound expression consisting of
 * two subexpressions joined by an operator.
 */
class CompoundExp : public Expression {
public:
    /**
     * The constructor initializes a new compound expression composed of
     * the operator (op) and the left and right subexpression (lhs and rhs).
     */
    CompoundExp(const std::string& op, Expression* lhs, Expression* rhs);

    /** Frees the memory for this expression and its sub-expressions. */
    virtual ~CompoundExp();

    /** Returns the evaluated result of applying the operator to the left and right operands. */
    virtual double eval(Spreadsheet& model);

    /** Returns COMPOUND. */
    virtual ExpressionType getType() const;

    /** Returns a parenthesized version of this expression, such as "(A1 + B2)". */
    virtual std::string toString() const;

    /** Returns the left-hand-side subexpression of a compound expression. */
    virtual const Expression* getLeft() const;

    /** Returns the operator used in a compound expression. */
    virtual std::string getOperator() const;

    /** Returns the right-hand-side subexpression of a compound expression. */
    virtual const Expression* getRight() const;

private:
    std::string op;    // the operator string (+, -, *, /)
    Expression* lhs;
    Expression* rhs;   // the left and right subexpression

    /* set of all operators that can appear in a compound expression (e.g. "+", "-") */
    static Set<std::string> KNOWN_OPERATORS;
};


/**
 * This subclass represents a numeric constant.
 */
class DoubleExp : public Expression {
public:
    /** The constructor creates a new integer constant expression. */
    DoubleExp(double value);

    /** Just returns the double's value itself. */
    virtual double eval(Spreadsheet& model);

    /** Returns DOUBLE. */
    virtual ExpressionType getType() const;

    /** Returns the real number as a string, such as "3.14" or "42". */
    virtual std::string toString() const;
};


/**
 * This subclass represents an identifier used as a variable name, such as "A2".
 */
class IdentifierExp : public Expression {
public:
    /** The constructor creates an identifier expression with the specified name. */
    IdentifierExp(const std::string& name);

    /** Returns the value of the referred cell by asking the spreadsheet. */
    virtual double eval(Spreadsheet& model);

    /** Returns IDENTIFIER. */
    virtual ExpressionType getType() const;

    /** Returns the identifier such as "A2". */
    virtual std::string toString() const;

private:
    std::string name;   // the name of the identifier
};


/**
 * This subclass represents an expression consisting of a function applied to
 * a range of cell values, such as B2:B5 or A1:D7.
 */
class RangeExp : public Expression {
public:
    /**
     * Constructs a range expression that applies the given function to the
     * given range of cells, such as "AVERAGE" and B2:B5.
     */
    RangeExp(const std::string& function, Range cells);

    /**
     * Evaluates the expression by asking the spreadsheet for the values of
     * all cells in the range and then applying the given function to them.
     */
    virtual double eval(Spreadsheet& model);

    /** Returns RANGE. */
    virtual ExpressionType getType() const;

    /** Returns a string such as "AVERAGE(B2:B5)". */
    virtual std::string toString() const;

    /**
     * Returns the name of the function being called in a range expression,
     * such as "AVERAGE".
     */
    virtual std::string getFunction() const;

    /**
     * Returns the range of cells being referenced in a range expression,
     * such as B2:B17.
     */
    virtual Range getRange() const;

private:
    std::string function;
    Range cells;
};


/**
 * This subclass represents a text string constant.
 */
class TextStringExp : public Expression {
public:
    /** The constructor creates a new text string constant expression. */
    TextStringExp(const std::string& str);

    /** Returns 0.0 because strings have no numeric value. */
    double eval(Spreadsheet& model);

    /** Returns TEXTSTRING. */
    ExpressionType getType() const;

    /** Returns the string passed to the constructor. */
    std::string toString() const;

private:
    std::string str;   // the value of the text string constant
};


/**
 * This class encapsulates the information that the evaluator needs to
 * know in order to evaluate an expression.
 * Students do not need to use this class directly.
 */
class EvaluationContext {
public:
    /**
     * Sets the value associated with the specified var.
     */
    void setValue(const std::string& var, double value);

    /**
     * Returns the value associated with the specified variable.
     */
    double getValue(const std::string& var) const;

    /**
     * Returns true if the specified variable is defined.
     */
    bool isDefined(const std::string& var) const;

private:
    Map<std::string, double> symbolTable;
};

/**
 * Prints the given expression out to the given output stream.
 */
std::ostream& operator <<(std::ostream& out, const Expression& expr);

#endif // _expression_h
