#include <cstring>
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
  string genfile = 
    filename.substr(0, int( strrchr(filename.c_str(), '.') - filename.c_str()));
  genfile += ".c";

  Scanner s(filename.c_str());
  if(!s.initialize())
  {
    cout << "Error opening " << argv[1] << ": " << strerror(errno) << endl;
    return 1;
  }

  Parser p(&s, genfile.c_str());
  if(!p.parse())
    cout << "error parsing " << filename << endl;

  int ne = s.numErrors();
  if(ne > 0)
  {
    const char* plural = ne == 1 ? "" : "s";
    cout << endl << ne << " fatal error" << plural << " found. Aborting..." 
      << endl;
    s.printErrors();
  }

  return 0;
}
