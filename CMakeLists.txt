cmake_minimum_required(VERSION 3.16)

project(FaaliyetReporter LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")

message("MSVC_TOOLSET_VERSION   " ${MSVC_TOOLSET_VERSION})
message("CMAKE_CXX_COMPILER_VERSION   " ${CMAKE_CXX_COMPILER_VERSION})

find_package(Qt6 6.2 COMPONENTS Core Gui Xml Widgets PrintSupport REQUIRED)


set(bsoncxx_DIR C:/Mongo/mongo-msvc2022x64-noBoost/lib/cmake/bsoncxx-3.7.0)
set(mongocxx_DIR C:/Mongo/mongo-msvc2022x64-noBoost/lib/cmake/mongocxx-3.7.0)


include_directories("${CMAKE_SOURCE_DIR}")
include_directories("C:/SerikBLDCoreRelease/MSVC2022x64noBoost/include/bilgiEdinme")


file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE FORMS ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)

set(PROJECT_SOURCES
    ${HEADERS}
    ${SOURCES}
    ${FORMS}
)

add_executable( FaaliyetReporter ${PROJECT_SOURCES} )


target_include_directories(FaaliyetReporter PUBLIC "C:/Mongo/mongo-msvc2022x64-noBoost/include")
target_link_directories(FaaliyetReporter PUBLIC "C:/Mongo/mongo-msvc2022x64-noBoost/lib")


target_include_directories(FaaliyetReporter PUBLIC "C:/SerikBLDCoreRelease/MSVC2022x64noBoost/include")
target_link_directories(FaaliyetReporter PUBLIC "C:/SerikBLDCoreRelease/MSVC2022x64noBoost/lib")

find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)
include_directories(${LIBMONGOCXX_INCLUDE_DIR})
include_directories(${LIBBSONCXX_INCLUDE_DIR})

target_link_libraries(FaaliyetReporter PRIVATE mongo::bsoncxx_shared)
target_link_libraries(FaaliyetReporter PRIVATE mongo::mongocxx_shared)


#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "C:/Qt/KDReport/msvc2017_64/lib/cmake") #KDReports 1.9
#target_link_directories(FaaliyetReporter PUBLIC "C:/Qt/KDReport/msvc2017_64/lib")#KDReports 1.9
#target_include_directories(FaaliyetReporter PUBLIC "C:/Qt/KDReport/msvc2017_64/include")#KDReports 1.9


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "C:/KDReport/lib/cmake")
# find_package(KDReports-qt6 REQUIRED)
target_link_directories(FaaliyetReporter PUBLIC "C:/KDReport/lib")
target_include_directories(FaaliyetReporter PUBLIC "C:/KDReport/include")






target_link_libraries(FaaliyetReporter PRIVATE Qt::Core Qt::Gui Qt::Xml Qt::Widgets Qt::PrintSupport SerikBLDCore KDReports-qt6)

#target_compile_options( FaaliyetReporter PRIVATE -fexceptions )

install(TARGETS FaaliyetReporter
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(FaaliyetReporter)
endif()
