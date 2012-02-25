#ifndef PARSER_H
#define PARSER_H

#include "Token.h"
#include <map>
#include <vector>
#include <fstream>

class Scanner;

class Parser
{
  public:
    Parser(Scanner*, const char*);
    ~Parser();
    bool parse();
    void initialize();
    inline void callerBegin(std::vector<int>&, const char*);
    inline void callerEnd(std::vector<int>&, const char*);
    inline void calleeBegin();
    inline void calleeEnd(bool restorePointers=true);

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
          ,mSize(1)
        {}

        SymbolType(structuretype st,
                   datatype dt)
          :mSt(st)
          ,mDt(dt)
          ,mSize(1)
          ,mParams(0)
        {}

        SymbolType(structuretype st,
                   datatype dt,
                   int size)
          :mSt(st)
          ,mDt(dt)
          ,mSize(size)
          ,mParams(0)
        {}

        SymbolType(structuretype st,
                   datatype dt,
                   int size,
                   std::vector<SymbolType>& params)
          :mSt(st)
          ,mDt(dt)
          ,mSize(size)
          ,mParams(params)
          {}

        void setDataType(datatype dt) { mDt = dt; }
        datatype getDataType() const { return mDt; }
        structuretype getStructureType() const { return mSt; }
        int getSize() const { return mSize; }
        void getParams(std::vector<SymbolType>& p) { p = mParams; }

        bool operator==(const SymbolType& rhs) const
        {
          return rhs.mSt == mSt &&
                 rhs.mDt == mDt &&
                 rhs.mSize == mSize;
        }

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
        SymbolType getSymbolType() const { return mSt; }
        long getAddr() const { return mAddr; }
        const char* getID() const { return mId; }

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
      // float or int
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
    bool equivalentTypes(datatype dt1, datatype dt2)
    {
      return dt1 == dt2 || arithOpCompatible(dt1, dt2);
    }
    bool lookupSymbol(std::string, SymbolTableIt& it, bool& global);
    SymbolTable& localSymbolTable() { return mLocalSymbols[mLevel]; }
    char* currentFunction() { return mFunctionNames[mLevel]; }
    // after returning, mReg is the register with the desired address
    void getMemoryLocation(int, bool global=false);
    void doOperation(int, int, const char* op, bool fp1 = false,
       bool fp2 = false);

    bool typemark(datatype&);
    bool variabledecl(int, datatype, SymbolType&, bool global=false);
    bool declaration(int, SymbolType&, bool& global);
    bool ifstatement();
    bool loopstatement();
    bool functioncall(std::vector<int>&);
    bool argumentlist(std::vector<SymbolType>&, std::vector<int>&);
    bool name(bool&);
    bool factor(datatype&);
    bool term(datatype&);
    bool term2(datatype&, const char*&);
    bool relation(datatype&);
    bool relation2(datatype&, const char*&);
    bool arithop(datatype&);
    bool arithop2(datatype&, const char*&);
    bool expression(datatype&);
    bool expression2(datatype&, const char*&);
    bool destination(SymbolTableIt& it, int& index, bool& global);
    bool assignmentstatement();
    bool statement();
    bool functionbody(const char*);
    bool parameterlist(std::vector<SymbolType>&, int addr=-1);
    bool functionheader(int addr, datatype dt, const char*&, bool global=false);
    bool functiondecl(int addr, datatype dt, bool global=false);

    Scanner* mScanner;
    Token mTok;
    bool mPreScanned;
    bool mError;
    int mLevel;
    int mFunctionCallCounter;
    long mCurrentAddr;
    int mReg;
    // hack to allow arrays as an expression
    char* mArrayID;
    std::ofstream mGenFile;

    SymbolTable mGlobalSymbols;
    std::vector<SymbolTable> mLocalSymbols;
    std::vector<char*> mFunctionNames;
};

#endif //PARSER_H
