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
  ,mTok(Token())
  ,mPreScanned(false)
  ,mErrorCount(0)
  ,mError(false)
  ,mLevel(-1)
  ,mFunctionCallCounter(0)
  ,mCurrentAddr(0)
  ,mReg(0)
  ,mArrayID(NULL)
  ,mGenFile(genfile)
  ,mLocalSymbols(0)
  ,mFunctionNames(0)
  ,mHaveReturn(0)
{}

Parser::~Parser()
{}

void Parser::initialize()
{
  vector<SymbolType> params;
  params.resize(0);

  mGlobalSymbols["getBool"] = Symbol("getBool",
                                     SymbolType(FUNCTION, BOOLEAN),
                                     mCurrentAddr++);
  mGenFile << "getBool:" << endl
           << "\tR[0] = (int)getBool();" << endl;
  calleeEnd(false);

  mGlobalSymbols["getInt"] = Symbol("getInt",
                                    SymbolType(FUNCTION, INTEGER),
                                    mCurrentAddr++);
  mGenFile << "getInt:" << endl
           << "\tR[0] = getInt();" << endl;
  calleeEnd(false);

  mGlobalSymbols["getString"] = Symbol("getString",
                                       SymbolType(FUNCTION, STRING),
                                       mCurrentAddr++);
  mGenFile << "getString:" << endl
           << "\tgetString(TMP_STRING);" << endl
           << "\tR[0] = (int)TMP_STRING;" << endl;
  calleeEnd(false);

  mGlobalSymbols["getFloat"] = Symbol("getFloat",
                                      SymbolType(FUNCTION, FLOAT),
                                      mCurrentAddr++);
  mGenFile << "getFloat:" << endl
           << "\tTMP_FLOAT1 = getFloat();" << endl
           << "\tmemcpy(&R[0], &TMP_FLOAT1, sizeof(float));" << endl;
  calleeEnd(false);

  params.resize(1);
  params[0] = SymbolType(SCALAR, BOOLEAN);
  mGlobalSymbols["putBool"] = Symbol("putBool",
                                     SymbolType(FUNCTION, INTEGER, 0, params),
                                     mCurrentAddr++);
  mReg = 1;
  mGenFile << "putBool:" << endl
           << "\tR[0] = MM[R[SP] - 1];" << endl
           << "\tR[1] = putBool((bool)R[0]);" << endl;
  calleeEnd(false);

  params[0].setDataType(INTEGER);
  mGlobalSymbols["putInt"] = Symbol("putInt",
                                    SymbolType(FUNCTION, INTEGER, 0, params),
                                    mCurrentAddr++);
  mReg = 1;
  mGenFile << "putInt:" << endl
           << "\tR[0] = MM[R[SP] - 1];" << endl
           << "\tR[1] = putInt(R[0]);" << endl;
  calleeEnd(false);

  mGlobalSymbols["sqrt"] = Symbol("sqrt",
                                  SymbolType(FUNCTION, FLOAT, 0, params),
                                  mCurrentAddr++);
  mReg = 1;
  mGenFile << "sqrt:" << endl
           << "\tR[0] = MM[R[SP] - 1];" << endl
           << "\tTMP_FLOAT1 = sqrt(R[0]);" << endl
           << "\tmemcpy(&R[1], &TMP_FLOAT1, sizeof(float));" << endl;
  calleeEnd(false);

  params[0].setDataType(STRING);
  mGlobalSymbols["putString"] = Symbol("putString",
                                       SymbolType(FUNCTION, INTEGER, 0, params),
                                       mCurrentAddr++);
  mReg = 1;
  mGenFile << "putString:" << endl
           << "\tR[0] = MM[R[SP] - 1];" << endl
           << "\tR[1] = putString((char*)R[0]);" << endl;
  calleeEnd(false);

  params[0].setDataType(FLOAT);
  mGlobalSymbols["putFloat"] = Symbol("putFloat",
                                      SymbolType(FUNCTION, INTEGER, 0, params),
                                      mCurrentAddr++);
  mReg = 1;
  mGenFile << "putFloat:" << endl
           << "\tR[0] = MM[R[SP] - 1];" << endl
           << "\tmemcpy(&TMP_FLOAT1, &R[0], sizeof(float));" << endl
           << "\tR[1] = putFloat(TMP_FLOAT1);" << endl;
  calleeEnd(false);
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

void Parser::getMemoryLocation(int fpOffset, bool global/*= false*/)
{
  mReg++;
  mGenFile << "\tR[" << mReg << "] = " << fpOffset << ";" << endl;

  if(!global)
    mGenFile << "\tR[" << mReg << "] = R[" << mReg << "] + R[FP];" << endl;

}

void Parser::doOperation(int op1, int op2, const char* op, 
    bool fp1,/*= false*/ bool fp2/*= false*/)
{
  mReg++;
  if(fp1 || fp2)
  {
    if(fp1)
      mGenFile << "\tmemcpy(&TMP_FLOAT1, &R[" << op1 << "], sizeof(float));"
               << endl;
    else
      mGenFile << "\tTMP_FLOAT1 = R[" << op1 << "];" << endl;
    if(fp2)
      mGenFile << "\tmemcpy(&TMP_FLOAT2, &R[" << op2 << "], sizeof(float));"
               << endl;
    else
      mGenFile << "\tTMP_FLOAT2 = R[" << op2 << "];" << endl;

    mGenFile << "\tTMP_FLOAT1 = TMP_FLOAT1 " << op << " TMP_FLOAT2;" << endl
             << "\tmemcpy(&R[" << mReg << "], &TMP_FLOAT1, sizeof(float));"
             << endl;
  }
  else
    mGenFile << "\tR[" << mReg << "] = R[" << op1 << "] " << op
             << " R[" << op2 << "];" << endl;
}

void Parser::throwError(string err)
{
  std::cout << "error on line " << mScanner->getLineNumber() << ": " 
            << err << std::endl;
  mError = true;
  mErrorCount++;
}

void Parser::throwWarning(string warn)
{
  std::cout << "warning: " << mScanner->getLineNumber() << ": " 
            << warn << std::endl;
}

void Parser::callerBegin(std::vector<int>& regs, const char* id)
{
  // push arguments on to stack, in reverse order
  vector<int>::reverse_iterator rit;
  for(rit = regs.rbegin(); rit != regs.rend(); ++rit)
     mGenFile << "\tR[SP] = R[SP] + 1;" << endl
              << "\tMM[R[SP]] = R[" << *rit << "];" << endl;

  int addr = mReg++;
  mGenFile << "\tR[" << mReg << "] = (int)&&" << id
           << mFunctionCallCounter << ";" << endl
           << "\tR[SP] = R[SP] + 1;" << endl
           << "\tMM[R[SP]] = R[" << mReg << "];" << endl
           << "\tgoto *(void*)R[" << addr << "];" << endl;
}

void Parser::callerEnd(std::vector<int>& regs, const char* id)
{
  mReg = 0;
  mGenFile << id << mFunctionCallCounter << ":" << endl
           << "\tR[" << mReg << "] = MM[R[SP]];" << endl
           << "\tR[SP] = R[SP] - 1;" << endl;
  mFunctionCallCounter++;

  // pop arguments off stack
  if(regs.size() > 0)
    mGenFile << "\tR[SP] = R[SP] - " << regs.size() << ";" << endl;
}

void Parser::calleeBegin()
{
  mGenFile << "\tR[SP] = R[SP] + 1;" << endl
           << "\tMM[R[SP]] = R[FP];" << endl
           << "\tR[FP] = R[SP] - 1;" << endl;
}

void Parser::calleeEnd(bool restorePointers/*=true*/)
{
  int ret = mReg;
  mReg++;
  if(restorePointers)
    mGenFile << "\tR[FP] = MM[R[SP]];" << endl
             << "\tR[SP] = R[SP] - 1;" << endl;

  mGenFile << "\tR[" << mReg << "] = MM[R[SP]];" << endl
           << "\tMM[R[SP]] = R[" << ret << "];" << endl
           << "\tgoto *(void*)R[" << mReg << "];" << endl;
  mReg = 0;
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

bool Parser::variabledecl(int addr, datatype dt, SymbolType& st,
    bool global/*= false*/)
{
  SymbolTable& table = global ? mGlobalSymbols : localSymbolTable();
  int tmpAddr = global ? mCurrentAddr++ : addr;
  if(nextTokenIs(Token::IDENTIFIER))
  {
    const char* id = mTok.getString();
    if(nextTokenIs(Token::OPENSQUARE) && nextTokenIs(Token::NUMBER))
    {
      if(getNumberType() != INTEGER)
      {
        throwError("array index must be an integer");
        return false;
      }
      int size = atoi(mTok.getString());
      if(nextTokenIs(Token::CLOSESQUARE))
      {
        st = SymbolType(ARRAY, dt, size);
        table[id] = Symbol(id, st, tmpAddr);
        return true;
      }
      stringstream m;
      m << "expected ], not " << mTok.getString();
      throwError(m.str());
      return false;
    }

    st = SymbolType(SCALAR, dt);
    table[id] = Symbol(id, st, tmpAddr);
    return true;
  }
  return false;
}

bool Parser::declaration(int addr, SymbolType& st, bool& global)
{
  if(nextTokenIs(Token::GLOBAL))
  {
    if(mLevel == 0)
      global = true;
    else
      throwWarning("global specifier only valid at top level. ignoring.");
  }

  datatype dt;
  if(typemark(dt))
    return functiondecl(addr, dt, global) || variabledecl(addr, dt, st, global);
  return false;
}

bool Parser::ifstatement()
{
  datatype dt;
  if(nextTokenIs(Token::IF) && expression(dt))
  {
    if(dt != BOOLEAN && dt != INTEGER)
    {
      throwError("conditional expressions must be convertable to boolean");
      return false;
    }
    int labelnum = mFunctionCallCounter++;
    mGenFile << "\tif(!R[" << mReg << "]) goto else" 
             << labelnum << ";" << endl;
    if(nextTokenIs(Token::THEN) &&
       statement() &&
       nextTokenIs(Token::SEMICOLON))
    {
      while(statement())
      {
        if(!nextTokenIs(Token::SEMICOLON))
        {
          throwError("expected ;");
          return false;
        }
      }
      mGenFile << "\tgoto endif" << labelnum << ";" << endl;
      if(nextTokenIs(Token::ELSE))
      {
        mGenFile << "else" << labelnum << ":" << endl;
        if(statement() && nextTokenIs(Token::SEMICOLON))
        {
          while(statement())
          {
            if(!nextTokenIs(Token::SEMICOLON))
            {
              throwError("expected ;");
              return false;
            }
          }
        }
      }
      else
        mGenFile << "else" << labelnum << ":" << endl;
      mGenFile << "endif" << labelnum << ":" << endl;

      return nextTokenIs(Token::END) && nextTokenIs(Token::IF);
    }
  }
  return false;
}

bool Parser::loopstatement()
{
  datatype dt;
  if(nextTokenIs(Token::FOR) && assignmentstatement())
  {
    int labelnum = mFunctionCallCounter++;
    mGenFile << "beginfor" << labelnum << ":" << endl;
    if(expression(dt))
    {
      mGenFile << "\tif(!R[" << mReg << "]) goto endfor" 
               << labelnum << ";" << endl;
      if(dt != BOOLEAN && dt != INTEGER)
      {
        throwError("conditional expressions must be convertable to boolean");
        return false;
      }
      while(statement())
      {
        if(!nextTokenIs(Token::SEMICOLON))
        {
          throwError("expected ;");
          return false;
        }
      }
      if(nextTokenIs(Token::END) && nextTokenIs(Token::FOR))
      {
        mGenFile << "\tgoto beginfor" << labelnum << ";" << endl
                 << "endfor" << labelnum << ":" << endl;
        return true;
      }
    }
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
    if(nextTokenIs(Token::CLOSEPAREN))
    {
      SymbolTableIt it;
      bool global;
      if(lookupSymbol(id, it, global))
      {
        vector<SymbolType> params;
        it->second.getSymbolType().getParams(params);
        if(params != args)
        {
          std::stringstream m;
          m << "argument types do match function signature for " << id;
          throwError(m.str());
          return false;
        }
        return true;
      }
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
      throwError("syntax error in argument list");
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
      throwError("array indexes must be integers");
      return false;
    }
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

     throwError("unmatched parentheses in expression");
     return false;
  }

  bool negate = nextTokenIs(Token::MINUS);
  if(nextTokenIs(Token::IDENTIFIER))
  {
    const char* id = mTok.getString();
    bool hasIndex = false;
    vector<int> regs;
    bool isFunctionCall = functioncall(regs);
    if(isFunctionCall || name(hasIndex))
    {
      int idx = mReg;
      SymbolTableIt it;
      bool global = false;
      if(!lookupSymbol(id, it, global))
      {
        stringstream m;
        m << "symbol \"" << id << "\" not found";
        throwError(m.str());
        return false;
      }
      int addr = it->second.getAddr();
      dt = it->second.getDataType();

      getMemoryLocation(addr, global);

      // if we were given an index, we need to add the offset
      if(hasIndex)
        mGenFile << "\tR[" << mReg << "] = R[" << mReg << "] "
                 << (addr < 0 ? '-' : '+') << " R[" << idx << "];" << endl;

      mReg++;
      mGenFile << "\tR[" << mReg << "] = MM[R[" << mReg - 1 << "]];" << endl;
      if(negate)
      {
        mReg++;
        mGenFile << "\tR[" << mReg << "] = -1*R[" << mReg - 1 << "];" << endl;
      }

      if(isFunctionCall)
      {
        callerBegin(regs, id);
        callerEnd(regs, id);
      }

      // hack to allow arrays as expression
      if(it->second.getStructureType() == ARRAY && !hasIndex)
        mArrayID = const_cast<char*>(id);
    }
  }
  else if(nextTokenIs(Token::NUMBER))
  {
    dt = getNumberType();
    if(dt == INTEGER)
      mGenFile << "\tR[" << ++mReg << "] = ";
    else
      mGenFile << "\tTMP_FLOAT1 = ";
    mGenFile << (negate ? "-" : "") << mTok.getString() << ";" << endl;
    if(dt == FLOAT)
      mGenFile << "\tmemcpy(&R[" << ++mReg << "], &TMP_FLOAT1, sizeof(float));"
               << endl;
  }
  else if(!negate && nextTokenIs(Token::STRING))
  {
    dt = STRING;
    mGenFile << "\tR[" << ++mReg << "] = (int)\"" << mTok.getString() 
             << "\";" << endl;
  }
  else if(!negate && (nextTokenIs(Token::TRUE) || nextTokenIs(Token::FALSE)))
  {
    dt = BOOLEAN;
    mGenFile << "\tR[" << ++mReg << "] = "
             << (mTok.getType() == Token::TRUE ? "true" : "false") << ";" 
             << endl;
  }
  else
  {
    return false;
  }
  return true;
}

bool Parser::term(datatype& dt)
{
  datatype dt1, dt2;
  if(factor(dt1))
  {
    int op1 = mReg;
    const char* op;
    if(term2(dt2, op))
    {
      if(!equivalentTypes(dt1, dt2) || mArrayID)
      {
        stringstream m;
        m << dtToString(dt1) << " type is not compatible with "
          << dtToString(dt2) << " type";
        throwError(m.str());
        return false;
      }
      int op2 = mReg;
      doOperation(op1, op2, op, dt1 == FLOAT, dt2 == FLOAT);
    }
    dt = dt1;
    return true;
  }
  return false;
}

bool Parser::term2(datatype& dt, const char*& op)
{
  datatype dt2;
  if(nextTokenIs(Token::MULTDIV))
  {
    op = mTok.getString();
    if(factor(dt))
    {
      int op1 = mReg;
      const char* thisOp;
      bool lastterm = !term2(dt2, thisOp);
      int op2 = mReg;
      if(!lastterm)
      {
        if(!arithOpCompatible(dt, dt2))
        {
          stringstream m;
          m << dtToString(dt) << " type is not compatible with "
            << dtToString(dt2) << " type for arithmetic operations";
          throwError(m.str());
          return false;
        }
        doOperation(op1, op2, thisOp, dt == FLOAT, dt2 == FLOAT);
      }
      return true;
    }
  }
  return false;
}

bool Parser::relation(datatype& dt)
{
  datatype dt1, dt2;
  if(term(dt1))
  {
    int op1 = mReg;
    const char* op;
    if(relation2(dt2, op))
    {
      if(!equivalentTypes(dt1, dt2) || mArrayID)
      {
        stringstream m;
        m << dtToString(dt1) << " type is not compatible with "
          << dtToString(dt2) << " type for conditional operations";
        throwError(m.str());
        return false;
      }
      int op2 = mReg;
      doOperation(op1, op2, op);
      dt = BOOLEAN;
    }
    else
      dt = dt1;

    return true;
  }
  return false;
}

bool Parser::relation2(datatype& dt, const char*& op)
{
  datatype dt2;
  if(nextTokenIs(Token::LTE) ||
      nextTokenIs(Token::LT) ||
      nextTokenIs(Token::GTE) ||
      nextTokenIs(Token::GT) ||
      nextTokenIs(Token::EQUAL) ||
      nextTokenIs(Token::NOTEQUAL))
  {
    op = mTok.getString();
    if(term(dt2))
    {
      int op1 = mReg;
      const char* thisOp;
      bool lastrelation = !relation2(dt2, thisOp);
      int op2 = mReg;
      if(!lastrelation)
      {
        if(!relationalOpCompatible(dt, dt2))
        {
          stringstream m;
          m << dtToString(dt) << " type is not compatible with "
            << dtToString(dt2) << " type for conditional operations";
          throwError(m.str());
          return false;
        }
        doOperation(op1, op2, thisOp);
      }
      dt = dt2;
      return true;
    }
  }

  return false;
}

bool Parser::arithop(datatype& dt)
{
  datatype dt1, dt2;
  if(relation(dt1))
  {
    int op1 = mReg;
    const char* op;
    if(arithop2(dt2, op))
    {
      if(!equivalentTypes(dt1, dt2) || mArrayID)
      {
        stringstream m;
        m << dtToString(dt1) << " type is not compatible with "
          << dtToString(dt2) << " type for arithmetic operations";
        throwError(m.str());
        return false;
      }
      int op2 = mReg;
      doOperation(op1, op2, op);
    }
    dt = dt1;
    return true;
  }
  return false;
}

bool Parser::arithop2(datatype& dt, const char*& op)
{
  datatype dt2;
  if(nextTokenIs(Token::PLUS) || nextTokenIs(Token::MINUS))
  { 
    op = mTok.getString();
    if(relation(dt))
    {
      int op1 = mReg;
      const char* thisOp;
      bool lastop = !arithop2(dt2, thisOp);
      int op2 = mReg;
      if(!lastop)
      {
        if(!arithOpCompatible(dt, dt2))
        {
          stringstream m;
          m << dtToString(dt) << " type is not compatible with "
            << dtToString(dt2) << " type for arithmetic operations";
          throwError(m.str());
          return false;
        }
        doOperation(op1, op2, op);
      }
      dt = dt2;
      return true;
    }
  }
  return false;
}

bool Parser::expression(datatype& dt)
{
  datatype dt1, dt2;
  bool haveNot = nextTokenIs(Token::NOT);
  if(arithop(dt1))
  {
    int op1 = mReg;
    const char* op;
    if(expression2(dt2, op))
    {
      if(!equivalentTypes(dt1, dt2) || mArrayID)
      {
        stringstream m;
        m << dtToString(dt) << " type is not compatible with "
          << dtToString(dt2) << " type";
        throwError(m.str());
        return false;
      }
      int op2 = mReg;
      doOperation(op1, op2, op);
    }
    dt = dt1;
    if(haveNot and dt == INTEGER)
    {
      int result = mReg++;
      mGenFile << "\tR[" << mReg << "] = ~R[" << result << "];" << endl;
    }
    return true;
  }
  return false;
}

bool Parser::expression2(datatype& dt, const char*& op)
{
  datatype dt2;
  if(nextTokenIs(Token::AND) ||
     nextTokenIs(Token::OR))
  {
    op = mTok.getString();
    if(relation(dt))
    {
      int op1 = mReg;
      const char* thisOp;
      bool lastexpression = !expression2(dt2, thisOp);
      int op2 = mReg;
      if(!lastexpression)
      {
        if(!(dt == dt2 && dt == INTEGER))
        {
          throwError("operands for bitwise operations must be integers");
          return false;
        }
        doOperation(op1, op2, thisOp);
      }
      dt = dt2;
      return true;
    }
  }
  return false;
}

bool Parser::destination(SymbolTableIt& it, int& index,  bool& global)
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
          throwError("array indexes must be integers");
          return false;
        }
        if(!nextTokenIs(Token::CLOSESQUARE))
          return false;
        index = mReg;
      }
    }
    if(!lookupSymbol(id, it, global))
    {
      stringstream m;
      m << "symbol \"" << id << "\" not found";
      throwError(m.str());
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
  bool global;
  int index = -1;
  if(destination(s, index, global) || mError)
  {
    if(!nextTokenIs(Token::ASSIGNMENT))
    {
      stringstream m;
      m << "expected \":=\" instead of " << mTok.getString();
      throwError(m.str());
      return false;
    }
    if(!expression(dt))
    {
      throwError("unrecognized expression in assignment");
      return false;
    }

    if(!equivalentTypes(s->second.getDataType(), dt))
    {
      throwError("invalid assignment; types are not compatible");
      return false;
    }

    // assignment to function means return from function
    if(!strcmp(s->second.getID(), currentFunction()))
    {
      // clean up the stack
      SymbolTableIt it = localSymbolTable().begin();
      int numLocals = 0;
      for(; it != localSymbolTable().end(); ++it)
      {
        SymbolType st = it->second.getSymbolType();
        structuretype structureType = st.getStructureType();
        // only clean up our own variables, not parameters
        // parameters have a negative offset address
        int addr = it->second.getAddr();
        if(addr > 0)
        {
          if((structureType == SCALAR || structureType == FUNCTION))
            numLocals++;
          else if(structureType == ARRAY)
            numLocals += st.getSize();
        }
      }
      if(numLocals > 0)
        mGenFile << "\tR[SP] = R[SP] - " << numLocals << ";" << endl;

      calleeEnd();
      mHaveReturn[mLevel] = true;
    }
    else
    {
      int result = mReg;
      int addr = s->second.getAddr();
      getMemoryLocation(addr, global);
      int addrReg = mReg;
      if(index != -1)
        mGenFile << "\tR[" << addrReg << "] = R[" << addrReg << "] "
                 << (addr < 0 ? '-' : '+') << " R[" << index << "];" << endl;

      mGenFile << "\tMM[R[" << addrReg << "]] = R[" << result << "];" << endl;
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

bool Parser::functionbody(const char* id)
{
  // addr relative to FP
  // 1 is stored FP
  // 2 is function address
  int addr = 3;
  SymbolType st;
  bool global = false;
  while(declaration(addr, st, global) || mError)
  {
    mError = false;
    if(!nextTokenIs(Token::SEMICOLON))
    {
      throwError("expected ;");
    }
    else if(!global)
    {
      if(st.getStructureType() == ARRAY)
        addr += st.getSize();
      else
        addr++;
    }
    global = false;
    st = SymbolType();
  }
  if(nextTokenIs(Token::BEGIN))
  {
    mGenFile << id << "_begin:" << endl;
    // add any stack variables plus the function address
    mGenFile << "\tR[SP] = R[SP] + " << addr - 2 << ";" << endl;
    mReg = 0;

    // store function addresses
    SymbolTableIt it = localSymbolTable().begin();
    for(; it != localSymbolTable().end(); ++it)
    {
      structuretype st = it->second.getStructureType();
      if(st == FUNCTION)
      {
        int addr = it->second.getAddr();
        mReg++;
        mGenFile << "\tR[" << mReg << "] = " << addr << ";" << endl
                 << "\tR[" << mReg << "] = R[" << mReg << "] + R[FP];" << endl
                 << "\tMM[R[" << mReg << "]] = (int)&&" << it->second.getID() 
                 << ";" << endl;
      }
    }

    while(statement() || mError)
    {
      mError = false;
      if(!nextTokenIs(Token::SEMICOLON))
      {
        throwError("expected ;");
        return false;
      }
    }
    if(nextTokenIs(Token::END) && nextTokenIs(Token::FUNCTION))
    {
      if(!mHaveReturn[mLevel])
      {
        stringstream m;
        m << "expected return from " << id << " function";
        throwError(m.str());
        return false;
      }
      mHaveReturn.pop_back();
      mFunctionNames.pop_back();
      return true;
    }
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
      throwError("syntax error in parameter list");
      return false;
    }
    return true;
  }
  return false;
}

bool Parser::functionheader(int addr, datatype dt, const char*& id,
    bool global/* = false*/)
{
  bool isFunctionHeader = nextTokenIs(Token::FUNCTION) &&
                          nextTokenIs(Token::IDENTIFIER);

  id = mTok.getString();

  isFunctionHeader = isFunctionHeader && nextTokenIs(Token::OPENPAREN);
  if(isFunctionHeader)
  {
    vector<SymbolType> params;
    parameterlist(params);
    if(nextTokenIs(Token::CLOSEPAREN))
    {
      SymbolTable& st = global ? mGlobalSymbols : localSymbolTable();
      int firstaddr = global ? mCurrentAddr++ : 2;

      st[id] = Symbol(id,
                      SymbolType(FUNCTION, dt, 0, params),
                      firstaddr);

      if(mLevel != 0 && !global)
        mLocalSymbols[mLevel - 1][id] = 
          Symbol(id, SymbolType(FUNCTION, dt, 0, params), addr);

      mFunctionNames.push_back(const_cast<char*>(id));
      mHaveReturn.push_back(false);
      mGenFile << id << ":" << endl;
      calleeBegin();
      mGenFile << "\tgoto " << id << "_begin;" << endl;
    }
  }
  return isFunctionHeader;
}

bool Parser::functiondecl(int addr, datatype dt, bool global/* = false*/)
{
  mLevel++;
  mLocalSymbols.push_back(SymbolTable());
  const char* id;
  bool isFunctionDecl = functionheader(addr, dt, id, global) && 
                        functionbody(id);
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
  bool parseSuccess = typemark(dt) && functiondecl(0, dt, false);

  mGenFile << "_main:" << endl;

  for(SymbolTableIt it(mGlobalSymbols.begin()); it!=mGlobalSymbols.end(); ++it)
  {
    if(it->second.getStructureType() == FUNCTION)
      mGenFile << "\tR[0] = " << it->second.getAddr() << ";" << endl
               << "\tMM[R[0]] = (int)&&" << it->second.getID() << ";" 
               << endl;
  }

  mGenFile << "\tR[SP] = STACK_START;" << endl
           << "\tR[FP] = STACK_START;" << endl
           << "\tMM[R[SP]] = (int)&&_end;" << endl
           << "\tgoto main;" << endl
           << "_end:" << endl
           << "\treturn MM[R[SP]];" << endl
           << "}" << endl;

  mGenFile.close();

  return parseSuccess && mErrorCount == 0;
}
