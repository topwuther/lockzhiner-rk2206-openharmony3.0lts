OBJS_DIR:=$(BUILD_DIR)/parser
TARGET = $(OBJS_DIR)/parser.a
SOURCE:=$(wildcard *.cpp)
OBJS:=$(patsubst %.cpp, $(OBJS_DIR)/%.o, $(SOURCE))

$(TARGET):$(OBJS)
	$(Q) echo $(TARGET)
	$(Q) ar -rc $@ $^

$(OBJS_DIR)/%.o:%.cpp
	$(Q) mkdir -p $(dir $@)
	$(Q) $(CXX) $(CXXFLAGS) $(INCLUDES) -c $^ -o $@