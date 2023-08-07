# pewpew

"pewpew" is an exciting little game developed using the Raylib, specifically designed for the C programming language. This project was a labor of passion, combining my love for programming. It is also compiled for wasm and ported to the WEB as a platform.

# References:  

https://www.raylib.com   

https://www.youtube.com/c/TsodingDaily

# Dependencies: 

C++  
raylib  
python  

# Installation
To install raylib:
```bash
$ sudo apt install libasound2-dev mesa-common-dev libx11-dev libxrandr-dev libxi-dev xorg-dev libgl1-mesa-dev libglu1-mesa-dev
$ git clone https://github.com/raysan5/raylib
$ cd raylib
$ mkdir build && cd build
$ cmake -DBUILD_SHARED_LIBS=ON ..
$ make up
$ sudo make install
$ sudo cp /usr/local/lib/libraylib.so.450 /usr/lib/
```
NOTE! if you get and error that /usr/local/lib/libraylib.so.450 doesnt exist just go to /usr/local/lib and there should be file libraylib.so.4xx so unsetad of 450 just write whatever number is there. This error occurs because of version diff.

# How to run
If you want to build and run the game:
```bash
$ ./build.sh
$ ./pewpew
```
And if you want to run WEB version:
```bash
$ cd web
$ python3 -m http.server
```
After that open your browser and go to the localhost:8000
