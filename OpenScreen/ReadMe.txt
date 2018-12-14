Putting a HTML (probably with transparent background) onto the screen.

Qt+CEF3 libraries needed for basic functions.
inih & ZipLib also contained for loading webpages and configuration from the disk/registry.
Privexec has been referred to for dropping privileges.

Only one sample provided, as I have not been licensed to use the snow.js project.
It loads standard zip files, with or without a config.ini inside. Tested with 7-zip.
Note that you should prevent frequent I/Os, for it is not so easy for a zip to be read.

For software improvement or more samples, pull requests are welcome.

Format of config.ini:
Only three key names available:
CaptureMouse = [true/false]
Alpha        = [1-100] (when 100, CaptureMouse is ignored)
Delay        = [6- 20] (when x, enter the CEF eventloop every x seconds)

When building, you should copy/extract the matching file(s) from dependencies/x86 or dependencies/x64 to the working directory.

When uninstalling......I am too lazy to do the clean up, so do it yourself! (Maybe I will add this with other functions later?)