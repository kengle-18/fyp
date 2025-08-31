# fyp

in python
docker build -t python-app .
docker run --rm python-app  

in scala
sbt clean compile assembly

docker build -t scala-app . 
docker run --rm scala-app
docker run -it --rm scala-app
docker run -it scala-app

in cpp
docker build -t cpp-app .
docker run --rm cpp-app

<!-- build in current dir of the cmake under /build-->
cmake -S . -B build
Download takes 300s

No diff for mac/linux 
cmake --build build

For windows
cmake --build build --config Release

fetch wghat
cmake --build build --target help

<!-- Build exe for later -->
in fyp\cpp\build\_deps\grpc-build
cmake ../grpc-src -G "Visual Studio 17 2022" -A x64 -DgRPC_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=ON -DCMAKE_CXX_STANDARD=17

cmake ../grpc-src -G "Visual Studio 17 2022" -A x64 -DgRPC_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=ON -DCMAKE_CXX_STANDARD=17
-DABSL_BUILD_TESTING=OFF
-DABSL_BUILD_TEST_HELPERS=OFF -DABSL_USE_GOOGLETEST_HEAD=OFF

in the grpc-build --> 
cmake --build . --target grpc_cpp_plugin --config Release

<!-- Try for protoc?-->
cd C:\Users\user\Downloads\fyp\cpp\build\_deps\protobuf-build

cmake ../protobuf-src -G "Visual Studio 17 2022" -A x64 -Dprotobuf_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=ON -DCMAKE_CXX_STANDARD=17 -DABSL_BUILD_TESTING=OFF -DABSL_BUILD_TEST_HELPERS=OFF
-DABSL_USE_GOOGLETEST_HEAD=OFF

cmake ../protobuf-src `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -Dprotobuf_BUILD_TESTS=OFF `
  -DBUILD_SHARED_LIBS=ON `
  -DCMAKE_CXX_STANDARD=17 `
  -DABSL_BUILD_TESTING=OFF `
  -DABSL_BUILD_TEST_HELPERS=OFF `
  -DABSL_USE_GOOGLETEST_HEAD=OFF `
  

cmake ../protobuf-src `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -Dprotobuf_BUILD_TESTS=OFF `
  -DBUILD_SHARED_LIBS=ON `
  -DCMAKE_CXX_STANDARD=17 `
  -DABSL_BUILD_TESTING=OFF `
  -DBUILD_TESTING=OFF `
  -DABSL_BUILD_TEST_HELPERS=OFF

in protobuf-build
cmake --build . --target protoc --config Release

<!-- Need to test -->
in cpp
<!-- Create the grpc -->
cmake --build build/_deps/grpc-build --config Release --target grpc_cpp_plugin
<!-- Create the protoc -->
cmake --build build/_deps/protobuf-build --config Release --target protoc
<!--  -->

<!-- For build -->
<!-- Cross-platform for firsdt build--> 
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

<!-- Then final -->
cmake --build build --config Release

cmake --build build/_deps/grpc-build --config Release --target grpc_cpp_plugin

<!-- Testing for protobuf -->
# 1. Clean build directory (recommended)
rm -r build

# 2. Configure for windows
cmake -S . -B build -G "Visual Studio 17 2022" -A x64

# 3. Build (Release or Debug)
cmake --build build --config Release

<!-- Full manaul command -->
Need to add path
$env:PATH += ";C:\Users\user\Downloads\fyp\cpp\build\_deps\protobuf-build\bin\Release"

Need to move all dll needed inclduing C:\Users\user\Downloads\fyp\cpp\build\_deps\grpc-build\Release\grpc_cpp_plugin.exe

File Type: EXECUTABLE IMAGE

  Image has the following dependencies:

    grpc_plugin_support.dll
    libprotoc.dll
    libprotobuf.dll
    abseil_dll.dll
    MSVCP140.dll
    VCRUNTIME140.dll
    VCRUNTIME140_1.dll
    api-ms-win-crt-runtime-l1-1-0.dll
    api-ms-win-crt-heap-l1-1-0.dll
    api-ms-win-crt-math-l1-1-0.dll
    api-ms-win-crt-stdio-l1-1-0.dll
    api-ms-win-crt-locale-l1-1-0.dll
    KERNEL32.dll

  Summary

from C:\Users\user\Downloads\fyp\cpp\build\_deps\grpc-build\Release to C:\Users\user\Downloads\fyp\cpp\build\_deps\grpc-build\bin\Release

$env:PATH = "C:\Users\user\Downloads\fyp\cpp\build\_deps\grpc-build\bin\Release;" + $env:PATH

you are prepending that folder to the existing PATH.
$env:PATH += ";C:\Users\user\Downloads\fyp\cpp\build\_deps\abseil-cpp-build\absl"

Check paths

C:\Users\user\Downloads\fyp\cpp\build\_deps\protobuf-build\Release\protoc.exe `
  -I C:\Users\user\Downloads\fyp\scala\src\main\protobuf `
  --cpp_out=C:\Users\user\Downloads\fyp\cpp\build\generated `
  --grpc_out=C:\Users\user\Downloads\fyp\cpp\build\generated `
  --plugin=protoc-gen-grpc=C:\Users\user\Downloads\fyp\cpp\build\_deps\grpc-build\Release\grpc_cpp_plugin.exe `
  C:\Users\user\Downloads\fyp\scala\src\main\protobuf\base.proto


