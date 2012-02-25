#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <iostream>

#include "Parser.h"
#include "Scanner.h"
#include "Token.h"

using namespace std;

int main(int argc, char** argv)
{
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

  Parser p(&s, genfile.c_str());
  if(!p.parse())
    cout << "error parsing " << filename << endl;
  else
  {
    string cmd = "gcc " + genfile + " runtime.c -lm";
    system(cmd.c_str());
  }

  return 0;
}
