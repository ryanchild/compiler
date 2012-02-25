CXX = g++
CXXFLAGS = -g -c -m32
OUT = compiler

$(OUT): main.o Scanner.o Token.o Parser.o
	$(CXX) -m32 -o $@ $^

main.o: main.cpp Scanner.h Token.h
	$(CXX) $(CXXFLAGS) main.cpp

Scanner.o: Scanner.cpp Scanner.h Token.h
	$(CXX) $(CXXFLAGS) Scanner.cpp

Token.o: Token.cpp Token.h
	$(CXX) $(CXXFLAGS) Token.cpp

Parser.o: Parser.cpp Parser.h Scanner.h Token.h
	$(CXX) $(CXXFLAGS) Parser.cpp

clean:
	rm -f *.o
	rm -f $(OUT)
