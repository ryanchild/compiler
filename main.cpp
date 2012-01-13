#include <string>
#include <iostream>

#include "Scanner.h"

using namespace std;

int main(int argc, char** argv)
{
  Scanner s("test");
  string token;
  while(s.scan(token))
  {
    cout << "recognized token: '" << token << "' on line " << s.getLineNumber() << endl;
  }
  return 0;
}
