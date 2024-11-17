#!/bin/bash

# Compilation command
g++ -std=c++11 main.cpp -o snake -L/opt/homebrew/lib -I/opt/homebrew/include -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo 
    ./snake
else
    echo "Compilation failed."
fi

# give file permissions enter this command in directory terminal
# chmod +x build.sh 