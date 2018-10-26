all:  OpenDesktop.dll clean

OpenDesktop.dll:OpenDesktop.obj
  link OpenDesktop.obj /def:OpenDesktop.def User32.lib

OpenDesktop.obj:OpenDesktop.cpp
  $(CC) /c OpenDesktop.cpp /Fo:OpenDesktop.obj

clean:
  -del OpenDesktop.obj