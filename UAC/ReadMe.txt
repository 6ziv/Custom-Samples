Replacement of the default UAC prompt consent.exe

Supports:
    Secure desktop
    Exe And Dll

Installation:
    Compile it and use your own test cert to sign it!
    Then rename the generated exe to consent.exe and replcae Windows\System32\consent.exe with it
    Remember to copy the external dlls together into Windows\System32. Including:Qt5Gui.dll,Qt5Widgets.dll,Qt5Core.dll.

Though it checks the signature of an exe file, it may not be so safe as the original consent.exe. Have fun with it, but do not use it when you need security!

Last, Menhera is so lovely!!(Well, the images should not be considered part of the project, but files for testing: they are not included in the WTFPL and be sure to piurchase for them before you use).