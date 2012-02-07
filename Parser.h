#ifndef PARSER_H
#define PARSER_H

#include "Token.h"

class Scanner;

class Parser
{
  public:
    Parser(Scanner* s);
    bool parse();

  private:
    Token nextToken();
    void setPreScanned() { mPreScanned = true; }

    bool typemark();
    bool variabledecl();
    bool declaration();
    bool ifstatement();
    bool loopstatement();
    bool functioncall();
    bool argumentlist();
    bool name();
    bool factor();
    bool term();
    bool term2();
    bool relation();
    bool relation2();
    bool arithop();
    bool arithop2();
    bool expression();
    bool expression2();
    bool destination();
    bool assignmentstatement();
    bool statement();
    bool functionbody();
    bool parameterlist();
    bool identifier();
    bool functionheader();
    bool functiondecl();

    Scanner* mScanner;
    Token mTok;
    bool mPreScanned;
    bool mError;
};

#endif //PARSER_H
