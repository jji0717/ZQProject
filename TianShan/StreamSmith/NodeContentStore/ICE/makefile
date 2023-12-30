TOPDIR := ../../..

include $(TOPDIR)/build/make.rules


CPSESSION := $(FREEZEFLAGS) \
			 CPSessions,string,TianShanIce::Storage::ContentProcess::CPSession \
			 SessionSafeStore

CONTENTDICT := $(FREEZEFLAGS) \
			   Contents,string,ContentDictData::ContentAttrs \
			   Data

PROVSCHDSAFESTORE := $(FREEZEFLAGS) \
					 ProvStatusMsgs,string,TianShanIce::Storage::StatusMsg \
					 StatusMsgData

IDLECONTENTS := $(FREEZEFLAGS) \
                IdleContents,string,string \
				IdleContents


ICEFILES := $(wildcard *.ice)
SRCS := $(patsubst %.ice, %.cpp, $(ICEFILES)) \
		SessionSafeStore.cpp Data.cpp StatusMsgData.cpp IdleContents.cpp
OBJS := $(SRCS:%.cpp=%.o)


all: $(OBJS)
	@echo "building object files ..."

$(OBJS): %.o: %.cpp
	@echo "compiling $< ..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(SRCS): $(ICEFILES)
	@echo "compiling ICE files..."
	@$(SLICE2CPP) $(SLICEFLAGS) $(ICEFILES) 
	@echo "compiling ICE for freeze..."
	@$(SLICE2FREEZE) $(SLICEFLAGS) $(CPSESSION) CPSession.ice 
	@$(SLICE2FREEZE) $(SLICEFLAGS) $(CONTENTDICT) ContentDict.ice 
	@$(SLICE2FREEZE) $(SLICEFLAGS) $(PROVSCHDSAFESTORE) ProvSchdSafeStore.ice 
	@$(SLICE2FREEZE) $(SLICEFLAGS) $(IDLECONTENTS)

clean:
	@echo "cleaning object files..."
	@rm -vf *.cpp *.h *.o *~

