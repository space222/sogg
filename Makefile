PROJDIRS := .
SRCFILES := $(shell find . -name "*.cpp")
OBJFILES := $(patsubst %.cpp,%.o, $(SRCFILES))
DEPFILES := $(patsubst %.o,%.d,$(OBJFILES))
CPPFLAGS := -I./include -O2 -std=c++17
LIBS := 

.PHONY : all clean

all: a.out

a.out : $(OBJFILES)
	@echo "Linking..."
	g++ -o a.out $(OBJFILES) $(LIBS)
	@echo "done."

%.o : %.cpp Makefile
	g++ $(CPPFLAGS) -MMD -MP -c $< -o $@

clean:
	@echo "Removing build files..."
	@rm -f *.o
	@rm -f *.d
	@echo "done."

-include $(DEPFILES)
