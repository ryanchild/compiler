#include <string.h>
#include <errno.h>
#include <iostream>

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

  Scanner s(filename.c_str());
  if(!s.initialize())
  {
    cout << "Error opening " << argv[1] << ": " << strerror(errno) << endl;
    return 1;
  }

  Token t;
  while(s.scan(t))
  {
    cout << "recognized token: '" << t << "' on line " << s.getLineNumber() << endl;
  }

  int ne = s.numErrors();
  if(ne > 0)
  {
    const char* plural = ne == 1 ? "" : "s";
    cout << endl << ne << " fatal error" << plural << " found. Aborting..." << endl;
    s.printErrors();
  }

  return 0;
}
