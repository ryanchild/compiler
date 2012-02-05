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
    {
      return nextToken().getType() == Token::NUMBER &&
             nextToken().getType() == Token::CLOSESQUARE;
    }
    else
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

bool Parser::statement()
{
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
