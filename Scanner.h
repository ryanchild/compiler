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

    bool initialize();
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
      FORWARDSLASH,
      ASTERISK,
      PLUS,
      MINUS,
      LT,
      GT,
      DOUBLEQUOTE,
      EQUALS,
      WHITESPACE,
      BRACKET,
      EXCLAMATION,
      LOGICALOP,
      COLON,
      SEMICOLON,
      COMMA,
      OTHER
    };
    
  static const charclass charclasses[128];

  private:

    struct error
    {
      int lineNum;
      std::string msg;
    };

    void addIllegalCharacterError(char c);

    FILE* mFile;
    std::string mFilename;
    int mLineNumber;
    std::map<std::string, Token::tokentype> mStrings;
    std::vector<error> mErrors;

    typedef std::map<std::string, Token::tokentype>::iterator StringIt;
};

#endif //SCANNER_H
