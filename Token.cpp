#include <iostream>

#include "Token.h"

Token::Token()
  :mString(NULL)
{}

std::ostream& operator<<(std::ostream& out, Token& t)
{
  out << t.getString();
  return out;
}
