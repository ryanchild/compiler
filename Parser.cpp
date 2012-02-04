#include <iostream>
#include "Parser.h"
#include "Scanner.h"
#include "Token.h"

Parser::Parser(Scanner* s)
  :mScanner(s)
{}

bool Parser::identifier(bool decl/* = false*/)
{
  mScanner->scan(mTok);
  Token::tokentype t = mTok.getType();
  if(t == Token::OPENSQUARE)
  {
    if(decl)
    {
      // declaration
      mScanner->scan(mTok);
      Token::tokentype t = mTok.getType();

      if(t == Token::NUMBER)
      {
        mScanner->scan(mTok);
        Token::tokentype t = mTok.getType();

        if(t == Token::CLOSESQUARE)
        {
          // TODO: add to symbol table
          return true;
        }
      }
    }
    else
    {
      // name, destination
    }
  }
  else
    return false;
}

bool Parser::global()
{
  mScanner->scan(mTok);
  switch(mTok.getType())
  {
    case Token::INTEGER:
    case Token::FLOAT:
    case Token::BOOLEAN:
    case Token::STRING:
      datatype(true);
      break;
    default:
      return false;
      break;
  }
}

bool Parser::datatype(bool global/* = false*/)
{
  mScanner->scan(mTok);
  switch(mTok.getType())
  {
    case Token::FUNCTION:
      if(!global && function())
        return true;
      break;
    case Token::IDENTIFIER:
      if(identifier())
        return true;
      break;
    default:
      break;
  }
  return false;
}

bool Parser::function()
{
  while(mTok.getType() != Token::END)
  {
    getNext();
  }
  mScanner->scan(mTok);
  if(mTok.getType() != Token::FUNCTION)
  {
    return false;
  }
  return true;
}

bool Parser::getNext()
{
  if(mScanner->scan(mTok))
  {
    switch(mTok.getType())
    {
      case Token::INTEGER:
      case Token::FLOAT:
      case Token::BOOLEAN:
      case Token::STRING:
        datatype();
        break;
      case Token::GLOBAL:
        global();
        break;
      case Token::FUNCTION:
        break;
      default:
        break;
    }

    return true;
  }
  return false;
}

void Parser::parse()
{
  while(getNext())
    std::cout << "got token: " << mTok.getString() << std::endl;
}
