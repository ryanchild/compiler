#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include "Parser.h"
#include "Scanner.h"
#include "Token.h"

Parser::Parser(Scanner* s)
  :mScanner(s)
  ,mPreScanned(false)
  ,mError(false)
  ,mLevel(0)
{}

void Parser::initialize()
{
  int addr = 0;
  std::vector<datatype> params;
  params.resize(0);
  Symbol s;
  s.st = FUNCTION;
  s.address = addr++;
  mGlobalSymbols[getSignature("getBool", BOOLEAN, params)] = s;
  s.address = addr++;
  mGlobalSymbols[getSignature("getInt", INTEGER, params)] = s;
  s.address = addr++;
  mGlobalSymbols[getSignature("getString", STRING, params)] = s;
  s.address = addr++;
  mGlobalSymbols[getSignature("getFloat", FLOAT, params)] = s;

  params.resize(1);
  params[0] = BOOLEAN;
  s.address = addr++;
  mGlobalSymbols[getSignature("putBool", INTEGER, params)] = s;
  params[0] = INTEGER;
  s.address = addr++;
  mGlobalSymbols[getSignature("putInt", INTEGER, params)] = s;
  s.address = addr++;
  mGlobalSymbols[getSignature("sqrt", INTEGER, params)] = s;
  params[0] = STRING;
  s.address = addr++;
  mGlobalSymbols[getSignature("putString", INTEGER, params)] = s;
  params[0] = FLOAT;
  s.address = addr++;
  mGlobalSymbols[getSignature("putFloat", INTEGER, params)] = s;
}

char Parser::getDataType(datatype t)
{
  return t == INTEGER ? 'i' :
         t == FLOAT ? 'f' :
         t == BOOLEAN ? 'b' :
         t == STRING ? 's' : '?';
}

std::string Parser::getSignature(const char* name, datatype ret, 
    std::vector<datatype>& params)
{
  std::ostringstream oss;
  oss << "_" << getDataType(ret) << params.size();

  std::vector<datatype>::iterator it;
  for(it=params.begin(); it!= params.end(); ++it)
    oss << getDataType(*it);

  oss << name;
  return oss.str();
}

