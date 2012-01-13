#ifndef SCANNER_H
#define SCANNER_H

class Scanner
{
  public:
    Scanner(const char* filename);
    ~Scanner();

    bool scan(std::string& tok);
    int getLineNumber() { return mLineNumber; }

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
      OTHER
    };
    
  static const charclass charclasses[128];

  private:
    FILE* mFile;
    int mLineNumber;
};

#endif //SCANNER_H
