#ifndef PARSER_H
#define PARSER_H

#include "Token.h"

class Scanner;

class Parser
{
  public:
    Parser(Scanner* s);
    void parse();

  private:
    bool identifier(bool decl = false);
    bool global();
    bool datatype(bool global = false);
    bool function();

    bool getNext();

    Scanner* mScanner;
    Token mTok;
};

#endif //PARSER_H
