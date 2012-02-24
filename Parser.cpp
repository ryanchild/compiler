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
  ,mArrayID(NULL)
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
           << "\tR[0] = (int)getBool();" << endl
           << "\tR[SP] = R[SP] - 1;" << endl
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
           << "\tR[SP] = R[SP] - 1;" << endl
           << "\tR[1] = MM[R[SP]];" << endl
           << "\tMM[R[SP]] = (int)TMP_STRING;" << endl
           << "\tgoto *R[1];" << endl << endl;

  mGlobalSymbols["getFloat"] = Symbol("getFloat",
                                      SymbolType(FUNCTION, FLOAT),
                                      mCurrentAddr++);
  mGenFile << "getFloat:" << endl
           << "\tTMP_FLOAT = getFloat();" << endl
           << "\tR[SP] = R[SP] - 1;" << endl
           << "\tR[1] = MM[R[SP]];" << endl
           << "\tmemcpy(&MM[R[SP]], &TMP_FLOAT, sizeof(float));" << endl
           << "\tgoto *R[1];" << endl << endl;

  params.resize(1);
  params[0] = SymbolType(SCALAR, BOOLEAN);
  mGlobalSymbols["putBool"] = Symbol("putBool",
                                     SymbolType(FUNCTION, INTEGER, 0, params),
                                     mCurrentAddr++);
  mGenFile << "putBool:" << endl
           << "\tR[SP] = R[SP] - 1;" << endl
           << "\tR[0] = MM[R[SP] - 1];" << endl
           << "\tR[1] = putBool((bool)R[0]);" << endl
           << "\tR[0] = MM[R[SP]];" << endl
           << "\tMM[R[SP]] = R[1];" << endl
           << "\tgoto *R[0];" << endl << endl;

  params[0].setDataType(INTEGER);
  mGlobalSymbols["putInt"] = Symbol("putInt",
                                    SymbolType(FUNCTION, INTEGER, 0, params),
                                    mCurrentAddr++);
  mGenFile << "putInt:" << endl
           << "\tR[SP] = R[SP] - 1;" << endl
           << "\tR[0] = MM[R[SP] - 1];" << endl
           << "\tR[1] = putInt(R[0]);" << endl
           << "\tR[0] = MM[R[SP]];" << endl
           << "\tMM[R[SP]] = R[1];" << endl
           << "\tgoto *R[0];" << endl << endl;

  mGlobalSymbols["sqrt"] = Symbol("sqrt",
                                  SymbolType(FUNCTION, FLOAT, 0, params),
                                  mCurrentAddr++);
  mGenFile << "sqrt:" << endl
           << "\tR[SP] = R[SP] - 1;" << endl
           << "\tR[0] = MM[R[SP] - 1];" << endl
           << "\tTMP_FLOAT = sqrt(R[0]);" << endl
           << "\tmemcpy(&R[1], &TMP_FLOAT, sizeof(float));" << endl
           << "\tR[0] = MM[R[SP]];" << endl
           << "\tMM[R[SP]] = R[1];" << endl
           << "\tgoto *R[0];" << endl << endl;

  params[0].setDataType(STRING);
  mGlobalSymbols["putString"] = Symbol("putString",
                                       SymbolType(FUNCTION, INTEGER, 0, params),
                                       mCurrentAddr++);
  mGenFile << "putString:" << endl
           << "\tR[SP] = R[SP] - 1;" << endl
           << "\tR[0] = MM[R[SP] - 1];" << endl
           << "\tR[1] = putString((char*)R[0]);" << endl
           << "\tR[0] = MM[R[SP]];" << endl
           << "\tMM[R[SP]] = R[1];" << endl
           << "\tgoto *R[0];" << endl << endl;

  params[0].setDataType(FLOAT);
  mGlobalSymbols["putFloat"] = Symbol("putFloat",
                                      SymbolType(FUNCTION, INTEGER, 0, params),
                                      mCurrentAddr++);
  mGenFile << "putFloat:" << endl
           << "\tR[SP] = R[SP] - 1;" << endl
           << "\tR[0] = MM[R[SP] - 1];" << endl
           << "\tmemcpy(&TMP_FLOAT, &R[0], sizeof(float));" << endl
           << "\tR[1] = putFloat(TMP_FLOAT);" << endl
           << "\tR[0] = MM[R[SP]];" << endl
           << "\tMM[R[SP]] = R[1];" << endl
           << "\tgoto *R[0];" << endl << endl;
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

