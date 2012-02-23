#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include "Parser.h"
#include "Scanner.h"
#include "Token.h"

using namespace std;

Parser::Parser(Scanner* s, const char* genfile)
  :mScanner(s)
  ,mPreScanned(false)
  ,mError(false)
  ,mLevel(-1)
  ,mFunctionCallCounter(0)
  ,mCurrentAddr(0)
  ,mReg(0)
  ,mIsArray(false)
  ,mGenFile(genfile)
  ,mCurrentFunction(NULL)
  ,mLocalSymbols(0)
{}

Parser::~Parser()
{
  mGenFile.close();
}

void Parser::initialize()
{
  vector<SymbolType> params;
  params.resize(0);

  mGlobalSymbols["getBool"] = Symbol("getBool",
                                     SymbolType(FUNCTION, BOOLEAN),
                                     mCurrentAddr++);
  mGenFile << "getBool:" << endl
           << "\tR[0] = (size_t)getBool();" << endl
           << "\tR[1] = MM[R[SP]];" << endl
           << "\tMM[R[SP]] = R[0];" << endl
           << "\tgoto *R[1];" << endl << endl;

  mGlobalSymbols["getInt"] = Symbol("getInt",
                                    SymbolType(FUNCTION, INTEGER),
                                    mCurrentAddr++);
  mGenFile << "getInt:" << endl
           << "\tR[0] = getInt();" << endl
           << "\tR[SP] = R[SP] - 1;" << endl
           << "\tR[1] = MM[R[SP]];" << endl
           << "\tMM[R[SP]] = R[0];" << endl
           << "\tgoto *R[1];" << endl << endl;

  mGlobalSymbols["getString"] = Symbol("getString",
                                       SymbolType(FUNCTION, STRING),
                                       mCurrentAddr++);
  mGenFile << "getString:" << endl
           << "\tgetString(TMP_STRING);" << endl
           << "\tR[1] = MM[R[SP]];" << endl
           << "\tMM[R[SP]] = (size_t)TMP_STRING;" << endl
           << "\tgoto *R[1];" << endl << endl;

  mGlobalSymbols["getFloat"] = Symbol("getFloat",
                                      SymbolType(FUNCTION, FLOAT),
                                      mCurrentAddr++);
  mGenFile << "getFloat:" << endl
           << "\tTMP_FLOAT = getFloat();" << endl
           << "\tR[1] = MM[R[SP]];" << endl
           << "\tmemcpy(&MM[R[SP]], &TMP_FLOAT, sizeof(float));" << endl
           << "\tgoto *R[1];" << endl << endl;

  params.resize(1);
  params[0] = SymbolType(SCALAR, BOOLEAN);
  mGlobalSymbols["putBool"] = Symbol("putBool",
                                     SymbolType(FUNCTION, INTEGER, 0, params),
                                     mCurrentAddr++);
  mGenFile << "putBool:" << endl
           << "\tR[0] = MM[R[SP] - 1];" << endl
           << "\tR[1] = MM[R[SP]];" << endl
           << "\tR[2] = putBool((bool)R[0]);" << endl
           << "\tMM[R[SP]] = R[2];" << endl
           << "\tgoto *R[1];" << endl << endl;

  params[0].setDataType(INTEGER);
  mGlobalSymbols["putInt"] = Symbol("putInt",
                                    SymbolType(FUNCTION, INTEGER, 0, params),
                                    mCurrentAddr++);
  mGenFile << "putInt:" << endl
           << "\tR[0] = MM[R[SP] - 1];" << endl
           << "\tR[0] = putInt(R[0]);" << endl
           << "\tR[1] = MM[R[SP]];" << endl
           << "\tMM[R[SP]] = R[0];" << endl
           << "\tgoto *R[1];" << endl << endl;

  mGlobalSymbols["sqrt"] = Symbol("sqrt",
                                  SymbolType(FUNCTION, FLOAT, 0, params),
                                  mCurrentAddr++);
  mGenFile << "sqrt:" << endl
           << "\tR[0] = MM[R[SP] - 1];" << endl
           << "\tTMP_FLOAT = sqrt(R[0]);" << endl
           << "\tR[1] = MM[R[SP]];" << endl
           << "\tmemcpy(&MM[R[SP]], &TMP_FLOAT, sizeof(float));" << endl
           << "\tgoto *R[1];" << endl << endl;

  params[0].setDataType(STRING);
  mGlobalSymbols["putString"] = Symbol("putString",
                                       SymbolType(FUNCTION, INTEGER, 0, params),
                                       mCurrentAddr++);
  mGenFile << "putString:" << endl
           << "\tR[0] = MM[R[SP] - 1];" << endl
           << "\tR[0] = putString((char*)R[0]);" << endl
           << "\tR[1] = MM[R[SP]];" << endl
           << "\tMM[R[SP]] = R[0];" << endl
           << "\tgoto *R[1];" << endl << endl;

  params[0].setDataType(FLOAT);
  mGlobalSymbols["putFloat"] = Symbol("putFloat",
                                      SymbolType(FUNCTION, INTEGER, 0, params),
                                      mCurrentAddr++);
  mGenFile << "putFloat:" << endl
           << "\tmemcpy(&TMP_FLOAT, &MM[R[SP] - 1], sizeof(float));" << endl
           << "\tR[0] = putFloat(TMP_FLOAT);" << endl
           << "\tR[1] = MM[R[SP]];" << endl
           << "\tMM[R[SP]] = R[0];" << endl
           << "\tgoto *R[1];" << endl << endl;
}

