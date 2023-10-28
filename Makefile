SYSCONF_LINK = g++
CPPFLAGS     =
LDFLAGS      =
LIBS         = -lm

DESTDIR = ./bin/
TARGET  = main

OBJDIR := ./obj/
OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))
OBJECTS := $(addprefix $(OBJDIR), $(OBJECTS))

all: $(DESTDIR)$(TARGET)

run: $(DESTDIR)$(TARGET)
	./$(DESTDIR)$(TARGET) && feh -Z -F --force-aliasing output.tga

$(DESTDIR)$(TARGET): $(OBJECTS)
	$(SYSCONF_LINK) -Wall $(LDFLAGS) -o $(DESTDIR)$(TARGET) $(OBJECTS) $(LIBS)

$(OBJECTS): $(OBJDIR)%.o: %.cpp
	$(SYSCONF_LINK) -Wall $(CPPFLAGS) -c $(CFLAGS) $< -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)
	-rm -f *.tga

