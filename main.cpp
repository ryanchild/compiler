#include <string>
#include <iostream>

#include "Scanner.h"
#include "Token.h"

using namespace std;

int main(int argc, char** argv)
{
  string filename;
  if(argc != 2)
  {
    cout << "1 argument required" << endl << "sample usage: " << argv[0] << " <source file>" << endl;
    return 1;
  }
  else
    filename = argv[1];

  Scanner s(filename.c_str());
  if(!s.initialize())
  {
    cout << "error initializing scanner.  are you sure the file exists?" << endl;
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
    cout << endl << ne << " fatal error" << plural << " found. aborting..." << endl;
    s.printErrors();
  }

  return 0;
}