Token Parser::nextToken()
{
  if(mPreScanned)
    mPreScanned = false;
  else
    mScanner->scan(mTok);
  return mTok;
}

bool Parser::nextTokenIs(Token::tokentype tt)
{
  if(nextToken().getType() == tt)
    return true;
  mPreScanned = true;
  return false;
}

bool Parser::lookupSymbol(string id, SymbolTableIt& it, bool& global)
{
  global = true;
  it = localSymbolTable().find(id);
  if(it == localSymbolTable().end())
    it = mGlobalSymbols.find(id);
  else 
  {
    global = false;
    return true;
  }
  return (it != mGlobalSymbols.end());
}

void Parser::getMemoryLocation(int spOffset, bool hasIndex)
{
  mReg++;
  mGenFile << "\tR[" << mReg << "] = " << spOffset << ";" << endl 
           << "\tR[" << mReg << "] = R[" << mReg << "] + R[SP];" << endl;
  if(hasIndex)
    mGenFile << "\tR[" << mReg << "] = R["
             << mReg << "] + R[" << mReg - 1 << "];" << endl;
}

bool Parser::typemark(datatype& dt)
{
  if(nextTokenIs(Token::INTEGER))
    dt = INTEGER;
  else if(nextTokenIs(Token::FLOAT))
    dt = FLOAT;
  else if(nextTokenIs(Token::BOOLEAN))
    dt = BOOLEAN;
  else if(nextTokenIs(Token::STRING))
    dt = STRING;
  else
    return false;
  return true;
}

bool Parser::variabledecl(int addr, datatype dt, SymbolType& st)
{
  if(nextTokenIs(Token::IDENTIFIER))
  {
    const char* id = mTok.getString();
    if(nextTokenIs(Token::OPENSQUARE) && nextTokenIs(Token::NUMBER))
    {
      if(getNumberType() != INTEGER)
      {
        //TODO: throw type error
        return false;
      }
      int size = atoi(mTok.getString());
      if(nextTokenIs(Token::CLOSESQUARE))
      {
        st = SymbolType(ARRAY, dt, size);
        localSymbolTable()[id] = Symbol(id, st, addr);
        return true;
      }
      return false;
    }

    st = SymbolType(SCALAR, dt);
    localSymbolTable()[id] = Symbol(id, st, addr);
    return true;
  }
  return false;
}

