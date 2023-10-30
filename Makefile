SYSCONF_LINK = g++
CPPFLAGS     = -Ilib/glm/ -Isrc/
LDFLAGS      = 
LIBS         = -lm

BIN_DIR = bin
TARGET  = main
EXECUTABLE = $(BIN_DIR)/$(TARGET)

SRC_DIR   = src
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_DIR   = obj
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))

.PHONY: all clean

all: $(EXECUTABLE)

run: all
	./$(EXECUTABLE) && feh -Z -F --force-aliasing output.tga

$(EXECUTABLE): $(OBJ_FILES)
	$(SYSCONF_LINK) -Wall $(LDFLAGS) -o $(EXECUTABLE) $(OBJ_FILES) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(SYSCONF_LINK) -Wall $(CPPFLAGS) -c $(CFLAGS) $< -o $@

clean:
	-rm -f $(OBJ_FILES)
	-rm -f $(EXECUTABLE)
	# -rm -f *.tga
