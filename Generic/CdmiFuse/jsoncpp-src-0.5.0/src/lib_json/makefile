#!/bin/sh
all:json

INCPATH := ../../include

OBJS := json_reader.o  json_value.o  json_writer.o

%.o : %.cpp
	g++ -o3 -I${INCPATH} -fPIC -o $@ -c $^

json:${OBJS}
	ar -rcs libjson.a ${OBJS}

.PHONY:clean

clean:
	- rm -f *.o
	- rm -f libjson.a