bool Parser::declaration(int addr, SymbolType& st)
{
  bool global = false;
  if(nextTokenIs(Token::GLOBAL))
  {
    if(mLevel == 1)
      global = true;
    else
    {
      //TODO: issue warning
    }
  }

  datatype dt;
  if(typemark(dt))
    return functiondecl(addr, dt, global) || variabledecl(addr, dt, st);
  return false;
}

bool Parser::ifstatement()
{
  datatype dt;
  if(nextTokenIs(Token::IF) &&
     expression(dt) &&
     nextTokenIs(Token::THEN) &&
     statement() &&
     nextTokenIs(Token::SEMICOLON))
  {
    if(dt != BOOLEAN && dt != INTEGER)
    {
      //TODO: throw type check error
      return false;
    }
    while(statement())
    {
      if(!nextTokenIs(Token::SEMICOLON))
        return false;
    }
    if(nextTokenIs(Token::ELSE) && statement() && nextTokenIs(Token::SEMICOLON))
    {
      while(statement())
      {
        if(!nextTokenIs(Token::SEMICOLON))
          return false;
      }
    }

    return nextTokenIs(Token::END) && nextTokenIs(Token::IF);
  }
  return false;
}

bool Parser::loopstatement()
{
  datatype dt;
  if(nextTokenIs(Token::FOR) &&
     assignmentstatement() &&
     expression(dt))
  {
    if(dt != BOOLEAN && dt != INTEGER)
    {
      //TODO: throw type check error
      return false;
    }
    while(statement())
    {
      if(!nextTokenIs(Token::SEMICOLON))
        return false;
    }
    return nextTokenIs(Token::END) && nextTokenIs(Token::FOR);
  }
  return false;
}

bool Parser::functioncall()
{
  const char* id = mTok.getString();
  vector<SymbolType> args;
  if(nextTokenIs(Token::OPENPAREN))
  {
    argumentlist(args);
    return !mError && nextTokenIs(Token::CLOSEPAREN);
  }
  return false;
}

bool Parser::argumentlist(vector<SymbolType>& args)
{
  datatype dt;
  mIsArray = false;
  if(expression(dt))
  {
    args.push_back(SymbolType(mIsArray ? ARRAY : SCALAR, dt));
    if(nextTokenIs(Token::COMMA) && !argumentlist(args))
    {
      //TODO: error reporting
      mError = true;
      return false;
    }
    return true;
  }
  return false;
}

bool Parser::name(bool& hasIndex)
{
  datatype dt;
  if(nextTokenIs(Token::OPENSQUARE) &&
     expression(dt))
  {
    if(dt != BOOLEAN && dt != INTEGER)
    {
      //TODO: throw type check error
      return false;
    }
    mGenFile << "R[" << ++mReg << "] = R[" << mReg - 1 << "];" << endl;
    hasIndex = true;

    if(nextTokenIs(Token::CLOSESQUARE))
      return true;
  }

  // already checked identifier
  return true;
}

