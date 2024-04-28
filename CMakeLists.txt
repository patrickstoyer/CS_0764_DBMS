cmake_minimum_required(VERSION 3.27)
project(Barebone)

set(CMAKE_CXX_STANDARD 17)

add_executable(Barebone test.cpp
        Scan.cpp
        Sort.cpp
        Record.cpp
        Iterator.cpp
        InputBuffer.cpp
        Filter.cpp
        defs.cpp
        PriorityQueue.cpp
        PriorityQueue.h
        InputStream.h)
