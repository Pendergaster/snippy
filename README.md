# snippy
x11 snipping tool utility, where user can drag area to the screen. This area can be pasted to desired location.
Currently this is WIP project and should not be used. All functions work, but large images which might need multiple property changes and INC loop might fail.
Also more formats needs to be implemented to make copy paste work on applications like libreoffice.

Project uses [Sean Barrets image utilies](https://github.com/nothings/stb/blob/master/stb_image_write.h) to generate png image.
Project also requires Xinerama (most system has this to manage multiple physical screens) 
and Xdbe (window double buffering to reduce flickering) X extensions from target computer.

This is very much utility for my own personal needs

# image
Lighter area of the desktop is selected area
![image1](snippy_ex.png)
