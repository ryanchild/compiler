#ifndef SCANNER_H
#define SCANNER_H

#include <map>
#include <vector>
#include "Token.h"

class Scanner
{
  public:
    Scanner(const char* filename);
    ~Scanner();

    bool scan(Token& tok);
    int getLineNumber() { return mLineNumber; }
    int numErrors() { return mErrors.size(); }

    void printErrors();

    enum charclass
    {
      DIGIT,
      LOWER,
      UPPER,
      UNDERSCORE,
      SEMICOLON,
      FORWARDSLASH,
      ASTERISK,
      PLUS,
      MINUS,
      LT,
      GT,
      SINGLEQUOTE,
      DOUBLEQUOTE,
      EQUALS,
      WHITESPACE,
      BRACKET,
      EXCLAMATION,
      OTHER
    };
    
  static const charclass charclasses[128];

  private:

    struct error
    {
      int lineNum;
      std::string msg;
    };

    FILE* mFile;
    int mLineNumber;
    std::map<std::string, Token::tokentype> mStrings;
    std::vector<error> mErrors;

    typedef std::map<std::string, Token::tokentype>::iterator StringIt;
};

#endif //SCANNER_H
