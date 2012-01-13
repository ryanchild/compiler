#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>

#include "Scanner.h"

using namespace std;

Scanner::Scanner(const char* filename)
  :mFile(NULL)
  ,mLineNumber(1)
{
  mFile = fopen(filename, "r");
}

Scanner::~Scanner()
{
  fclose(mFile);
}

bool Scanner::scan(string& tok)
{
  int c = 0;
  charclass cc;

  while(true)
  {
    c = fgetc(mFile);
    cc = charclasses[c];
    if(c == EOF)
      return false;
    else if(cc == WHITESPACE)
    {
      while(cc == WHITESPACE)
      {
        if(c == '\n')
          mLineNumber++;
        c = fgetc(mFile);
        cc = charclasses[c];
      }
      ungetc(c,mFile);
    }
    else
      break;
  }

  ostringstream oss;
  switch(cc)
  {
    case FORWARDSLASH:
      c = fgetc(mFile);
      if(c == '/')
      {
        while(fgetc(mFile) != '\n');
        mLineNumber++;
      }
      else
      {
        ungetc(c,mFile);
        tok = '/';
      }
      break;

    case EQUALS:
    case MINUS:
    case PLUS:
      tok = (char)c;
      break;

    case DIGIT: 
      while(cc == DIGIT)
      {
        oss << (char)c;
        c = fgetc(mFile);
        cc = charclasses[c];
      }
      ungetc(c,mFile);
      tok = oss.str();
      break;

    case LOWER:
    case UPPER:
      while(cc == LOWER || cc == UPPER || cc == UNDERSCORE || cc == DIGIT)
      {
        oss << (char)c;
        c = fgetc(mFile);
        cc = charclasses[c];
      }
      ungetc(c,mFile);
      tok = oss.str();
      break;

    case OTHER:
    default: break;
  }

  return true;
}

const Scanner::charclass Scanner::charclasses[128] = {
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  WHITESPACE,   // \n
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  WHITESPACE,   // <SPACE>
  OTHER,
  DOUBLEQUOTE,  // "
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  SINGLEQUOTE,  // '
  OTHER,
  OTHER,
  ASTERISK,     // *
  PLUS,         // +
  OTHER,
  MINUS,        // -
  OTHER,
  FORWARDSLASH, // /
  DIGIT,        // 0
  DIGIT,        // 1
  DIGIT,        // 2
  DIGIT,        // 3
  DIGIT,        // 4
  DIGIT,        // 5
  DIGIT,        // 6
  DIGIT,        // 7
  DIGIT,        // 8
  DIGIT,        // 9
  OTHER,
  SEMICOLON,    // ;
  LT,           // <
  EQUALS,       // =
  GT,           // >
  OTHER,
  OTHER,
  UPPER,        // A
  UPPER,        // B
  UPPER,        // C
  UPPER,        // D
  UPPER,        // E
  UPPER,        // F
  UPPER,        // G
  UPPER,        // H
  UPPER,        // I
  UPPER,        // J
  UPPER,        // K
  UPPER,        // L
  UPPER,        // M
  UPPER,        // N
  UPPER,        // O
  UPPER,        // P
  UPPER,        // Q
  UPPER,        // R
  UPPER,        // S
  UPPER,        // T
  UPPER,        // U
  UPPER,        // V
  UPPER,        // W
  UPPER,        // X
  UPPER,        // Y
  UPPER,        // Z
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  UNDERSCORE,   // _
  OTHER,
  LOWER,        // a
  LOWER,        // b
  LOWER,        // c
  LOWER,        // d
  LOWER,        // e
  LOWER,        // f
  LOWER,        // g
  LOWER,        // h
  LOWER,        // i
  LOWER,        // j
  LOWER,        // k
  LOWER,        // l
  LOWER,        // m
  LOWER,        // n
  LOWER,        // o
  LOWER,        // p
  LOWER,        // q
  LOWER,        // r
  LOWER,        // s
  LOWER,        // t
  LOWER,        // u
  LOWER,        // v
  LOWER,        // w
  LOWER,        // x
  LOWER,        // y
  LOWER,        // z
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER
};