const char* Parser::getNameFromSignature(const char* sig)
{
  return strstr(sig, "_") + 1;
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

bool Parser::typemark()
{
  return nextTokenIs(Token::INTEGER) ||
         nextTokenIs(Token::FLOAT) ||
         nextTokenIs(Token::BOOLEAN) ||
         nextTokenIs(Token::STRING);
}

bool Parser::variabledecl()
{
  Token::tokentype tt = mTok.getType();
  datatype dt = tt == Token::INTEGER ? INTEGER :
                tt == Token::FLOAT ? FLOAT :
                tt == Token::BOOLEAN ? BOOLEAN : 
                tt == Token::STRING ? STRING : INTEGER;
                
  if(nextTokenIs(Token::IDENTIFIER))
  {
    if(nextTokenIs(Token::OPENSQUARE) && nextTokenIs(Token::NUMBER))
    {
      int size = atoi(mTok.getString());
      if(nextTokenIs(Token::CLOSESQUARE))
      {
        // add array to symbol table
      }
      else
        return false;
    }
    // add type to symbol table
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

  if(typemark())
    return functiondecl(global) || variabledecl();
  return false;
}

bool Parser::ifstatement()
{
  datatype dt;
  if(nextTokenIs(Token::IF) &&
     expression(dt) &&
     nextTokenIs(Token::THEN) &&
     statement())
  {
    while(statement());
    if(nextTokenIs(Token::ELSE) && statement())
      while(statement());

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
    while(statement());
    return nextTokenIs(Token::END) && nextTokenIs(Token::FOR);
  }
  return false;
}

bool Parser::functioncall()
{
  return nextTokenIs(Token::OPENPAREN) &&
         argumentlist() && 
         nextTokenIs(Token::CLOSEPAREN);
}

bool Parser::argumentlist()
{
  datatype dt;
  if(expression(dt) && nextTokenIs(Token::COMMA))
      argumentlist();
  return true;
}

bool Parser::name()
{
  datatype dt;
  if(nextTokenIs(Token::OPENSQUARE) &&
     expression(dt) && 
     nextTokenIs(Token::CLOSESQUARE))
    return true;

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
    if(functioncall())
    {
      // look up function name in symbol table and get return type
      // dt = 
    }
    else if(name())
    {
      // look up variable in symbol table and get type
      // dt = 
    }

  if(nextTokenIs(Token::MINUS))
  {
    //TODO: do something with minus
    if(nextTokenIs(Token::IDENTIFIER))
    {
      const char* id = mTok.getString();
      if(name())
      {
        // look up variable in symbol table and get type
        // dt = 
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

bool Parser::term(datatype dt)
{
  datatype dt1, dt2;
  if(factor(dt1) && term2(dt2))
  {
    if(dt1 == dt2)
    {
      dt = dt1;
      return true;
    }
    else
    {
      //TODO: report type check error
      return false;
    }
  }
  return false;
}

bool Parser::term2()
{
  if(nextTokenIs(Token::MULTDIV) && factor())
    term2();
  return true;
}

bool Parser::relation()
{
  return term() && relation2();
}

bool Parser::relation2()
{
  if(nextTokenIs(Token::LTE) && term() ||
     nextTokenIs(Token::GTE) && term() ||
     nextTokenIs(Token::LT) && term() ||
     nextTokenIs(Token::GT) && term() ||
     nextTokenIs(Token::EQUAL) && term() ||
     nextTokenIs(Token::NOTEQUAL) && term())
    relation2();

  return true;
}

bool Parser::arithop()
{
  return relation() && arithop2();
}

bool Parser::arithop2()
{
  if(nextTokenIs(Token::PLUS) && relation() ||
     nextTokenIs(Token::MINUS) && relation())
    arithop2();

  return !mError;
}

bool Parser::expression(datatype& dt)
{
  //TODO: do something with not
  return (nextTokenIs(Token::NOT) && arithop() && expression2()) ||
          arithop() && expression2();
}

bool Parser::expression2()
{
  if(nextTokenIs(Token::AND) && relation() ||
     nextTokenIs(Token::OR) && relation())
    expression2();
  return true;
}

bool Parser::destination()
{
  datatype dt;
  if(nextTokenIs(Token::IDENTIFIER))
  {
    if(nextTokenIs(Token::OPENSQUARE))
      return expression(dt) && nextTokenIs(Token::CLOSESQUARE);
    return true;
  }
  return false;
}

bool Parser::assignmentstatement()
{
  datatype dt;
  return destination() &&
         nextTokenIs(Token::ASSIGNMENT) &&
         expression(dt);
}

bool Parser::statement()
{
  return assignmentstatement() ||
         ifstatement() ||
         loopstatement();
}

bool Parser::functionbody()
{
  while(declaration());
  if(nextTokenIs(Token::BEGIN))
  {
    while(statement());
    return nextTokenIs(Token::END) &&
           nextTokenIs(Token::FUNCTION);
  }
  return false;
}

bool Parser::parameterlist(std::vector<datatype>& params)
{
  if(typemark() && variabledecl() && nextTokenIs(Token::COMMA))
      parameterlist(params);
  return true;
}

bool Parser::functionheader(bool global/* = false*/)
{
  bool isFunctionHeader = nextTokenIs(Token::FUNCTION) &&
                          nextTokenIs(Token::IDENTIFIER) &&
                          nextTokenIs(Token::OPENPAREN);
  if(isFunctionHeader)
  {
    std::vector<datatype> params;
    isFunctionHeader = parameterlist(params) &&
                       nextTokenIs(Token::CLOSEPAREN);
    if(isFunctionHeader)
    {
      // do something with parameters
    }
  }
  return isFunctionHeader;
}

bool Parser::functiondecl(bool global/* = false*/)
{
  mLevel++;
  bool isFunctionDecl = functionheader(global) && functionbody();
  mLevel--;
  return isFunctionDecl;
}

bool Parser::parse()
{
  return typemark() && functiondecl(true);
}
