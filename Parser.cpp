#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include "Parser.h"
#include "Scanner.h"
#include "Token.h"

Parser::Parser(Scanner* s, const char* genfile)
  :mScanner(s)
  ,mPreScanned(false)
  ,mError(false)
  ,mLevel(-1)
  ,mCurrentAddr(0)
  ,mIsArray(false)
  ,mGenFile(genfile)
  ,mLocalSymbols(0)
{}

void Parser::initialize()
{
  initializeFileGen();
  initializeSymbolTable();
}

void Parser::initializeFileGen()
{
  mGenFile << "include \"runtime.h\"" << std::endl;

}

void Parser::initializeSymbolTable()
{
  std::vector<SymbolType> params;
  params.resize(0);
  mGlobalSymbols["getBool"] = Symbol("getBool",
                                     SymbolType(FUNCTION, BOOLEAN),
                                     mCurrentAddr++);
  mGlobalSymbols["getInt"] = Symbol("getInt",
                                    SymbolType(FUNCTION, INTEGER),
                                    mCurrentAddr++);
  mGlobalSymbols["getString"] = Symbol("getString",
                                       SymbolType(FUNCTION, STRING),
                                       mCurrentAddr++);
  mGlobalSymbols["getFloat"] = Symbol("getFloat",
                                      SymbolType(FUNCTION, FLOAT),
                                      mCurrentAddr++);

  params.resize(1);
  params[0] = SymbolType(SCALAR, BOOLEAN);
  mGlobalSymbols["putBool"] = Symbol("putBool",
                                     SymbolType(FUNCTION, INTEGER, 0, params),
                                     mCurrentAddr++);
  params[0].setDataType(INTEGER);
  mGlobalSymbols["putInt"] = Symbol("putInt",
                                    SymbolType(FUNCTION, INTEGER, 0, params),
                                    mCurrentAddr++);
  mGlobalSymbols["sqrt"] = Symbol("sqrt",
                                  SymbolType(FUNCTION, FLOAT, 0, params),
                                  mCurrentAddr++);
  params[0].setDataType(STRING);
  mGlobalSymbols["putString"] = Symbol("putString",
                                       SymbolType(FUNCTION, INTEGER, 0, params),
                                       mCurrentAddr++);
  params[0].setDataType(FLOAT);
  mGlobalSymbols["putFloat"] = Symbol("putFloat",
                                      SymbolType(FUNCTION, INTEGER, 0, params),
                                      mCurrentAddr++);
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

bool Parser::lookupSymbol(std::string id, SymbolTableIt& it)
{
  it = localSymbolTable().find(id);
  if(it == localSymbolTable().end())
    it = mGlobalSymbols.find(id);
  else 
    return true;
  return (it != mGlobalSymbols.end());
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

bool Parser::variabledecl(datatype dt, SymbolType& st)
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
        localSymbolTable()[id] = Symbol(id, st, mCurrentAddr++);
        return true;
      }
      return false;
    }

    st = SymbolType(SCALAR, dt);
    localSymbolTable()[id] = Symbol(id, st, mCurrentAddr++);
    return true;
  }
  return false;
}

bool Parser::declaration()
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
  SymbolType st;
  if(typemark(dt))
    return functiondecl(dt, global) || variabledecl(dt, st);
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
  std::vector<SymbolType> args;
  if(nextTokenIs(Token::OPENPAREN))
  {
    argumentlist(args);
    return !mError && nextTokenIs(Token::CLOSEPAREN);
  }
  return false;
}

bool Parser::argumentlist(std::vector<SymbolType>& args)
{
  datatype dt;
  mIsArray = false;
  if(expression(dt))
  {
    args.push_back(SymbolType(mIsArray ? ARRAY : SCALAR, dt));
    if(nextTokenIs(Token::COMMA))
    {
      if(!argumentlist(args))
      {
        mError = true;
        return false;
      }
    return true;
    }
  }
  return false;
}

bool Parser::name()
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
    if(functioncall() || name())
    {
      SymbolTableIt it;
      if(!lookupSymbol(id, it))
      {
        //TODO: throw symbol not found error
        return false;
      }
      dt = it->second.getDataType();

      // hack to allow arrays as expression
      if(it->second.getStructureType() == ARRAY)
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
      if(name())
      {
        SymbolTableIt it;
        if(!lookupSymbol(id, it))
        {
          //TODO: throw symbol not found error
          return false;
        }
        dt = it->second.getDataType();
      }
    }
    else if(nextTokenIs(Token::NUMBER))
      dt = getNumberType();
    else
      return false;
    return true;
  }

  if(nextTokenIs(Token::NUMBER))
    dt = getNumberType();
  else if(nextTokenIs(Token::STRING))
    dt = STRING;
  else if(nextTokenIs(Token::TRUE) || nextTokenIs(Token::FALSE))
    dt = BOOLEAN;
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
    if(!lookupSymbol(id, it))
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
  while(declaration())
  {
    if(!nextTokenIs(Token::SEMICOLON))
      return false;
  }
  if(nextTokenIs(Token::BEGIN))
  {
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

bool Parser::parameterlist(std::vector<SymbolType>& params)
{
  datatype dt;
  SymbolType st;
  if(typemark(dt) && variabledecl(dt, st))
  {
    params.push_back(st);
    if(nextTokenIs(Token::COMMA))
      return parameterlist(params);
    return true;
  }
  return false;
}

bool Parser::functionheader(datatype dt, bool global/* = false*/)
{
  bool isFunctionHeader = nextTokenIs(Token::FUNCTION) &&
                          nextTokenIs(Token::IDENTIFIER);

  const char* id = mTok.getString();

  isFunctionHeader = isFunctionHeader && nextTokenIs(Token::OPENPAREN);
  if(isFunctionHeader)
  {
    std::vector<SymbolType> params;
    isFunctionHeader = parameterlist(params) &&
                       nextTokenIs(Token::CLOSEPAREN);
    if(isFunctionHeader)
    {
      SymbolTable& st = global ? mGlobalSymbols : localSymbolTable();
      st[id] = Symbol(id,
                      SymbolType(FUNCTION, dt, 0, params),
                      mCurrentAddr++);
    }
  }
  return isFunctionHeader;
}

bool Parser::functiondecl(datatype dt, bool global/* = false*/)
{
  mLevel++;
  mLocalSymbols.push_back(SymbolTable());
  bool isFunctionDecl = functionheader(dt, global) && functionbody();
  mLocalSymbols.pop_back();
  mLevel--;
  return isFunctionDecl;
}

bool Parser::parse()
{
  mGenFile << "include \"runtime.h\"" << std::endl
           << std::endl
           << "int main()" << std::endl
           << "{" << std::endl
           << "SP = 1024;" << std::endl;

  datatype dt;
  bool success = typemark(dt) && functiondecl(dt, true);

  mGenFile << "}" << std::endl;
  return success;
}
