CC       = $(CROSS_COMPILING)gcc -c 
CFLAGS   = -O2
LD       = $(CROSS_COMPILING)gcc -o
BITS	 := 32
DFLAGS :=
MSCRT    := -lmsvcrt
DLL_MAIN_STDCALL_NAME =

ifeq ($(BITS),64)
DLL_MAIN_STDCALL_NAME = DllMain
else
DLL_MAIN_STDCALL_NAME = _DllMain@12
endif

LDFLAGS  = -nodefaultlibs -Wl,-static -lmingw32 -lmingwex -lgcc -lkernel32 -luser32 -Wl,-s
CFLAGS   += $(DFLAGS)  -Wall -Wno-unused -Wno-format -Wno-int-to-pointer-cast -msse2 \
	    -fomit-frame-pointer -finline-functions -fno-stack-protector
LDLIBS   = -lshlwapi -lshell32 -lgdi32 $(MSCRT)
MD       = mkdir -p
CP        = cp
SRC      = src
SUB_DIR  = $(SRC)/mhook-lib
SUBMK    = $(MAKE) -C $(SUB_DIR)
DEP      = .dep
X86FLAG  = -m32
X64FLAG  = -m64
OBJECTS  = $(DEP)/portable.o $(DEP)/inipara.o $(DEP)/ice_error.o \
	   $(DEP)/safe_ex.o $(DEP)/bosskey.o $(DEP)/resource.o
DISTDIR  = Release
OUT1     = $(DISTDIR)/libmhook$(BITS).a
OUT2     = $(DISTDIR)/portable$(BITS).dll
RC       = $(CROSS_COMPILING)windres
RCFLAGS  = -l "LANGUAGE 4,2" -J rc -O coff
DLLFLAGS += -shared -Wl,--out-implib,$(DISTDIR)/libportable$(BITS).dll.a --entry=$(DLL_MAIN_STDCALL_NAME)
MKDLL	 += $(LD) $(DLLFLAGS) -shared -L$(DISTDIR) -lmhook(BITS)

EXEC     = \
    @echo Starting Compile... \
    $(shell $(MD) $(DISTDIR) 2>/dev/null) \
    $(shell $(MD) $(DEP) 2>/dev/null) \

ifeq ($(BITS),32)
    CFLAGS += $(X86FLAG)
    LDFLAGS += $(X86FLAG)
else
    ifeq ($(BITS),64)
        CFLAGS	+= $(X64FLAG)
        LDFLAGS += $(X64FLAG)
	RCFLAGS += -F pe-x86-64
    endif
endif

all		      : $(OUT1) $(OUT2)
$(OUT1)		      : $(SUB_DIR)/Makefile
	$(call SUBMK)
$(OUT2)		      : $(OBJECTS)
	$(LD) $@ $(OBJECTS) $(DLLFLAGS) $(OUT1) $(LDFLAGS) $(LDLIBS)
	-$(CP) $(DISTDIR)/libportable$(BITS).dll.a $(DISTDIR)/portable$(BITS).lib 2>/dev/null
$(DEP)/portable.o     : $(SRC)/portable.c $(SRC)/portable.h $(SRC)/ttf_list.h
	$(call EXEC)
	$(CC) $< $(CFLAGS) -o $@
$(DEP)/inipara.o      : $(SRC)/inipara.c $(SRC)/inipara.h
	$(CC) $< $(CFLAGS) -o $@
$(DEP)/safe_ex.o      : $(SRC)/safe_ex.c $(SRC)/safe_ex.h $(SRC)/header.h
	$(CC) $< $(CFLAGS) -o $@
$(DEP)/ice_error.o    : $(SRC)/ice_error.c $(SRC)/ice_error.h
	$(CC) $< $(CFLAGS) -o $@
$(DEP)/bosskey.o    : $(SRC)/bosskey.c $(SRC)/bosskey.h
	$(CC) $< $(CFLAGS) -o $@
$(DEP)/resource.o     : $(SRC)/resource.rc
	$(RC) -i $< $(RCFLAGS) -o $@

.PHONY		      : clean
clean                 : 
	-rm -rf $(DISTDIR) $(DEP)


