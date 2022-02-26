OBJS_DIR:=$(BUILD_DIR)/ast
TARGET = $(OBJS_DIR)/ast.a
SOURCE:=$(wildcard *.cpp)
OBJS:=$(patsubst %.cpp, $(OBJS_DIR)/%.o, $(SOURCE))

$(TARGET):$(OBJS)
	$(Q) echo $(TARGET)
	$(Q) ar -rc $@ $^

$(OBJS_DIR)/%.o:%.cpp
	$(Q) mkdir -p $(dir $@)
	$(Q) $(CXX) $(CXXFLAGS) $(INCLUDES) -c $^ -o $@