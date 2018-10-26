all:  WelcomeMessager.dll clean

WelcomeMessager.dll:CSampleCredential.obj CSampleProvider.obj helpers.obj Dll.obj guid.obj resources.res
  link CSampleCredential.obj CSampleProvider.obj helpers.obj Dll.obj guid.obj resources.res /def:WelcomeMessager.def advapi32.lib Shlwapi.lib Secur32.lib Credui.lib Ole32.lib User32.lib

CSampleCredential.obj:CSampleCredential.cpp
  $(CC) /c CSampleCredential.cpp /Fo:CSampleCredential.obj
CSampleProvider.obj:CSampleProvider.cpp
  $(CC) /c CSampleProvider.cpp /Fo:CSampleProvider.obj

helpers.obj:helpers.cpp
  $(CC) /c helpers.cpp /Fo:helpers.obj

Dll.obj:Dll.cpp
  $(CC) /c Dll.cpp /Fo:Dll.obj

guid.obj:guid.cpp
  $(CC) /c guid.cpp /Fo:guid.obj

resources.res:resources.rc
  rc /fo resources.res resources.rc
 
clean:
  -del CSampleCredential.obj CSampleProvider.obj helpers.obj Dll.obj guid.obj resources.res