bool Parser::factor(datatype& dt)
{
  if(nextTokenIs(Token::OPENPAREN))
  {
     if(expression(dt) && nextTokenIs(Token::CLOSEPAREN))
       return true;

     mError = true;
     return false;
  }

  if(nextTokenIs(Token::IDENTIFIER))
  {
    const char* id = mTok.getString();
    bool hasIndex = false;
    bool isFunctionCall = functioncall();
    if(isFunctionCall || name(hasIndex))
    {
      SymbolTableIt it;
      bool global = false;
      if(!lookupSymbol(id, it, global))
      {
        //TODO: throw symbol not found error
        return false;
      }
      dt = it->second.getDataType();

      if(global)
      {
        mReg++;
        mGenFile << "\tR[" << mReg << "] = " << it->second.getAddr() << ";"
                 << endl;
      }
      else
        getMemoryLocation(it->second.getAddr(), hasIndex);

      mReg++;
      mGenFile << "\tR[" << mReg << "] = MM[R[" << mReg - 1 << "]];"
               << endl;

      if(isFunctionCall)
      {
        mReg++;
        mGenFile << "\tR[" << mReg << "] = (size_t)&&" << id
                 << mFunctionCallCounter << ";" << endl
                 << "\tMM[R[SP]] = R[" << mReg << "];" << endl
                 << "\tR[SP] = R[SP] + 1;" << endl
                 << "\tgoto *(void*)R[" << mReg - 1 << "];" << endl
                 << id << mFunctionCallCounter << ":" << endl;
        mReg++;
        mGenFile << "\tR[" << mReg << "] = MM[R[SP]];" << endl;
        mFunctionCallCounter++;
      }

      // hack to allow arrays as expression
      if(it->second.getStructureType() == ARRAY && !hasIndex)
        mIsArray = true;

      return true;
    }
  }

  if(nextTokenIs(Token::MINUS))
  {
    //TODO: do something with minus
    if(nextTokenIs(Token::IDENTIFIER))
    {
      const char* id = mTok.getString();
      bool hasIndex;
      if(name(hasIndex))
      {
        SymbolTableIt it;
        bool global;
        if(!lookupSymbol(id, it, global))
        {
          //TODO: throw symbol not found error
          return false;
        }
        dt = it->second.getDataType();
        if(it->second.getStructureType() == ARRAY && !hasIndex)
        {
          //TODO: error, need index here
          return false;
        }
        getMemoryLocation(it->second.getAddr(), hasIndex);
        mReg++;
        mGenFile << "\tR[" << mReg << "] = MM[R[" << mReg - 1 << "]];"
                 << endl;
      }
    }
    else if(nextTokenIs(Token::NUMBER))
    {
      dt = getNumberType();
      if(dt == INTEGER)
        mGenFile << "\tR[" << ++mReg << "] = ";
      else
        mGenFile << "\tTMP_FLOAT = ";
      mGenFile << mTok.getString() << ";" << endl;
      if(dt == FLOAT)
        mGenFile << "\tmemcpy(&R[" << ++mReg << "], &TMP_FLOAT, sizeof(float));"
                 << endl;
    }
    else
      return false;

    mGenFile << "\tR[" << ++mReg << "] = 0;" << endl
             << "\tR[" << ++mReg << "] = R[" << mReg - 1
             << "] - R[" << mReg - 2 << "];" << endl;
    return true;
  }

  if(nextTokenIs(Token::NUMBER))
  {
    dt = getNumberType();
    if(dt == INTEGER)
      mGenFile << "\tR[" << ++mReg << "] = ";
    else
      mGenFile << "\tTMP_FLOAT = ";
    mGenFile << mTok.getString() << ";" << endl;
    if(dt == FLOAT)
      mGenFile << "\tmemcpy(&R[" << ++mReg << "], &TMP_FLOAT, sizeof(float));"
               << endl;
  }
  else if(nextTokenIs(Token::STRING))
  {
    dt = STRING;
    mGenFile << "\tR[" << ++mReg << "] = " << mTok.getString() << ";" << endl;
  }
  else if(nextTokenIs(Token::TRUE) || nextTokenIs(Token::FALSE))
  {
    dt = BOOLEAN;
    mGenFile << "\tR[" << ++mReg << "] = "
             << (mTok.getType() == Token::TRUE ? "true" : "false") << ";" 
             << endl;
  }
  else
    return false;
  return true;
}

bool Parser::term(datatype& dt)
{
  datatype dt1, dt2;
  if(factor(dt1))
  {
    if(term2(dt2) && (dt1 != dt2 || mIsArray))
    {
      //TODO: report type check error
      mError = true;
      return false;
    }
    dt = dt1;
    return true;
  }
  return false;
}

bool Parser::term2(datatype& dt)
{
  datatype dt2;
  if(nextTokenIs(Token::MULTDIV) && factor(dt))
  {
    bool lastterm = term2(dt2);
    if(!lastterm && !arithOpCompatible(dt, dt2))
    {
      //TODO: report type check error
      mError = true;
      return false;
    }
    return true;
  }

  return false;
}

