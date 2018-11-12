## XONITIX

Xonitix is an action-packed game for the computer terminal
where you gain points by claiming space on the line while
avoiding the moving dots. Think you have what it takes?

![xonitix gameplay](https://img.itch.zone/aW1nLzE2MjczNzYuZ2lm/original/zLnXU%2B.gif)

Xonitix was created for the [Terminal Jam 2018](https://itch.io/jam/terminal-jam/) on [itch.io](https://itch.io).


#### Playing

Use the left/right arrow keys to move, and the down key to stop.
Press the spacebar to create a barrier where you stand, but careful not
to hit the moving dots. Level up by filling over 50% of the line!

This is just the source code, meant for people interested in checking
out how it was made and maybe tweaking it or posting patches.

For the Xonitix project page, including screenshots, updates and
official binary builds for Windows, Linux and OSX that you can just
download and play, see https://wildgaru.itch.io/xonitix


#### Building

Xonitix has no external dependencies other than C++ standard libraries.

If you are on a Windows environment, fire up Visual Studio with C++ support
(the Community Edition is free) and load the `xonitix.cpp` file, then press
F7 to Build.

On Linux or OSX, you will need a C++ compiler - gcc/g++ will do just fine.
To build, simply go the project directory where the `xonitix.cpp`
file is and type:

    g++ -Wall -g -std=c++11 xonitix.cpp -o xonitix


#### Contributing

Bug reports and patches are welcome!

If you have [clang extra tools](https://clang.llvm.org/extra/clang-tidy/),
this repository includes a .clang-format and a .clang-tidy file which
you can use to format and tidy your patch according to this project's
C++ coding style:

    clang-format -i -style=file xonitix.cpp
    clang-tidy xonitix.cpp

If you don't have clang tools in your environment, that's ok. Just please
try and follow the project's coding standards as best as you can :D


#### License & Copyright

Copyright © 2018 Breno G. de Olivera.
Released under the MIT License.
