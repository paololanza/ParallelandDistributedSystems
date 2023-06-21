CC  =  g++
CFLAGS += -std=c++20 #-Wall -g 
ARFLAGS         =  rvs
INCLUDES	= -I.
LDFLAGS 	= -L.
OPTFLAGS	= -O3 
LIBS            = -pthread

# aggiungere qui altri targets
TARGETS		= Client		\
		  ServerObject   

.PHONY: all clean cleanall
.SUFFIXES: .c .h

%: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<

all		: $(TARGETS)

clean		: 
	rm -f $(TARGETS)
cleanall	: clean