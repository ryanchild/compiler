#include <iostream>
#include "Parser.h"
#include "Scanner.h"
#include "Token.h"

Parser::Parser(Scanner* s)
  :mScanner(s)
{}

Token Parser::nextToken()
{
  if(mPreScanned)
    mPreScanned = false;
  else
    mScanner->scan(mTok);
  return mTok;
}

bool Parser::typemark()
{
  Token::tokentype tt = nextToken().getType();
  return tt == Token::INTEGER ||
         tt == Token::FLOAT ||
         tt == Token::BOOLEAN ||
         tt == Token::STRING;
}

bool Parser::variabledecl()
{
  if(identifier())
  {
    if(nextToken().getType() == Token::OPENSQUARE)
      return nextToken().getType() == Token::NUMBER &&
             nextToken().getType() == Token::CLOSESQUARE;
    setPreScanned();
    return true;
  }
  return false;
}

bool Parser::global()
{
  return nextToken().getType() == Token::GLOBAL;
}

bool Parser::declaration()
{
  if(!global())
    setPreScanned();

  if(!typemark())
    setPreScanned();

  if(functiondecl())
    return true;

  setPreScanned();
  if(variabledecl())
    return true;
  
  setPreScanned();
  return false;
}

bool Parser::ifstatement()
{
  if(nextToken().getType() == Token::IF &&
     expression() &&
     nextToken().getType() == Token::THEN &&
     statement())
  {
    while(statement());
    if(nextToken().getType() == Token::ELSE &&
       statement())
    {
      while(statement());
    }
    if(nextToken().getType() == Token::END &&
       nextToken().getType() == Token::IF)
      return true;
  }
  return false;
}

bool Parser::loopstatement()
{
  if(nextToken().getType() == Token::FOR &&
     assignmentstatement() &&
     expression())
  {
    while(statement());
    if(nextToken().getType() == Token::END &&
       nextToken().getType() == Token::FOR)
      return true;
  }
  return false;
}

bool Parser::functioncall()
{
  if(identifier())
  {
    if(nextToken().getType() == Token::OPENPAREN)
      return argumentlist() && 
             nextToken().getType() == Token::CLOSEPAREN;
    setPreScanned();
    return true;
  }
  return false;

}

bool Parser::argumentlist()
{
  if(expression() && nextToken().getType() == Token::COMMA) 
      argumentlist();
  setPreScanned();
  return true;
}

bool Parser::name()
{
  if(identifier())
  {
    if(nextToken().getType() == Token::OPENSQUARE)
      return expression() && 
             nextToken().getType() == Token::CLOSESQUARE;
    setPreScanned();
    return true;
  }
  return false;
}

bool Parser::factor()
{
  if(nextToken().getType() == Token::OPENPAREN &&
     expression() &&
     nextToken().getType() == Token::CLOSEPAREN)
    return true;

  setPreScanned();
  if(functioncall())
    return true;

  setPreScanned();
  bool minus = false;
  if(nextToken().getType() == Token::MINUS)
  {
    minus = true;
    //TODO: do something with minus
    if(name())
      return true;

    setPreScanned();
    if(nextToken().getType() == Token::NUMBER)
      return true;

    setPreScanned();
    return false;
  }

  setPreScanned();
  if(nextToken().getType() == Token::STRING)
    return true;

  setPreScanned();
  if(nextToken().getType() == Token::TRUE)
    return true;

  setPreScanned();
  if(nextToken().getType() == Token::FALSE)
    return true;
}

bool Parser::term()
{
}

bool Parser::relation()
{
}

bool Parser::arithop()
{
}

bool Parser::expression()
{
}

bool Parser::destination()
{
  if(identifier())
  {
    if(nextToken().getType() == Token::OPENSQUARE)
      return expression() && nextToken().getType() == Token::CLOSESQUARE;
    setPreScanned();
    return true;
  }
  return false;
}

bool Parser::assignmentstatement()
{
  return destination() &&
         nextToken().getType() == Token::COLON &&
         nextToken().getType() == Token::EQUALS &&
         expression();
}

bool Parser::statement()
{
  if(assignmentstatement())
    return true;

  setPreScanned();
  if(ifstatement())
    return true;

  setPreScanned();
  if(loopstatement())
    return true;

  setPreScanned();
  return false;
}

bool Parser::functionbody()
{
  while(declaration());
  if(nextToken().getType() == Token::BEGIN)
  {
    while(statement());
    return nextToken().getType() == Token::END &&
           nextToken().getType() == Token::FUNCTION;
  }
  return false;
}

bool Parser::parameterlist()
{
  if(typemark() && variabledecl() && nextToken().getType() == Token::COMMA) 
      parameterlist();
  setPreScanned();
  return true;
}

bool Parser::identifier()
{
  return nextToken().getType() == Token::IDENTIFIER;
}

bool Parser::functionheader()
{
  return nextToken().getType() == Token::FUNCTION &&
         identifier() &&
         nextToken().getType() == Token::OPENPAREN &&
         parameterlist() &&
         nextToken().getType() == Token::CLOSEPAREN;
}

bool Parser::functiondecl()
{
  return typemark() && functionheader() && functionbody();
}

bool Parser::parse()
{
  return functiondecl();
}
