#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <iostream>
#include <sstream>

#include "Parser.h"
#include "Scanner.h"
#include "Token.h"

using namespace std;

int main(int argc, char** argv)
{
  const char* tempfilename = "wgen.tmp";
  string filename;
  if(argc != 2)
  {
    cout << "usage: " << endl << argv[0] << " <source file>" << endl;
    return 1;
  }
  else
    filename = argv[1];

  // get the file base
  string base = 
    filename.substr(0, int( strrchr(filename.c_str(), '.') - filename.c_str()));
  string genfile = base + ".c";

  Scanner s(filename.c_str());
  if(!s.initialize())
  {
    cout << "Error opening " << argv[1] << ": " << strerror(errno) << endl;
    return 1;
  }

  ostringstream oss;
  Parser p(&s, tempfilename);
  if(!p.parse())
  {
    cout << "error parsing " << filename << endl;
    oss << "rm " << tempfilename;
    system(oss.str().c_str());
    return 1;
  }
  else
  {
    oss << "mv " << tempfilename << " " << genfile;
    system(oss.str().c_str());
    oss.str("");
    oss << "gcc " << genfile << " runtime.c -lm";
    system(oss.str().c_str());
  }

  return 0;
}