bool Parser::functioncall(vector<int>& regs)
{
  const char* id = mTok.getString();
  vector<SymbolType> args;
  if(nextTokenIs(Token::OPENPAREN))
  {
    argumentlist(args, regs);
    if(!mError && nextTokenIs(Token::CLOSEPAREN))
    {
      SymbolTableIt it;
      bool global;
      lookupSymbol(id, it, global);
      vector<SymbolType> params;
      it->second.getSymbolType().getParams(params);
      if(params != args)
      {
        //TOOD: report type check error
        return false;
      }
      return true;
    }
  }
  return false;
}

bool Parser::argumentlist(vector<SymbolType>& args, vector<int>& regs)
{
  datatype dt;
  mArrayID = NULL;
  if(expression(dt))
  {
    if(mArrayID)
    {
      SymbolTableIt it;
      bool global;
      lookupSymbol(mArrayID, it, global);
      SymbolType st = it->second.getSymbolType();
      args.push_back(st);
      int size = st.getSize();
      // mReg - 1 has address of first element, from factor()
      int addr = mReg - 1;
      regs.push_back(mReg);
      for(int i = 1; i < size; i++)
      {
        mReg++;
        mGenFile << "\tR[" << addr << "] = R[" << addr << "] + 1;" << endl
                 << "\tR[" << mReg << "] = MM[R[" << addr << "]];" << endl;
        regs.push_back(mReg);
      }
    }
    else
    {
      args.push_back(SymbolType(SCALAR, dt));
      regs.push_back(mReg);
    }

    if(nextTokenIs(Token::COMMA) && !argumentlist(args, regs))
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
    vector<int> regs;
    bool isFunctionCall = functioncall(regs);
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
        // push arguments on to stack, in reverse order
        vector<int>::reverse_iterator rit;
        for(rit = regs.rbegin(); rit != regs.rend(); ++rit)
        {
          mGenFile << "\tMM[R[SP]] = R[" << *rit << "];" << endl
                   << "\tR[SP] = R[SP] + 1;" << endl;
        }
        mReg++;
        mGenFile << "\tR[" << mReg << "] = (int)&&" << id
                 << mFunctionCallCounter << ";" << endl
                 << "\tMM[R[SP]] = R[" << mReg << "];" << endl
                 << "\tR[SP] = R[SP] + 1;" << endl
                 << "\tgoto *(void*)R[" << mReg - 1 << "];" << endl
                 << id << mFunctionCallCounter << ":" << endl;
        mReg++;
        mGenFile << "\tR[" << mReg << "] = MM[R[SP]];" << endl;
        mFunctionCallCounter++;

        // pop arguments off stack
        mGenFile << "\tR[SP] = R[SP] - " << regs.size() << ";" << endl;
      }

      // hack to allow arrays as expression
      if(it->second.getStructureType() == ARRAY && !hasIndex)
        mArrayID = const_cast<char*>(id);

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
    mGenFile << "\tR[" << ++mReg << "] = (int)\"" << mTok.getString() 
             << "\";" << endl;
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
    if(term2(dt2) && (dt1 != dt2 || mArrayID))
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
    if(relation2(dt2) && (dt1 != dt2 || mArrayID))
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
    if(arithop2(dt2) && (dt1 != dt2 || mArrayID))
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
    if(expression2(dt2) && (dt1 != dt2 || mArrayID))
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
               << "\tMM[R[0]] = (int)&&" << it->second.getID() << ";" 
               << endl;
  }

  mGenFile << "\tR[SP] = 1024;" << endl
           << "\tMM[R[SP]] = (int)&&_end;" << endl
           << "\tR[SP] = R[SP] + 1;" << endl
           << "\tgoto " << mCurrentFunction << ";" << endl
           << "_end:" << endl
           << "\treturn MM[R[SP]];" << endl
           << "}" << endl;

  return success;
}
