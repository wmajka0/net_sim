cmake_minimum_required(VERSION 3.28)
project(MOJEZ_NET_SIM)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

set(SOURCE_FILES
        src/package.cpp
        src/storage_types.cpp
)

set(SOURCES_FILES_TESTS
        test/test.cpp

)
if(EXISTS ${PROJECT_SOURCE_DIR}/test)
    set(EXEC_TEST ${PROJECT_ID}__test)
    add_executable(${EXEC_TEST} ${SOURCE_FILES} ${SOURCES_FILES_TESTS} test/main_gtest.cpp)

    # == Uwzględnij pliki frameworku Google Testing Framework (GTF) ==

    # Przyjmij, że główny katalog z plikami frameworku GTF znajduje się
    # dwa katalogi wyżej względem katalogu projektu.
    set(GTEST_ROOT ../googletest-master)

    # Dodaj katalogi z plikami nagłówkowymi GTF.
    target_include_directories(${EXEC_TEST} PUBLIC
            ${GTEST_ROOT}/googlemock/include
            ${GTEST_ROOT}/googletest/include
    )

    add_subdirectory(${GTEST_ROOT} googletest-master)

    # Dołącz bibliotekę Google Mock.
    target_link_libraries(${EXEC_TEST} gmock)
endif()


add_executable(MOJEZ_NET_SIM ${SOURCE_FILES} main.cpp)
