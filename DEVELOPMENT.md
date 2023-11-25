# Build with CMake
This is the command to use to build the program inside `build/` directory. Also, it generates `compile_commands.json` into root project directory so linter can watch include files in `include/` directory:

```shell
cd build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON . && make -j4 && cp compile_commands.json ../ && cd ..
```

# Run the program
An example:

```shell
./build/fitgalgo -h 127.0.0.1 -p 8000
```
