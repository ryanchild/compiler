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
      INTEGER = 0x0,
      BOOLEAN = 0x01,
      FLOAT   = 0x02,
      STRING  = 0x04
    };
    
    enum structuretype
    {
      SCALAR,
      ARRAY,
      FUNCTION
    };

    class SymbolType
    {
      public:
        SymbolType()
          :mSt(SCALAR)
          ,mDt(INTEGER)
          ,mSize(0)
        {}

        SymbolType(structuretype st,
                   datatype dt)
          :mSt(st)
          ,mDt(dt)
          ,mSize(0)
        {}

        SymbolType(structuretype st,
                   datatype dt,
                   int size)
          :mSt(st)
          ,mDt(dt)
          ,mSize(size)
        {}

        SymbolType(structuretype st,
                   datatype dt,
                   int size,
                   std::vector<SymbolType> params)
          :mSt(st)
          ,mDt(dt)
          ,mSize(size)
          ,mParams(params)
          {}

        void setDataType(datatype dt) { mDt = dt; }
        datatype getDataType() const { return mDt; }
        structuretype getStructureType() const { return mSt; }

      private:
        structuretype mSt;
        datatype mDt;
        int mSize;
        std::vector<SymbolType> mParams;
    };

    class Symbol
    {
      public:
        Symbol(const char* id, SymbolType st, long addr)
          :mId(id)
          ,mSt(st)
          ,mAddr(addr)
        {}
        
        Symbol()
          :mId(NULL)
          ,mSt(SymbolType())
          ,mAddr(0)
        {}

        datatype getDataType() const { return mSt.getDataType(); }
        structuretype getStructureType() const 
        { 
          return mSt.getStructureType(); 
        }
        long getAddr() const { return mAddr; }

      private:
        const char* mId;
        SymbolType mSt;
        long mAddr;
    };

  private:
    typedef std::map<std::string, Symbol> SymbolTable;
    typedef SymbolTable::iterator SymbolTableIt;

    Token nextToken();
    bool nextTokenIs(Token::tokentype tt);
    datatype getNumberType() 
    {
      return strchr(mTok.getString(),'.') ? FLOAT : INTEGER;
    }
    bool arithOpCompatible(datatype dt1, datatype dt2) 
    { 
      // no strings or booleans
      return !((dt1 | dt2) & 0x05);
    }
    bool relationalOpCompatible(datatype dt1, datatype dt2) 
    { 
      // no floats or strings
      return !((dt1 | dt2) & 0x06);
    }
    bool bitwiseOpCompatible(datatype dt1, datatype dt2)
    {
      // ints only
      return !(dt1 | dt2);
    }
    datatype tokenTypeToDataType(Token::tokentype tt)
    {
      return tt == Token::INTEGER ? INTEGER :
             tt == Token::FLOAT ? FLOAT :
             tt == Token::BOOLEAN ? BOOLEAN : 
             tt == Token::STRING ? STRING : INTEGER;
    }
    bool lookupSymbol(std::string, SymbolTableIt& it);
    SymbolTable& localSymbolTable() { return mLocalSymbols[mLevel]; }

    bool typemark();
    bool variabledecl();
    bool declaration();
    bool ifstatement();
    bool loopstatement();
    bool functioncall();
    bool argumentlist(std::vector<SymbolType>&);
    bool name();
    bool factor(datatype&);
    bool term(datatype&);
    bool term2(datatype&);
    bool relation(datatype&);
    bool relation2(datatype&);
    bool arithop(datatype&);
    bool arithop2(datatype&);
    bool expression(datatype&);
    bool expression2(datatype&);
    bool destination(SymbolTableIt& it);
    bool assignmentstatement();
    bool statement();
    bool functionbody();
    bool parameterlist(std::vector<SymbolType>&);
    bool functionheader(bool global=false);
    bool functiondecl(bool global=false);

    Scanner* mScanner;
    Token mTok;
    bool mPreScanned;
    bool mError;
    int mLevel;
    long mCurrentAddr;
    // hack to allow arrays as an expression
    bool mIsArray;

    SymbolTable mGlobalSymbols;
    std::vector<SymbolTable> mLocalSymbols;
};

#endif //PARSER_H
