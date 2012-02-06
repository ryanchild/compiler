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
    bool global();
    bool declaration();
    bool ifstatement();
    bool loopstatement();
    bool functioncall();
    bool argumentlist();
    bool name();
    bool factor();
    bool term();
    bool relation();
    bool arithop();
    bool expression();
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
};

#endif //PARSER_H
