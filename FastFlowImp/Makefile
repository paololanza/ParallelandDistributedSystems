CC  =  g++
CFLAGS = -g -O3 -std=c++20 
INCLUDES = -I /usr/local/include
LIBS = -pthread

# aggiungere qui altri targets
TARGETS = FFHuffman
PARAMS = "../test.txt"


all: $(TARGETS)

$(TARGETS): $(TARGETS).cpp
		  $(CC) $(CFLAGS) -o $(TARGETS) $(TARGETS).cpp

clean: 
	rm -f $(TARGETS)

cleanall	: clean

run: 
	./$(TARGETS) $(PARAMS)
