@echo off
echo Compiling...
g++ tmp.cpp -fopenmp ftcp.cpp foldsToEdges.cpp loopToFolds.cpp -O3 -o tmp.exe
if %errorlevel% neq 0 exit /b %errorlevel%
echo Build successful.