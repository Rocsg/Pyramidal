cmake_minimum_required(VERSION 2.8)
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()
 

PROJECT(Pyramidal) 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
 
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
 
file(GLOB UI_FILES *.ui)
file(GLOB QT_WRAP *.h)
file(GLOB CXX_FILES *.cxx *.cpp)

if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()


if(${VTK_VERSION} VERSION_GREATER "6" AND VTK_QT_VERSION VERSION_GREATER "4")
  # Instruct CMake to run moc automatically when needed.
  set(CMAKE_AUTOMOC ON)
  find_package(Qt5Widgets REQUIRED QUIET)
  qt5_wrap_ui(UISrcs ${UI_FILES} )

  # CMAKE_AUTOMOC in ON so the MOC headers will be automatically wrapped.
  add_executable(Pyramidal MACOSX_BUNDLE ${CXX_FILES} ${UISrcs} ${QT_WRAP})
  qt5_use_modules(Pyramidal Core Gui)
  target_link_libraries(Pyramidal ${VTK_LIBRARIES})
else()
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
  QT4_WRAP_UI(UISrcs ${UI_FILES})
  QT4_WRAP_CPP(MOCSrcs ${QT_WRAP})
  add_executable(Pyramidal MACOSX_BUNDLE ${CXX_FILES} ${UISrcs} ${MOCSrcs})
 
  if(VTK_LIBRARIES)
    if(${VTK_VERSION} VERSION_LESS "6")
      target_link_libraries(Pyramidal ${VTK_LIBRARIES} QVTK)
    else()
      target_link_libraries(Pyramidal ${VTK_LIBRARIES})
    endif()
  else()
    target_link_libraries(Pyramidal vtkHybrid QVTK vtkViews ${QT_LIBRARIES})
  endif()
endif()


