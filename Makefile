
HCONLIB_ROOT=/home/haakon/Documents/Code/C++/HConLib
SPURV_ROOT=/home/haakon/Documents/Code/C++/spurv

WG_OBJS=Wingine.o WgUtils.o
TEST_OBJS=test.o
# EXTERNAL_OBJS=$(SPURV_ROOT)/constant_registry.o $(SPURV_ROOT)/types.o $(SPURV_ROOT)/uniforms.o $(SPURV_ROOT)/utils.o


INCLUDE_DIRS=-I . -I $(HCONLIB_ROOT)/include -I $(SPURV_ROOT)/include -I $(VULKAN_SDK)/include

LIB_DIRS=-L $(VULKAN_SDK)/lib -L $(HCONLIB_ROOT)/lib -L $(SPURV_ROOT)/lib

LIBS=-lspurv -lWinval -lX11 -lvulkan -lFlatAlg 

OPTIONS=-g -O0 -std=c++1z -Wall -Wno-delete-non-virtual-dtor

all: texture_test test

test: $(WG_OBJS) test.o
	g++ -o $@ $^ $(LIB_DIRS) $(LIBS) $(OPTIONS)

texture_test: $(WG_OBJS) texture_test.o
	g++ -o $@ $^ $(LIB_DIRS) $(LIBS) $(OPTIONS)

%.o: %.cpp $(HDRS)
	g++ -c -o $@ $< $(INCLUDE_DIRS) $(OPTIONS)



clean:
	rm texture_test test *.o
