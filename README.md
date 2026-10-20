C++ based game with custom falling sand simulation engine inspired by noita.

1. `mkdir build`
2. `cd build`
3. `cmake ..`
4. make (or `cmake --build .` on Windows)
5. `./FallingSandIter1` (or `Debug\FallingSandIter1.exe` on Windows)

NOTE on Linux, CMakeLists.txt can generate the compile_commands.json file which allows nvim to 
properly act as a linter / debugger. Windows can't do this using 
