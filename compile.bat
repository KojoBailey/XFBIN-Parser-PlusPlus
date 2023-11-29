@ECHO OFF

echo Compiling...
g++ src/main.cpp src/functions.cpp src/templates/messageInfo.cpp src/templates/MainModeParam.cpp src/templates/GuideCharParam.cpp src/templates/Template.cpp -o main.exe -std=c++20 -static -static-libgcc -static-libstdc++