bool Parser::relation(datatype& dt)
{
  datatype dt1, dt2;
  if(term(dt1))
  {
    if(relation2(dt2) && (dt1 != dt2 || mIsArray))
    {
      //TODO: report type check error
      return false;
    }
    dt = dt1;
    return true;
  }
  return false;
}

bool Parser::relation2(datatype& dt)
{
  datatype dt2;
  if((nextTokenIs(Token::LTE) ||
      nextTokenIs(Token::LT) ||
      nextTokenIs(Token::GTE) ||
      nextTokenIs(Token::GT) ||
      nextTokenIs(Token::EQUAL) ||
      nextTokenIs(Token::NOTEQUAL)) && term(dt))
  {
    bool lastrelation = relation2(dt2);
    if(!lastrelation && !relationalOpCompatible(dt, dt2))
    {
      //TODO: report type check error
      mError = true;
      return false;
    }
    return true;
  }

  return false;
}

bool Parser::arithop(datatype& dt)
{
  datatype dt1, dt2;
  if(relation(dt1))
  {
    if(arithop2(dt2) && (dt1 != dt2 || mIsArray))
    {
      //TODO: report type check error
      mError = true;
      return false;
    }
    dt = dt1;
    return true;
  }
  return false;
}

bool Parser::arithop2(datatype& dt)
{
  datatype dt2;
  if((nextTokenIs(Token::PLUS) ||
      nextTokenIs(Token::MINUS)) && relation(dt))
  {
    bool lastop = arithop2(dt2);
    if(!lastop && !arithOpCompatible(dt, dt2))
    {
      //TODO: report type check error
      mError = true;
      return false;
    }
    return true;
  }

  return false;
}

bool Parser::expression(datatype& dt)
{
  datatype dt1, dt2;
  //TODO: do something with not
  bool haveNot = nextTokenIs(Token::NOT);
  if(arithop(dt1))
  {
    if(expression2(dt2) && (dt1 != dt2 || mIsArray))
    {
      //TODO: report type check error
      mError = true;
      return false;
    }
    dt = dt1;
    return !mError;
  }
  return false;
}

bool Parser::expression2(datatype& dt)
{
  datatype dt2;
  if((nextTokenIs(Token::AND) ||
      nextTokenIs(Token::OR)) && relation(dt))
  {
    bool lastexpression = expression2(dt2);
    if(!lastexpression && !bitwiseOpCompatible(dt, dt2))
    {
      //TODO: report type check error
      return false;
    }
    return true;
  }

  return false;
}

bool Parser::destination(SymbolTableIt& it)
{
  datatype dt;
  if(nextTokenIs(Token::IDENTIFIER))
  {
    const char* id = mTok.getString();
    if(nextTokenIs(Token::OPENSQUARE))
    {
      if(expression(dt))
      {
        if(dt != INTEGER)
        {
          //TODO: report type error
          return false;
        }
        if(!nextTokenIs(Token::CLOSESQUARE))
          return false;
      }
    }
    bool global;
    if(!lookupSymbol(id, it, global))
    {
      //TODO: error symbol not found
      return false;
    }
    return true;
  }
  return false;
}

bool Parser::assignmentstatement()
{
  datatype dt;
  SymbolTableIt s;
  if(destination(s) && nextTokenIs(Token::ASSIGNMENT) && expression(dt))
  {
    if(s->second.getDataType() != dt)
    {
      //TODO: throw type error
      return false;
    }
    int result = mReg;
    if(!strcmp(s->second.getID(), mCurrentFunction))
    {
      mGenFile << "\tR[SP] = R[SP] - 1;" << endl;
      mReg++;
      mGenFile << "\tR[" << mReg << "] = MM[R[SP]];" << endl
               << "\tMM[R[SP]] = R[" << result << "];" << endl
               << "\tgoto *(void*)R[" << mReg << "];" << endl << endl;
    }
    else
    {
      getMemoryLocation(s->second.getAddr(), false);
      int destAddr = mReg;
      mGenFile << "\tMM[R[" << destAddr << "]] = R[" << result << "];" << endl;
    }

    return true;
  }
  return false;
}

