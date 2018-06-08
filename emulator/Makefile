PRODUCT := casioemu
SRCDIR := src
DEPDIR := dep
BINDIR := bin
OBJDIR := obj

LIBSTOLINK :=
INCLUDEDIRS :=

CXX ?= g++
CCEXT ?= cpp
CCFLAGS ?= -g -std=c++11 -Wall -Wextra -Werror -pedantic
CCLINK ?= $(LIBSTOLINK) -lSDL2 -lSDL2_image -llua5.3 -lpthread

FILES := $(shell find $(SRCDIR) -name *.$(CCEXT))

# RELATIVEPATH only affects GCC output

$(BINDIR)/$(PRODUCT): $(patsubst %,$(OBJDIR)/%.o,$(subst /,__,$(patsubst $(SRCDIR)/%,%,$(FILES)))) $(LIBSTOLINK) Makefile
	@$(CXX) -o $(BINDIR)/$(PRODUCT) $(patsubst %,$(OBJDIR)/%.o,$(subst /,__,$(patsubst $(SRCDIR)/%,%,$(FILES)))) $(CCLINK)

-include $(patsubst %,$(DEPDIR)/%.dep,$(subst /,__,$(patsubst $(SRCDIR)/%,%,$(FILES))))

$(OBJDIR)/%.$(CCEXT).o: Makefile
	@$(eval CPATH := $(patsubst %,$(SRCDIR)/%,$(subst __,/,$*.$(CCEXT))))
	@$(eval DPATH := $(patsubst %,$(DEPDIR)/%,$*.$(CCEXT)).dep)
	@$(eval RELBACK := `echo $(RELATIVEPATH) | sed -e 's/[^/][^/]*/\.\./g'`)
	@cd $(RELATIVEPATH). && $(CXX) $(CCFLAGS) $(subst -I,-I$(RELBACK),$(INCLUDEDIRS)) -c ./$(RELBACK)$(CPATH) -o ./$(RELBACK)$(OBJDIR)/$*.$(CCEXT).o
	@cd $(RELATIVEPATH). && $(CXX) $(CCFLAGS) $(subst -I,-I$(RELBACK),$(INCLUDEDIRS)) -MM ./$(RELBACK)$(CPATH) > ./$(RELBACK)$(DPATH)
	@mv -f $(DPATH) $(DPATH).tmp
	@sed -e 's|.*:|$(OBJDIR)/$*.$(CCEXT).o:|' < $(DPATH).tmp > $(DPATH)
	@sed -e 's/.*://' -e 's/ .$$//' -e 's/^ *//' -e 's/ *$$//' -e 's/$$/:/' < $(DPATH).tmp >> $(DPATH)
	@rm -f $(DPATH).tmp

clean:
	@rm -f $(BINDIR)/$(PRODUCT) $(OBJDIR)/*.o $(DEPDIR)/*.dep
