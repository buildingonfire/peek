@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 >/dev/null 2>&1
echo VCVARS DONE
"C:\Program Files\CMake\bin\cmake.exe" -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release . 2>&1
echo CMAKE DONE
cd build
nmake 2>&1
echo NMAKE DONE
