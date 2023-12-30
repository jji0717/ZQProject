objects = AsyncFrame.o AsyncServerLinuxImpl.o IAsyncClientHandler.o IAsyncServer.o ICommunicator.o\
          IMgr.o Locks.o MgrLinuxImpl.o NativeThread.o

CXXFLAGS += -g
CXXFLAGS += -DZQ_OS_LINUX

LDFLAGS += -lpthread -ldl -lrt -g

AsyncFrame: $(objects)
	$(CXX) $(objects) -L. $(LDFLAGS) -o $@

$(objects): %.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -DZQ_OS_LINUX -DDEBUG $< -o $@ 

clean:
	rm $(objects)