Fomrt
protoc -I path/to/protos --grpc_out=output_dir --plugin=protoc-gen-grpc=grpc_cpp_plugin your.proto

Shortcut full
protoc -I C:\Users\user\Downloads\fyp\scala\src\main\protobuf `
       --cpp_out=C:\Users\user\Downloads\fyp\cpp\build\generated `
       --grpc_out=C:\Users\user\Downloads\fyp\cpp\build\generated `
       --plugin=protoc-gen-grpc=C:\Users\user\Downloads\fyp\cpp\build\_deps\grpc-build\Release\grpc_cpp_plugin.exe `
       C:\Users\user\Downloads\fyp\scala\src\main\protobuf\base.proto

C:\Users\user\Downloads\vcpkg\installed\x64-windows\tools\protobuf\protoc.exe -I C:\Users\user\Downloads\fyp\scala\src\main\protobuf --cpp_out=C:\Users\user\Downloads\fyp\cpp\build\generated --grpc_out=C:\Users\user\Downloads\fyp\cpp\build\generated --plugin=protoc-gen-grpc=C:\Users\user\Downloads\vcpkg\installed\x64-windows\tools\grpc\grpc_cpp_plugin.exe C:\Users\user\Downloads\fyp\scala\src\main\protobuf\base.proto

C:\Users\user\Downloads\vcpkg\installed\x64-windows\tools\protobuf\protoc.exe -I C:\Users\user\Downloads\fyp\scala\src\main\protobuf --cpp_out=C:\Users\user\Downloads\fyp\cpp\build\generated --grpc_out=C:\Users\user\Downloads\fyp\cpp\build\generated --plugin=protoc-gen-grpc=C:\Users\user\Downloads\fyp\cpp\build\_deps\grpc-build\Release\grpc_cpp_plugin.exe C:\Users\user\Downloads\fyp\scala\src\main\protobuf\base.proto

C:\Users\user\Downloads\fyp\cpp\build\_deps\protobuf-build\protoc.exe -I C:\Users\user\Downloads\fyp\scala\src\main\protobuf --cpp_out=C:\Users\user\Downloads\fyp\cpp\build\generated --grpc_out=C:\Users\user\Downloads\fyp\cpp\build\generated --plugin=protoc-gen-grpc=C:\Users\user\Downloads\fyp\cpp\build\_deps\grpc-build\Release\grpc_cpp_plugin.exe C:\Users\user\Downloads\fyp\scala\src\main\protobuf\base.proto


C:\Users\user\Downloads\fyp\cpp\build\_deps\grpc-build\third_party\protobuf\Release\protoc.exe -I C:\Users\user\Downloads\fyp\scala\src\main\protobuf --cpp_out=C:\Users\user\Downloads\fyp\cpp\build\generated --grpc_out=C:\Users\user\Downloads\fyp\cpp\build\generated --plugin=protoc-gen-grpc=C:\Users\user\Downloads\fyp\cpp\build\_deps\grpc-build\Release\grpc_cpp_plugin.exe C:\Users\user\Downloads\fyp\scala\src\main\protobuf\base.proto

check paths in system
$env:PATH -split ';'  


<!-- Final -->
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 

cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
    -Dprotobuf_BUILD_TESTS=OFF `
    -DgRPC_BUILD_TESTS=OFF `
    -DABSL_BUILD_TESTING=OFF `
    -DABSL_BUILD_TEST_HELPERS=OFF `
    -Dprotobuf_BUILD_PROTOC_BINARIES=ON `
    -Dprotobuf_BUILD_SHARED_LIBS=OFF `
    -DgRPC_BUILD_SHARED_LIBS=OFF `
    -DBUILD_SHARED_LIBS=OFF

then
in 
cd :\Users\user\Downloads\fyp\cpp\build\_deps\protobuf-build
cmake ../protobuf-src `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -Dprotobuf_BUILD_TESTS=OFF `
  -DBUILD_SHARED_LIBS=OFF `
  -DCMAKE_CXX_STANDARD=17 `
  -DABSL_BUILD_TESTING=OFF `
  -DBUILD_TESTING=OFF `
 -DABSL_BUILD_TEST_HELPERS=OFF

cmake -S "..\protobuf-src" -B . `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -Dprotobuf_BUILD_TESTS=OFF `
  -DBUILD_SHARED_LIBS=OFF `
  -DCMAKE_CXX_STANDARD=17 `
  -DABSL_BUILD_TESTING=OFF `
  -DBUILD_TESTING=OFF `
  -DABSL_BUILD_TEST_HELPERS=OFF `
  -Dprotobuf_BUILD_PROTOC_BINARIES=ON


cmake --build . --config Release --target protoc

go back to cpp
cmake --build build --config Release



<!-- build wihout cache  -->
docker compose build --no-cache
<!-- Starts container -->
remains running 
docker compose up --build
<!-- shut down -->
docker compose down

<!-- format -->
sbt scalafmtAll

<!-- check EOF LF or CRLF -->
git ls-files --eol


What’s META-INF/services?

In Java (and also in Scala), ServiceLoader is a way to discover implementations of an interface at runtime without hardcoding them.

Each file in META-INF/services is named after an interface or abstract class and contains the fully qualified class names of implementations.

gRPC uses this internally to discover available channel providers, transport factories, etc.

Example: