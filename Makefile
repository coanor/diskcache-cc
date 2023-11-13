# examples fome https://stackoverflow.com/a/32168987/342348
TARGET = lib.a

$(TARGET): open.o put.o
	@ar rcs $@ $^

open.o: open.cc
	@g++ -std=c++2a -c $< -o $@

put.o: put.cc
	@g++ -std=c++2a -c $< -o $@

clean:
	@rm -rf *.o *.a *.gch $(TARGET)
