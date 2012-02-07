#include <iostream>
#include "Parser.h"
#include "Scanner.h"
#include "Token.h"

Parser::Parser(Scanner* s)
  :mScanner(s)
  ,mPreScanned(false)
  ,mError(false)
{}

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
  if(nextTokenIs(Token::IDENTIFIER))
  {
    if(nextTokenIs(Token::OPENSQUARE))
      return nextTokenIs(Token::NUMBER) &&
             nextTokenIs(Token::CLOSESQUARE);
    return true;
  }
  return false;
}

bool Parser::declaration(bool toplevel/* = false*/)
{
  if(nextTokenIs(Token::GLOBAL))
  {
    if(toplevel)
    {
      //TODO: deal with global
    }
    else
    {
      //TODO: issue warning
    }
  }

  if(typemark())
    return functiondecl() || variabledecl();
  return false;
}

bool Parser::ifstatement()
{
  if(nextTokenIs(Token::IF) &&
     expression() &&
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
  if(nextTokenIs(Token::FOR) &&
     assignmentstatement() &&
     expression())
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
  if(expression() && nextTokenIs(Token::COMMA))
      argumentlist();
  return true;
}

bool Parser::name()
{
  if(nextTokenIs(Token::OPENSQUARE) &&
     expression() && 
     nextTokenIs(Token::CLOSESQUARE))
    return true;

  // already checked identifier
  return true;
}

bool Parser::factor()
{
  if(nextTokenIs(Token::OPENPAREN))
  {
     if(expression() && nextTokenIs(Token::CLOSEPAREN))
       return true;

     mError = true;
     return false;
  }

  if(nextTokenIs(Token::IDENTIFIER))
    return functioncall() || name();

  if(nextTokenIs(Token::MINUS))
  {
    //TODO: do something with minus
    return ((nextTokenIs(Token::IDENTIFIER) && name()) ||
            nextTokenIs(Token::NUMBER));
  }

  return nextTokenIs(Token::NUMBER) ||
         nextTokenIs(Token::STRING) ||
         nextTokenIs(Token::TRUE) ||
         nextTokenIs(Token::FALSE) ||
         nextTokenIs(Token::NUMBER);
}

bool Parser::term()
{
  return factor() && term2();
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

bool Parser::expression()
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
  if(nextTokenIs(Token::IDENTIFIER))
  {
    if(nextTokenIs(Token::OPENSQUARE))
      return expression() && nextTokenIs(Token::CLOSESQUARE);
    return true;
  }
  return false;
}

bool Parser::assignmentstatement()
{
  return destination() &&
         nextTokenIs(Token::ASSIGNMENT) &&
         expression();
}

bool Parser::statement()
{
  return assignmentstatement() ||
         ifstatement() ||
         loopstatement();
}

bool Parser::functionbody(bool toplevel/* = false*/)
{
  while(declaration(toplevel));
  if(nextTokenIs(Token::BEGIN))
  {
    while(statement());
    return nextTokenIs(Token::END) &&
           nextTokenIs(Token::FUNCTION);
  }
  return false;
}

bool Parser::parameterlist()
{
  if(typemark() && variabledecl() && nextTokenIs(Token::COMMA))
      parameterlist();
  return true;
}

bool Parser::functionheader()
{
  return nextTokenIs(Token::FUNCTION) &&
         nextTokenIs(Token::IDENTIFIER) &&
         nextTokenIs(Token::OPENPAREN) &&
         parameterlist() &&
         nextTokenIs(Token::CLOSEPAREN);
}

bool Parser::functiondecl(bool toplevel/* = false*/)
{
  return functionheader() && functionbody(toplevel);
}

bool Parser::parse()
{
  return typemark() && functiondecl(true);
}
