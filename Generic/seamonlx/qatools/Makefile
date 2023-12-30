TARGET = bcast

# Add new cpp files here
SRCS =	bcasttest.cpp

OBJS = $(SRCS:.cpp=.o)

#INCLUDES = -I../../includes


CFLAGS += -g -W -Wall
 

#CFLAGS += -g -W -Wall $(INCLUDES)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo building executable $@
	$(CXX) $(CFLAGS) -o $@ $^

$(OBJS):%.o:%.cpp
#	$(CXX) -c $(CFLAGS) $(INCLUDES) $<
	$(CXX) -c $(CFLAGS) $<
depend:
	makedepend -f- -- $(SRCS) > .depend_file

clean:
	rm -f *.o *~ core $(TARGET) .depend_file

