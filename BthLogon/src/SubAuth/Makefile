FileName=subauth
LIBS=Bthprops.lib advapi32.lib
all: $(FileName).dll clean
  
$(FileName).obj:
  $(CC) /c $(FileName).cpp

$(FileName).dll: $(FileName).obj
  Cl $(FileName).obj /link -out:$(FileName).dll -def:$(FileName).def $(LIBS)
  
.PHONY: clean
clean:
    -del $(FileName).obj $(FileName).lib $(FileName).exp
