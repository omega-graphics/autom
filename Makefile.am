AUTOMC_LIB_OBJS := build/automc/base.o build/automc/interp.o build/automc/lexer.o build/automc/parser.o
AUTOM_EXE_OBJS := build/autom/build.o
BUILD_BIN := build/bin
# Compiler
COMPILER := clang++ -stdlib=libc++ -std=c++17
LINKER := clang++
BUILD_DIR := build/automc
SO_FLAGS = -fPIC
LD_FLAGS =  -lc++ -lc++abi
INCLUDE_DIRS = -Icc

build:
	mkdir build
build/automc : build
	mkdir build/automc

# automc build

build/automc/base.o : build build/automc
	$(COMPILER) $(SO_FLAGS) -c cc/automc/base.cpp -o $(BUILD_DIR)/base.o

build/automc/interp.o :
	$(COMPILER) $(SO_FLAGS) -c cc/automc/interp.cpp -o $(BUILD_DIR)/interp.o

build/automc/lexer.o:
	$(COMPILER) $(SO_FLAGS) -c cc/automc/lexer.cpp -o $(BUILD_DIR)/lexer.o

build/automc/parser.o:
	$(COMPILER) $(SO_FLAGS) -c cc/automc/parser.cpp -o $(BUILD_DIR)/parser.o

build/automc.dylib : $(AUTOMC_LIB_OBJS)
	$(LINKER) -o build/automc.dylib --shared $(AUTOMC_LIB_OBJS) $(LD_FLAGS)

# autom build
build/autom : build
	mkdir build/autom

build/bin :
	mkdir build/bin

build/autom/build.o : build/autom
	$(COMPILER) $(SO_FLAGS) $(INCLUDE_DIRS) -c cc/autom/main.cpp -o build/autom/build.o

build/bin/autom : $(AUTOM_EXE_OBJS) build/automc.dylib build/bin
	$(LINKER) -o $(BUILD_BIN)/autom  $(AUTOM_EXE_OBJS) $(LD_FLAGS)

clean:
	rm -r -d build/automc

all: build/bin/autom build/automc.dylib