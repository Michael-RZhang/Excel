/**
 * CS 106B/X Stanford 1-2-3
 * This file acts as the interface to the parser module.
 *
 * Please do not modify this provided file. Your turned-in files should work
 * with an unmodified version of all provided code files.
 *
 * @author Marty Stepp, Julie Zelenski, Jerry Cain
 * @version 2016/11/29
 * - 16au 106x version (stepp)
 */

#ifndef _parser_h
#define _parser_h

#include "expression.h"
#include "range.h"
#include "tokenscanner.h"

class Parser {
public:
    /**
     * Function: parseExp
     * Usage: Expression* exp = parseExp(rawText);
     * -------------------------------------------
     * Parses a complete expression from the specified text,
     * making sure that there are no tokens left in its scanner at the end.
     */
    static Expression* parseExpression(const std::string& rawText);

private:
    static Expression* readExpression(TokenScanner& scanner);
    static Expression* readFormula(TokenScanner& scanner, int prec = 0);
    static Range readRange(TokenScanner& scanner);
    static Expression* readTerm(TokenScanner& scanner);
    static int precedence(const std::string& token);
};

#endif // _parser_h
