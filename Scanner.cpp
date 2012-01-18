#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>

#include "Scanner.h"
#include "Token.h"

using namespace std;

Scanner::Scanner(const char* filename)
  :mFile(NULL)
  ,mLineNumber(1)
{
  // build string map
  mStrings["for"] = Token::FOR;
  mStrings["end"] = Token::END;
  mStrings["function"] = Token::FUNCTION;
  mStrings["if"] = Token::IF;
  mStrings["then"] = Token::THEN;
  mStrings["global"] = Token::GLOBAL;
  mStrings["integer"] = Token::INTEGER;
  mStrings["boolean"] = Token::BOOLEAN;
  mStrings["float"] = Token::FLOAT;
  mStrings["string"] = Token::STRING;
  mStrings["not"] = Token::NOT;
  mStrings["+"] = Token::ADDSUBTRACT;
  mStrings["-"] = Token::ADDSUBTRACT;
  mStrings["/"] = Token::MULTDIV;
  mStrings["*"] = Token::MULTDIV;
  mStrings["="] = Token::ASSIGNMENT;
  mStrings["["] = Token::OPENSQUARE;
  mStrings["]"] = Token::CLOSESQUARE;
  mStrings["("] = Token::OPENPAREN;
  mStrings[")"] = Token::CLOSEPAREN;
  mStrings["<"] = Token::LT;
  mStrings[">"] = Token::GT;
  mStrings["=="] = Token::EQUAL;
  mStrings["!="] = Token::NOTEQUAL;
  mStrings["\""] = Token::QUOTE;

  mFile = fopen(filename, "r");
}

Scanner::~Scanner()
{
  fclose(mFile);
}

void Scanner::printErrors()
{
  for(vector<error>::iterator it(mErrors.begin()); it != mErrors.end(); ++it)
  {
    error err = *it;
    cout << "line " << err.lineNum << ": " << err.msg << endl;
  }
}

void Scanner::addIllegalCharacterError(char c)
{
  ostringstream msg;
  msg << "unrecognized character: " << c << endl;
  error err = {mLineNumber, msg.str()};
  mErrors.push_back(err);
}

bool Scanner::scan(Token& tok)
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
    else if(cc == FORWARDSLASH)
    {
      c = fgetc(mFile);
      if(c == '/')
      {
        while(fgetc(mFile) != '\n');
        mLineNumber++;
      }
      else
      {
        ungetc(c,mFile);
        c = '/';
        break;
      }
    }
    else if(cc == EXCLAMATION)
    {
      c = fgetc(mFile);
      ungetc(c,mFile);
      if(c == '=')
        c = '!';
      else
        addIllegalCharacterError('!');
    }
    else if(cc == OTHER)
    {
      addIllegalCharacterError(c);
    }
    else
      break;
  }

  StringIt strit;
  switch(cc)
  {
    case FORWARDSLASH:
    case MINUS:
    case PLUS:
    case ASTERISK:
    case GT:
    case LT:
    case BRACKET:
      strit = mStrings.find((char*)&c);
      break;

    case EQUALS:
      c = fgetc(mFile);
      if(c == '=')
        strit = mStrings.find("==");
      else
      {
        ungetc(c,mFile);
        strit = mStrings.find("=");
      }
      break;

    case EXCLAMATION:
      c = fgetc(mFile);
      if(c == '=')
        strit = mStrings.find("!=");
      break;

    case DIGIT: 
    {
      ostringstream oss;
      while(cc == DIGIT)
      {
        oss << (char)c;
        c = fgetc(mFile);
        cc = charclasses[c];
      }
      ungetc(c,mFile);
      strit = mStrings.find(oss.str());
      if(strit == mStrings.end())
      {
        mStrings[oss.str()] = Token::INTEGER;
        strit = mStrings.find(oss.str());
      }
      break;
    }

    case LOWER:
    case UPPER:
    {
      ostringstream oss;
      while(cc == LOWER || cc == UPPER || cc == UNDERSCORE || cc == DIGIT)
      {
        oss << (char)c;
        c = fgetc(mFile);
        cc = charclasses[c];
      }
      ungetc(c,mFile);
      strit = mStrings.find(oss.str());
      if(strit == mStrings.end())
      {
        mStrings[oss.str()] = Token::IDENTIFIER;
        strit = mStrings.find(oss.str());
      }
      break;
    }

    case DOUBLEQUOTE:
    {
      ostringstream oss;
      bool foundError = false;
      c = fgetc(mFile);
      cc = charclasses[c];
      while(cc != DOUBLEQUOTE)
      {
        oss << (char)c;
        if(cc != LOWER && cc != UPPER && cc != UNDERSCORE && cc != DIGIT
            && c != ' ')
        {
          foundError = true;
        }
        c = fgetc(mFile);
        cc = charclasses[c];
      }
      if(foundError)
      {
        ostringstream msg;
        msg << "illegal string: " << oss.str() << endl;
        error err = {mLineNumber, msg.str()};
        mErrors.push_back(err);
      }
      else
      {
        strit = mStrings.find(oss.str());
        if(strit == mStrings.end())
        {
          mStrings[oss.str()] = Token::STRING;
          strit = mStrings.find(oss.str());
        }
      }
      break;
    }


    default: break;
  }
  if(strit != mStrings.end())
  {
    tok.setString(strit->first.c_str());
    tok.setType(strit->second);
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
  EXCLAMATION,  // !
  DOUBLEQUOTE,  // "
  OTHER,
  OTHER,
  OTHER,
  OTHER,
  OTHER,        // '
  BRACKET,      // (
  BRACKET,      // )
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
  OTHER,        // ;
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
  BRACKET,      // [
  OTHER,
  BRACKET,      // ]
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
