This program arms the Thorlabs camera CS2100M-USB and waits for the camera to receive an external trigger. When a trigger is received, the image is saved in a lossless TIF image file. After the trigger, the program returns to the "wait-state" and waits for additional external triggers. The process runs indefinitely unless terminated by the user.

The code can be modified to suit different applications, settings, and triggering modes (all detailed in the C API from Thorlabs).



Required software: (This build will only work with Windows 64 bit)

1) CMake https://cmake.org/download/
Download the windows 64 bit version

2) ThorCam from Thorlabs (version 3.1.0 or above)
You must add this folder to your PATH environmental variable after installation
"C:\Program Files\Thorlabs\Scientific Imaging\Scientific Camera Support\SDK\SDK\Native Compact Scientific Camera Toolkit\bin\Native_64_lib"

3) MinGW-w64 64-bit C compiler.
You can download here: https://sourceforge.net/projects/mingw-w64/
In the setup settings, select x86_64 under architecture. Leave the default values for all other settings.
If you use a different compiler, you will have to modify the cmake and make commands
 (number 3 and 4 in the list below). I have not tested it with any other compiler.

4) ImageMagick 7
Download here: https://www.imagemagick.org/script/download.php
Download version 7.0.8 for windows 64 bit. Download the dll version
If there is a newer 7.x.x version, you will have to modify the CMakeLists.txt file with the correct install directory name







To build and run program:

1) Create build directory "ArmCamera-build" next to the source directory (not inside source directory).

2) cd to "ArmCamera-build"

3) execute command "cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=RELEASE ../ArmCamera"

4) execute command "mingw32-make"

6) execute "ArmCamera.exe 10000"
second argument is the exposure time (in usec) for software and standard hardware trigger modes. For bulb mode, also type in a number, even though the exposure time is determined by the external pulse duration of your trigger.


If you modify ArmCamera.c, you will have to rebuild the program for the changes to take effect. To run the executable in another windows 64 machine, you need to download ThorCam (also add the Native_64_lib folder to PATH) and ImageMagick. No need to install any of the C stuff to just run the executable.


Made by danielmtzz
