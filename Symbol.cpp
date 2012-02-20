bool Symbol::typeMatchesWith(Symbol* s)
{
  return s->mDt == mDt;
}

bool ArraySymbol::typeMatchesWith(Symbol* s)
{
  ArraySymbol* arrs = dynamic_cast<ArraySymbol*>(s);
  return arrs && 
         Symbol::typeMatchesWith(s) &&
         arrs->mSize == mSize;
}

bool FunctionSymbol::typeMatchesWith(Symbol* s)
{
  FunctionSymbol(Symbol* s)
         
