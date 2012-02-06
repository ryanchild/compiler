#include <iostream>
#include "Parser.h"
#include "Scanner.h"
#include "Token.h"

Parser::Parser(Scanner* s)
  :mScanner(s)
  ,mPreScanned(false)
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

bool Parser::declaration()
{
  if(nextToken().getType() != Token::GLOBAL)
    setPreScanned();
  else
  {
    //TODO: deal with global
  }

  if(typemark())
  {
    if(functiondecl())
      return true;

    setPreScanned();
    if(variabledecl())
      return true;
  }
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
    else
      setPreScanned();

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
  if(nextToken().getType() == Token::OPENPAREN &&
     argumentlist() && 
     nextToken().getType() == Token::CLOSEPAREN)
    return true;

  setPreScanned();
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
  if(nextToken().getType() == Token::OPENSQUARE &&
     expression() && 
     nextToken().getType() == Token::CLOSESQUARE)
    return true;

  setPreScanned();
  return true;
}

bool Parser::factor()
{
  if(nextToken().getType() == Token::OPENPAREN &&
     expression() &&
     nextToken().getType() == Token::CLOSEPAREN)
    return true;

  setPreScanned();
  if(nextToken().getType() == Token::NUMBER)
    return true;

  setPreScanned();
  if(nextToken().getType() == Token::STRING)
    return true;

  setPreScanned();
  if(nextToken().getType() == Token::TRUE)
    return true;

  setPreScanned();
  if(nextToken().getType() == Token::FALSE)
    return true;

  setPreScanned();
  if(identifier())
  {
    if(functioncall())
      return true;

    setPreScanned();
    if(name())
      return true;
  }

  setPreScanned();
  if(nextToken().getType() == Token::NUMBER)
    return true;

  setPreScanned();
  if(nextToken().getType() == Token::MINUS)
  {
    //TODO: do something with minus
    if(identifier() && name())
      return true;

    setPreScanned();
    if(nextToken().getType() == Token::NUMBER)
      return true;

    setPreScanned();
    return false;
  }

  setPreScanned();
  return false;
}

bool Parser::term()
{
  return factor() && term2();
}

bool Parser::term2()
{
  if(nextToken().getType() == Token::MULTDIV &&
     factor())
    term2();
  setPreScanned();
  return true;
}

bool Parser::relation()
{
  return term() && relation2();
}

bool Parser::relation2()
{
  if(nextToken().getType() == Token::LTE && term())
    relation2();

  setPreScanned();
  if(nextToken().getType() == Token::GTE && term())
    relation2();

  setPreScanned();
  if(nextToken().getType() == Token::LT && term())
    relation2();

  setPreScanned();
  if(nextToken().getType() == Token::GT && term())
    relation2();

  setPreScanned();
  if(nextToken().getType() == Token::EQUAL && term())
    relation2();

  setPreScanned();
  if(nextToken().getType() == Token::NOTEQUAL && term())
    relation2();

  setPreScanned();
  return true;
}

bool Parser::arithop()
{
  return relation() && arithop2();
}

bool Parser::arithop2()
{
  if(nextToken().getType() == Token::PLUS && relation())
    arithop2();

  setPreScanned();
  if(nextToken().getType() == Token::MINUS && relation())
    arithop2();

  setPreScanned();
  return true;
}

bool Parser::expression()
{
  //TODO: do something with not
  if(nextToken().getType() == Token::NOT &&
     arithop() &&
     expression2())
    return true;

  setPreScanned();
  if(arithop() && expression2())
    return true;

  setPreScanned();
  return false;
}

bool Parser::expression2()
{
  if(nextToken().getType() == Token::AND && relation())
    expression2();

  setPreScanned();
  if(nextToken().getType() == Token::OR && relation())
    expression2();

  setPreScanned();
  return true;
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
         nextToken().getType() == Token::ASSIGNMENT &&
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
