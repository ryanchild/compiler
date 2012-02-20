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
      NONE = 0,
      INTEGER,
      FLOAT,
      BOOLEAN,
      STRING
    };

    enum symboltype
    {
      SCALAR,
      ARRAY,
      FUNCTION
    };

    struct Symbol
    {
      symboltype st;
      long address;
      int size;
    };

  private:
    std::string getSignature(const char*, datatype, std::vector<datatype>&);
    const char* getNameFromSignature(const char*);
    char getDataType(datatype);
    Token nextToken();
    bool nextTokenIs(Token::tokentype tt);
    datatype getNumberType() 
    {
      return strchr(mTok.getString(),'.') ? FLOAT : INTEGER;
    }

    bool typemark();
    bool variabledecl();
    bool declaration();
    bool ifstatement();
    bool loopstatement();
    bool functioncall();
    bool argumentlist();
    bool name();
    bool factor(datatype&);
    bool term(datatype&);
    bool term2(datatype&);
    bool relation();
    bool relation2();
    bool arithop();
    bool arithop2();
    bool expression(datatype&);
    bool expression2();
    bool destination();
    bool assignmentstatement();
    bool statement();
    bool functionbody();
    bool parameterlist(std::vector<datatype>&);
    bool functionheader(bool global=false);
    bool functiondecl(bool global=false);

    Scanner* mScanner;
    Token mTok;
    bool mPreScanned;
    bool mError;
    int mLevel;

    typedef std::map<std::string, Symbol> SymbolTable;
    typedef SymbolTable::iterator SymbolTableIt;
    SymbolTable mGlobalSymbols;
    SymbolTable mLocalSymbols;
};

#endif //PARSER_H
