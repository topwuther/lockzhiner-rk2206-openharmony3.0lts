OBJS_DIR:=$(BUILD_DIR)/codegen
TARGET = $(OBJS_DIR)/codegen.a
SOURCE:=$(wildcard *.cpp)
OBJS:=$(patsubst %.cpp, $(OBJS_DIR)/%.o, $(SOURCE))

$(TARGET):$(OBJS)
	$(Q) echo $(TARGET)
	$(Q) ar -rc $@ $^

$(OBJS_DIR)/%.o:%.cpp
	$(Q) mkdir -p $(dir $@)
	$(Q) $(CXX) $(CXXFLAGS) $(INCLUDES) -c $^ -o $@