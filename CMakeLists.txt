cmake_minimum_required(VERSION 3.10)
project(vlak LANGUAGES CXX VERSION 1.4)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")

include_directories(
    .
    packet/include
    packet/lossless/include
    packet/lossless/srcvlak
)

file(GLOB WAVELETSRC
    "packet/src/*.cpp"
    "packet/lossless/src/*.cpp"
    "packet/lossless/srcvlak/*.cpp"
)

# Object lists
set(COMMONSRC
    File.cpp
    cblockinfo.cpp
    csimplecompressedframe.cpp
    cblock.cpp
    RiceCoder/cricecoder.cpp
    RiceCoder/cbitstream.cpp
    WaveFile.cpp
    BlocksProviderFromWaveReader.cpp
    streamops.cpp
    ichannelmanipulator.cpp
    icompressedframe.cpp
    cfircompressedframe.cpp
    clpcriceframe.cpp
    clpcpredictor.cpp
    ipredictor.cpp
    cfftcompressedframe.cpp
    cfftpredictor.cpp
    fft.cpp
)

set(TESTINCSRC
    testincludes.cpp
    ${COMMONSRC}
    ${WAVELETSRC}
)

set(TESTMAINSRC
    testmain.cpp
    ${COMMONSRC}
    BitVector.cpp
    ${WAVELETSRC}
)

set(WAVDIFFSRC
    wavdiff.cpp
    WaveFile.cpp
    File.cpp
    cblock.cpp
    cblockinfo.cpp
    BlocksProviderFromWaveReader.cpp
)

set(VLAKSRC
    vlak.cpp
    ${COMMONSRC}
    ${WAVELETSRC}
)

add_executable(vlak ${VLAKSRC})
add_executable(testlpc testlpc.cpp ${COMMONSRC} ${WAVELETSRC})
add_executable(testincludes ${TESTINCSRC})
add_executable(testmain ${TESTMAINSRC})
add_executable(wavdiff ${WAVDIFFSRC})

add_custom_target(tests DEPENDS testincludes testmain testlpc)
