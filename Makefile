CXX      := -g++
CXXFLAGS := -Wall -Wextra -std=c++11 -lsfml-graphics -lsfml-window -lsfml-system -O3 -lrt
LDFLAGS  := -L/usr/lib -lstdc++
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := .
TARGET   := program
INCLUDE  := -Iinclude/
SRC      :=  $(wildcard *.cc)

OBJECTS := $(SRC:%.cc=$(OBJ_DIR)/%.o)

all: build $(APP_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: %.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $<

$(APP_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(OBJECTS) $(CXXFLAGS) $(INCLUDE) $(LDFLAGS) -o $(APP_DIR)/$(TARGET)

.PHONY: all build clean debug release

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O2
release: all

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(TARGET)
	-@rm -rvf *.ppm
