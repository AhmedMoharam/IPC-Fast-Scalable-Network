.PHONY: $(TARGET)

BUILD_DIR := build
SRCS := $(shell echo *.cpp)
OBJS = $(addprefix $(BUILD_DIR)/obj/,$(patsubst %.cpp,%.o,$(SRCS)))
TARGET  = $(BUILD_DIR)/libipc.a
HEADERS = $(shell echo *.h)
CPPFLAGS = -g -Wall

$(BUILD_DIR)/obj/%.o: %.cpp $(HEADERS) 
	@mkdir -p $(BUILD_DIR)/obj
	$(CC) $(CPPFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(AR) rcs $@ $^

clean:
	rm -rf *.a *~ *.o ${BUILD_DIR}
