#include <string>
#include <iostream>

#include "Scanner.h"
#include "Token.h"

using namespace std;

int main(int argc, char** argv)
{
  Scanner s("test");
  Token token;
  while(s.scan(token))
  {
    cout << "recognized token: '" << token << "' on line " << s.getLineNumber() << endl;
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