bool Parser::statement()
{
  return assignmentstatement() ||
         ifstatement() ||
         loopstatement();
}

bool Parser::functionbody()
{
  int addr = 0;
  SymbolType st;
  while(declaration(addr, st))
  {
    if(!nextTokenIs(Token::SEMICOLON))
      return false;
    else
    {
      if(st.getStructureType() == ARRAY)
        addr += st.getSize();
      else
        addr++;
    }
  }
  if(nextTokenIs(Token::BEGIN))
  {
    mReg = 0;
    while(statement())
    {
      if(!nextTokenIs(Token::SEMICOLON))
        return false;
    }
    return nextTokenIs(Token::END) &&
           nextTokenIs(Token::FUNCTION);
  }
  return false;
}

bool Parser::parameterlist(vector<SymbolType>& params, int addr/*=-1*/)
{
  datatype dt;
  SymbolType st;
  if(typemark(dt) && variabledecl(addr, dt, st))
  {
    if(st.getStructureType() == ARRAY)
      addr -= st.getSize();
    else
      addr--;

    params.push_back(st);
    if(nextTokenIs(Token::COMMA) && !parameterlist(params,addr))
    {
      //TODO: error reporting
      mError = true;
      return false;
    }
    return true;
  }
  return false;
}

bool Parser::functionheader(int addr, datatype dt, bool global/* = false*/)
{
  bool isFunctionHeader = nextTokenIs(Token::FUNCTION) &&
                          nextTokenIs(Token::IDENTIFIER);

  const char* id = mTok.getString();

  isFunctionHeader = isFunctionHeader && nextTokenIs(Token::OPENPAREN);
  if(isFunctionHeader)
  {
    vector<SymbolType> params;
    parameterlist(params);
    if(nextTokenIs(Token::CLOSEPAREN))
    {
      SymbolTable& st = global ? mGlobalSymbols : localSymbolTable();
      addr = global ? mCurrentAddr++ : 0;
      st[id] = Symbol(id,
                      SymbolType(FUNCTION, dt, 0, params),
                      0);

      mCurrentFunction = const_cast<char*>(id);
      mGenFile << id << ":" << endl;
    }
  }
  return isFunctionHeader;
}

bool Parser::functiondecl(int addr, datatype dt, bool global/* = false*/)
{
  mLevel++;
  mLocalSymbols.push_back(SymbolTable());
  bool isFunctionDecl = functionheader(addr, dt, global) && functionbody();
  mLocalSymbols.pop_back();
  mLevel--;
  return isFunctionDecl;
}

bool Parser::parse()
{
  mGenFile << "#include \"stdbool.h\"" << endl
           << "#include \"math.h\"" << endl
           << "#include \"string.h\"" << endl
           << "#include \"stdio.h\"" << endl
           << "#include \"runtime.h\"" << endl
           << endl
           << "int main()" << endl
           << "{" << endl
           << "\tgoto _main;" << endl
           << endl;

  initialize();

  datatype dt;
  bool success = typemark(dt) && functiondecl(0, dt, false);

  mGenFile << "_main:" << endl;

  for(SymbolTableIt it(mGlobalSymbols.begin()); it!=mGlobalSymbols.end(); ++it)
  {
    if(it->second.getStructureType() == FUNCTION)
      mGenFile << "\tR[0] = " << it->second.getAddr() << ";" << endl
               << "\tMM[R[0]] = (size_t)&&" << it->second.getID() << ";" 
               << endl;
  }

  mGenFile << "\tR[SP] = 1024;" << endl
           << "\tMM[R[SP]] = (size_t)&&_end;" << endl
           << "\tR[SP] = R[SP] + 1;" << endl
           << "\tgoto " << mCurrentFunction << ";" << endl
           << "_end:" << endl
           << "\tprintf(\"the program exited with value %d\\n\",MM[R[SP]]);"<< endl
           << "\treturn 0;" << endl
           << "}" << endl;

  return success;
}
