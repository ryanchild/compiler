#ifndef TOKEN_H
#define TOKEN_H

#include <string>

class Token
{
  public:
    Token();

    enum tokentype
    {
      IDENTIFIER,
      ADDSUBTRACT,
      MULTDIV,
      EQUAL,
      NOTEQUAL,
      LT,
      GT,
      STRING,
      INTEGER,
      FLOAT,
      BOOLEAN,
      IF,
      THEN,
      ELSE,
      FOR,
      END,
      GLOBAL,
      FUNCTION,
      AND,
      OR,
      NOT,
      ASSIGNMENT,
      OPENSQUARE,
      CLOSESQUARE,
      OPENPAREN,
      CLOSEPAREN,
      QUOTE
    };

    const char* getString() const { return mString; }
    void setString(const char* s) { mString = s; }
    tokentype getType() const { return mType; }
    void setType(tokentype t) { mType = t; }

  private:
    tokentype mType;
    const char* mString;
};

std::ostream& operator<<(std::ostream& out, Token& t);

#endif //TOKEN_H