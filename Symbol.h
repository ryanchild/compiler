class Symbol
{
  public:
    Symbol(datatype dt, long addr, const char* id)
      :mDt(dt)
      ,mAddr(addr)
      ,mId(id)
    {}

    virtual bool typeMatchesWith(Symbol*);

    enum datatype
    {
      INTEGER,
      FLOAT,
      BOOLEAN,
      STRING
    };

  protected:
    datatype mDt;
    long mAddr;
    const char* mId;
};

class ArraySymbol : public Symbol
{
  private:
    int mSize;
};

class FunctionSymbol : public Symbol
{
  public:
    FunctionSymbol(datatype dt, long addr, const char* id, 
        std::vector<Symbol*>& params)
      :Symbol(dt, addr, id)
      ,mParams(params)
  {}

  private:
    std::vector<Symbol*> mParams;
};
