########################################################################
####################### Makefile Template ##############################
########################################################################

# Compiler settings - Can be customized.
CC = gcc
CXXFLAGS = -std=c11 -Wall -lm -D COMPILER=\"$(CC)\" -D BRACE_LIB_PATH=\"$(libpath)\"
LDFLAGS = 

# Makefile settings - Can be customized.
APPNAME = app
EXT = .c
SRCDIR = src
HEADERDIR = include
BINDIR = bin
OBJDIR = $(BINDIR)/obj

############## Do not change anything from here downwards! #############
SRC = $(wildcard $(SRCDIR)/*$(EXT))
OBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)/%.o)
APP = $(BINDIR)/$(APPNAME)
DEP = $(OBJ:$(OBJDIR)/%.o=%.d)

DEBUGFLAGS = -D DEBUG_TRACE_EXECUTION -D DEBUG_PRINT_CODE

# count stuff
OBJCOUNT_NOPAD = $(shell v=`echo $(OBJ) | wc -w`; echo `seq 1 $$(expr $$v)`)
OBJCOUNT = $(foreach v,$(OBJCOUNT_NOPAD),$(shell printf '%02d' $(v)))

# UNIX-based OS variables & settings
RM = rm
MKDIR = mkdir
DELOBJ = $(OBJ)

########################################################################
####################### Targets beginning here #########################
########################################################################

.MAIN: $(APP)
all: $(APP)

# Builds the app
$(APP): $(OBJ) | makedirs
	@printf "[final] compiling final product $(notdir $@)..."
	@$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@#cp $(APP) "."
	@printf "\b\b done!\n"

# Building rule for .o files and its .c/.cpp in combination with all .h
# $(OBJDIR)/%.o: $(SRCDIR)/%$(EXT) | makedirs
$(OBJDIR)/%.o: $(SRCDIR)/%$(EXT) | makedirs
	@printf "[$(word 1,$(OBJCOUNT))/$(words $(OBJ))] compiling $(notdir $<) into $(notdir $@)..."
	@$(CC) $(CXXFLAGS) -I $(HEADERDIR) -o $@ -c $<
	@printf "\b\b done!\n"
	$(eval OBJCOUNT = $(filter-out $(word 1,$(OBJCOUNT)),$(OBJCOUNT)))

################### Cleaning rules for Unix-based OS ###################
# Cleans complete project
.PHONY: clean
clean:
	@# $(RM) $(DELOBJ) $(DEP) $(APP)
	@$(RM) -rf $(OBJDIR)
	@$(RM) -rf $(BINDIR)

.PHONY: run
run: $(APP)
	@printf "============ Running \"$(APP)\" with args \"$(args)\" ============\n\n"
	@$(APP) $(args)

.PHONY: routine
routine: $(APP) run clean

.PHONY: makedirs
makedirs:
	@$(MKDIR) -p $(BINDIR)
	@$(MKDIR) -p $(OBJDIR)

.PHONY: remake
remake: clean $(APP)

.PHONY: printdebug
printdebug:
	@printf "debug mode set!\n"

# .PHONY: debug
debug: CXXFLAGS += $(DEBUGFLAGS)
debug: printdebug
debug: all
