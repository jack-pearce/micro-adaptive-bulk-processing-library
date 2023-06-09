cmake_minimum_required(VERSION 3.10.2)

project(MABPL)
enable_testing()
add_test(NAME MemoryCheck COMMAND valgrind --tool=memcheck --leak-check=yes ./out/build/MABPL)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -O3 -march=native -std=c++17")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -O3") # Required for folly (no -m flags)

set(benchmark_DIR "libs/benchmark/build")
find_package(benchmark REQUIRED)

link_directories(/home/jack/CLionProjects/micro-adaptive-bulk-processing-library/libs/papi/src/install/lib)

#set(FOLLY_DIR "libs//folly")
#find_package(folly REQUIRED)
#find_package(gflags CONFIG REQUIRED)

find_package(absl REQUIRED)

add_subdirectory(libs/robin-map)

add_subdirectory(libs/hopscotch-map)

add_executable(${PROJECT_NAME}
        src/data_generation/dataGenerators.cpp
        src/data_generation/dataFiles.cpp
        src/data_generation/machineConfiguration.cpp
        src/library/operators/select.cpp
        src/library/utilities/papi.cpp
        src/library/utilities/systemInformation.cpp
        src/time_benchmarking/selectTimeBenchmark.cpp
        src/time_benchmarking/timeBenchmarkHelpers.cpp
        src/main.cpp
        src/utilities/dataHelpers.cpp
        src/utilities/papiHelpers.cpp
        src/cycles_benchmarking/selectCyclesBenchmark.cpp
        src/library/operators/groupBy.cpp
        src/cycles_benchmarking/groupByCyclesBenchmark.cpp src/library/mabpl.h)

target_link_libraries(${PROJECT_NAME} benchmark::benchmark)

target_link_libraries(${PROJECT_NAME} papi)

#target_link_libraries(${PROJECT_NAME} Folly::folly)

target_link_libraries(${PROJECT_NAME} absl::base absl::hash absl::flat_hash_map)

target_link_libraries(${PROJECT_NAME} tsl::robin_map)




