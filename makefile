
# define the directories
SRCDIR := src
INCDIR := include
BUILDDIR := build
BINDIR := bin
LIBDIR := lib

# define common variables
SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -iname "*.$(SRCEXT)" )
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
INC := $(shell find $(INCDIR) -maxdepth 1 -type d -exec echo -I {}  \;)

# flags
CFLAGS := -g -lz 


# define specific binaries to create
APP0 := onion-helper
TARGET := $(BINDIR)/$(APP0)



all: info $(TARGET) tester

$(TARGET): $(OBJECTS) 
	@mkdir -p $(BINDIR)
	@echo " Linking..."
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(LIB)


# generic: build any object file required
$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	$(RM) -r $(BUILDDIR) $(BINDIR) $(LIB0_TARGET)

info:
	@echo "CC: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "LIB: $(LIB)"
	@echo "INC: $(INC)"
	@echo "SOURCES: $(SOURCES)"
	@echo "OBJECTS: $(OBJECTS)"


# Tests
tester:
	@echo " Compiling tester"
	$(CC) $(CFLAGS) test/tester.c $(INC) $(LIB) -c -o build/tester.o
	@echo " Linking tester"
	$(CC) build/tester.o build/background-process.o build/curl-intf.o $(CFLAGS) $(LDFLAGS) -o bin/tester $(LIB)

# Spikes
#ticket:
#  $(CXX) $(CXXFLAGS) spikes/ticket.c $(INC) $(LIB) -o bin/ticket

.PHONY: clean
