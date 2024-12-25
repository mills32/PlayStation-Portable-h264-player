TARGET = mpeg1
OBJS = avi.o

INCDIR = 
CFLAGS = -O2 -Wall -funroll-loops
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS = 
LIBS = -lpspgu -lpspgum -lpspaudio -lpspaudiocodec -lpspmpeg -lpspjpeg -lpspmpegbase -lpspmp3 -lpsppower 

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = MPEG1 PLAYER FOR 6.60CFW & ARK4

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

LIBS += -lpsphprm_driver
