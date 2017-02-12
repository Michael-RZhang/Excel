/**
 * CS 106B/X Stanford 1-2-3
 * This file implements the parser.h interface.
 *
 * Please do not modify this provided file. Your turned-in files should work
 * with an unmodified version of all provided code files.
 *
 * @author Marty Stepp, Julie Zelenski, Jerry Cain
 * @version 2016/11/29
 * - 16au 106x version (stepp)
 */

#include "parser.h"
#include <iostream>
#include <string>
#include "error.h"
#include "set.h"
#include "strlib.h"
#include "tokenscanner.h"
#include "expression.h"
#include "range.h"

// set to true to see debug messages related to parsing
static const bool DEBUG = false;

/**
 * Implementation notes: parseExp
 * ------------------------------
 * This code just reads an expression and then checks for extra tokens.
 */
Expression* Parser::parseExpression(const std::string& rawText) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.scanStrings();
    scanner.setInput(rawText);
    Expression* exp = readExpression(scanner);
    exp->setRawText(rawText);
    return exp;
}

/**
 * Implementation notes: precedence
 * --------------------------------
 * This function checks the token against each of the defined operators
 * and returns the appropriate precedence value.
 */
int Parser::precedence(const std::string& token) {
    if (token == "+" || token == "-") {
        return 1;
    } else if (token == "*" || token == "/") {
        return 2;
    } else {
        return 0;
    }
}

/**
 * Implementation notes: readExpression
 * ------------------------------------
 * This function scans an overall cell expression.
 * An expression can be a number, a string, or a formula.
 */
Expression* Parser::readExpression(TokenScanner& scanner) {
    if (DEBUG) std::cout << "  readExpr(" << scanner << ")" << std::endl;
    std::string token = scanner.nextToken();
    TokenType type = scanner.getTokenType(token);
    if (token == "=") {
        // beginning of a formula
        Expression* exp = readFormula(scanner);
        if (scanner.hasMoreTokens()) {
            error("Parse error: Unexpected token: \"" + scanner.nextToken() + "\"");
        }
        return exp;
    } else if (type == NUMBER && !scanner.hasMoreTokens()) {
        return new DoubleExp(stringToReal(token));
    } else {
        return new TextStringExp(trim(scanner.getInput()));
    }
}

/**
 * Implementation notes: readFormula
 * Usage: exp = readFormula(scanner, prec);
 * ----------------------------------------
 * This implementation uses precedence to resolve the ambiguity in
 * the grammar.  At each level, the parser reads operators and subexpressions
 * until it finds an operator whose precedence is greater than the prevailing
 * one.  When a higher-precedence operator is found, readE calls itself
 * recursively to read that subexpression as a unit.
 */
Expression* Parser::readFormula(TokenScanner& scanner, int prec) {
    if (DEBUG) std::cout << "  readForm(" << scanner << "), prec=" << prec << ")" << std::endl;
    Expression* exp = readTerm(scanner);
    std::string token;
    while (true) {
        // read operator
        token = scanner.nextToken();
        int tprec = precedence(token);
        if (tprec <= prec) {
            break;
        }

        if (!scanner.hasMoreTokens()) {
            error("Parse error: Invalid binary " + token
                  + " expression; missing right operand");
            exp = nullptr;
        } else {
            Expression* rhs = readFormula(scanner, tprec);
            exp = new CompoundExp(token, exp, rhs);
        }
    }
    scanner.saveToken(token);
    return exp;
}

/**
 * Implementation notes: readRange
 * Usage: exp = readRange(scanner);
 * --------------------------------
 * This function scans a range of cells, such as A1:A7.
 */
Range Parser::readRange(TokenScanner& scanner) {
    if (DEBUG) std::cout << "  readRang(" << scanner << ")" << std::endl;
    if (scanner.nextToken() != "(") {
        error("Parse error: Invalid range format; missing initial (.");
    }
    std::string startCellName = scanner.nextToken();
    if (!Range::isValidName(startCellName)) {
        error("Parse error: Invalid start cell name for range: \"" + startCellName + "\"");
    }

    std::string sep = scanner.nextToken();
    if (sep != ":" && sep != "-") {
        error("Parse error: Invalid range format; missing : in middle.");
    }
    std::string endCellName = scanner.nextToken();
    if (!Range::isValidName(endCellName)) {
        error("Parse error: Invalid end cell name for range: \"" + endCellName + "\"");
    }
    if (scanner.nextToken() != ")") {
        error("Parse error: Invalid range format; missing final ).");
    }

    //Check that is valid values for range
    Range rng(startCellName, endCellName);
    return rng;
}

/**
 * Implementation notes: readTerm
 * ------------------------------
 * This function scans a term, which is either an integer, an identifier,
 * or a parenthesized subexpression.
 */
Expression* Parser::readTerm(TokenScanner& scanner) {
    if (DEBUG) std::cout << "readTerm(" << scanner << ")" << std::endl;
    std::string token = scanner.nextToken();
    TokenType type = scanner.getTokenType(token);
    Expression* result = nullptr;
    if (token == "(") {
        // beginning of a parenthesized expression
        Expression* exp = readFormula(scanner);
        token = scanner.nextToken();
        if (token != ")") {
            error("Parse error: Unclosed parenthesis.");
        } else {
            result = exp;
        }
    } else if (type == NUMBER) {
        result = new DoubleExp(stringToReal(token));
    } else if (type == WORD) {
        token = toUpperCase(token);
        if (Range::isKnownFunctionName(token)) {
            result = new RangeExp(token, readRange(scanner));
        } else if (Range::isValidName(token)) {
            result = new IdentifierExp(token);
        } else {
            error("Parse error: Invalid cell name or token: \"" + token + "\"");
        }
    } else {
        result = new TextStringExp(token);
    }
    return result;
}
