TARGET = hdi-gen

PWD:=$(shell pwd)
export BUILD_DIR = $(PWD)/build
export BOUNDS_CHECK_LIB := $(abspath ../../../../third_party/bounds_checking_function)
export INCLUDES = -I$(PWD) -I$(BOUNDS_CHECK_LIB)/include
export CXXFLAGS = -std=c++14 -O2
export Q := @
export MAKEFLAGS += --no-print-directory

BOUNDS_CHECK_SOURCE := $(wildcard $(BOUNDS_CHECK_LIB)/src/*.c)
BOUNDS_CHECK_BUILD := $(BUILD_DIR)/bounds_checking_function
BOUNDS_CHECK_OBJS := $(addprefix $(BOUNDS_CHECK_BUILD)/, $(patsubst %.c, %.o, $(notdir $(BOUNDS_CHECK_SOURCE))))
BOUNDS_CHECK_TARGET := $(BOUNDS_CHECK_BUILD)/bounds_checking_function.a

SOURCE := $(wildcard *.cpp)
OBJS := $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp, %.o, $(SOURCE)))

SUBDIRS = util ast parser codegen
LIBS = $(BUILD_DIR)/codegen/codegen.a $(BUILD_DIR)/parser/parser.a $(BUILD_DIR)/ast/ast.a $(BUILD_DIR)/util/util.a

all:$(TARGET)

$(TARGET): $(OBJS) | $(BOUNDS_CHECK_TARGET)
	$(Q) for subdir in $(SUBDIRS); do \
		$(MAKE) -C $$subdir || exit 1; \
	done
	$(Q) $(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LIBS) $(BOUNDS_CHECK_TARGET)
	$(Q) echo $(TARGET):build successful.

$(OBJS):$(SOURCE)
	$(Q) mkdir -p $(BUILD_DIR)
	$(Q) $(CXX) $(CXXFLAGS) $(INCLUDES) -c $^ -o $@

$(BOUNDS_CHECK_TARGET):$(BOUNDS_CHECK_OBJS)
	$(Q) echo $(BOUNDS_CHECK_TARGET)
	$(Q) ar -rc $@ $^

$(BOUNDS_CHECK_BUILD)/%.o : $(BOUNDS_CHECK_LIB)/src/%.c
	$(Q) mkdir -p $(BOUNDS_CHECK_BUILD)
	$(Q) $(CXX) $(CXXFLAGS) $(INCLUDES) -c $^ -o $@

clean:
	$(Q) rm -rf $(BUILD_DIR) $(TARGET)

.PHONY:all clean