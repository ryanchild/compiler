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

    bool ifstatement();
    bool loopstatement();
    bool typemark();
    bool variabledecl();
    bool global();
    bool declaration();
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
