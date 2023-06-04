# Tiny Language Compiler (tlc)
# Makefile
# 2016年 木村啓二

# PLATFORM = RASPI
PLATFORM = LINUX
#PLATFORM = MAC
#PLATFORM = ARMMAC
#PLATFORM = CYGWIN

ifeq ($(PLATFORM), RASPI)
TARGET_FLAG = -DTARGET_RASPI
LFLAGS = -ll -ly
SRCS_ARCH = arch_arm64.c
OBJS_ARCH = arch_arm64.o
DEPS_ARCH = arch_arm64.d
else ifeq ($(PLATFORM), LINUX)
TARGET_FLAG = -DTARGET_LINUX
LFLAGS = -ll -ly
SRCS_ARCH = arch_x64.c
OBJS_ARCH = arch_x64.o
DEPS_ARCH = arch_x64.d
else ifeq ($(PLATFORM), MAC)
TARGET_FLAG = -DTARGET_MAC
LFLAGS = -ll -ly
SRCS_ARCH = arch_x64.c
OBJS_ARCH = arch_x64.o
DEPS_ARCH = arch_x64.d
else ifeq ($(PLATFORM), ARMMAC)
TARGET_FLAG = -DTARGET_AMAC
LFLAGS = -ll -ly
SRCS_ARCH = arch_arm64.c
OBJS_ARCH = arch_arm64.o
DEPS_ARCH = arch_arm64.d
else ifeq ($(PLATFORM), CYGWIN)
TARGET_FLAG = -DTARGET_CYGWIN
LFLAGS = -lfl -ly
SRCS_ARCH = arch_x64.c
OBJS_ARCH = arch_x64.o
DEPS_ARCH = arch_x64.d
endif

TARGET = tlc
SRCS = main.c tl_gram.y tl_lex.l util.c util.h ast.c ast.h parse_action.c parse_action.h symtab.c symtab.h cg.c cg.h
OBJS = main.o tl_gram.o tl_lex.o util.o ast.o parse_action.o symtab.o cg.o
DEPS = main.d util.d ast.d parse_action.d symtab.d cg.d $(DEPS_ARCH)
FETMPS = tl_lex.c tl_gram.c tl_gram.h


CFLAGS = -O0 -Wall -g

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(DEPS) $(OBJS) $(OBJS_ARCH)
	gcc -o $@ $(OBJS) $(OBJS_ARCH) $(LFLAGS)

ifneq ($(filter clean,$(MAKECMDGOALS)),clean)
-include $(DEPS)
endif

%.d: %.c
	gcc -MM $(CFLAGS) $(TARGET_FLAG) $< | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@

%.o: %.c
	gcc $(CFLAGS)  $(TARGET_FLAG) -c $<

tl_lex.c: tl_lex.l tl_gram.c
	flex -o $@ $<

tl_gram.c: tl_gram.y ast.h parse_action.h
	bison -d -o $@ $<

clean:
	-rm -f *~ *.o *.d $(TARGET) $(FETMPS)
