#ifndef PARSER_H
#define PARSER_H

#include "Token.h"
#include <map>
#include <vector>

class Scanner;

class Parser
{
  public:
    Parser(Scanner* s);
    bool parse();
    void initialize();

    enum datatype
    {
      INTEGER,
      FLOAT,
      BOOLEAN,
      STRING
    };

    enum symboltype
    {
      DATA,
      FUNCTION
    };

    struct Symbol
    {
      symboltype st;
      datatype dt;
      long address;
    };

  private:
    std::string getSignature(const char*, datatype, std::vector<datatype>&);
    char getDataType(datatype);
    Token nextToken();
    bool nextTokenIs(Token::tokentype tt);

    bool typemark();
    bool variabledecl();
    bool declaration(bool toplevel=false);
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
    bool functionbody(bool toplevel=false);
    bool parameterlist();
    bool functionheader();
    bool functiondecl(bool toplevel=false);

    Scanner* mScanner;
    Token mTok;
    bool mPreScanned;
    bool mError;

    std::map<std::string, Symbol> mGlobalSymbols;
    std::map<std::string, Symbol> mLocalSymbols;
};

#endif //PARSER_H
