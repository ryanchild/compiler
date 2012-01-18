CXX = g++
OUT = compiler

$(OUT): main.o Scanner.o Token.o
	$(CXX) -o $@ $^

main.o: main.cpp Scanner.h Token.h
	$(CXX) -g -c main.cpp

Scanner.o: Scanner.cpp Scanner.h Token.h
	$(CXX) -g -c Scanner.cpp

Token.o: Token.cpp Token.h
	$(CXX) -g -c Token.cpp

clean:
	rm -f *.o
	rm -f $(OUT)
