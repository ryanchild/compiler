CXX = g++
OUT = compiler

$(OUT): main.o Scanner.o
	$(CXX) -o $@ $^

main.o: main.cpp Scanner.h
	$(CXX) -c main.cpp

Scanner.o: Scanner.cpp Scanner.h
	$(CXX) -c Scanner.cpp

clean:
	rm -f *.o
	rm -f $(OUT)
