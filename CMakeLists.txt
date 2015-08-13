project(FileCopyEx3)
cmake_minimum_required(VERSION 2.8)

include(${CMAKE_ROOT}/Modules/CMakeDetermineSystem.cmake)
message("system: ${CMAKE_SYSTEM_NAME}")

#-------------------------------------------------------------------------------

if(NOT DEFINED CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Debug)
endif()

if(NOT DEFINED FAR_VERSION OR FAR_VERSION STREQUAL "")
set(FAR_VERSION Far3)
endif()

if(NOT DEFINED CONF)
set(CONF "x86")
endif()

if(CONF STREQUAL "x86")
set(PLATFORM "Win32")
else()
set(PLATFORM "x64")
endif()

set(OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${FAR_VERSION}_${CONF}/Plugins/FileCopyEx3)

#-------------------------------------------------------------------------------

if(MSVC)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWIN32 -D_WINDOWS -D_WINDLL -D_USRDL /EHsc /MP2 -D_SCL_SECURE_NO_WARNINGS -D_UNICODE -DUNICODE -D_WINXP -D_HAS_EXCEPTIONS=0"
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWIN32 -D_WINDOWS -D_WINDLL -D_USRDL /MP2 -D_SCL_SECURE_NO_WARNINGS -D_UNICODE -DUNICODE -D_WINXP -D_HAS_EXCEPTIONS=0"
)

set(FLAGS_DEBUG "-D_DEBUG /Gm- /MTd /GS"
)

set(FLAGS_RELEASE "-U_DEBUG -DNDEBUG /GL /Gm- /MT -Ox -Ob1 -Oi -Os -GF -GS- -Gy /fp:fast"
)

else()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -D_WINDOWS -D_WINDLL -D_USRDL -D_SCL_SECURE_NO_WARNINGS -D_UNICODE -DUNICODE"
)
endif()


if(PLATFORM STREQUAL "x64")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWIN64"
)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "${FLAGS_DEBUG}")

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${FLAGS_RELEASE}")

set(CMAKE_LINK_FLAGS
  "/DEBUG
  /MANIFEST:NO
  /TLBID:1 /DYNAMICBASE /SUBSYSTEM:WINDOWS /NOLOGO
  /MAP /OPT:REF
  /INCREMENTAL:NO
"
)

set(CMAKE_EXTRA_LINK_FLAGS "${CMAKE_EXTRA_LINK_FLAGS} /INCREMENTAL:NO /OPT:REF /OPT:ICF"
)

#-------------------------------------------------------

include_directories(
#  src
#  src/Framework
#  src/AsyncFramework
)

#-------------------------------------------------------------------------------
# target FileCopyEx3

set(FILECOPYEX_SOURCES
  src/ui.cpp
  src/tools.cpp
  src/taskbarIcon.cpp
  src/common.cpp
  src/FarSettings.cpp
  src/FarProgress.cpp
  src/FarPayload.cpp
  src/FarNode.cpp
  src/FarMenu.cpp
  src/FarMacro.cpp
  src/FarPlugin.cpp
  src/FileCopyEx.cpp
  src/EngineTools.cpp
  src/Engine.cpp
  src/CopyProgress.cpp
  src/Framework/StrUtils.cpp
  src/Framework/ObjString.cpp
  src/Framework/FileUtils.cpp
  src/Framework/LocaleUtils.cpp
  src/Framework/Properties.cpp
  src/Framework/StringParent.cpp
  src/Framework/Payload.cpp
  src/Framework/ObjectManager.cpp
  src/Framework/Node.cpp
  src/Framework/FileNameStoreEnum.cpp
  src/Framework/DescList.cpp
  src/Framework/FrameworkUtils.cpp

  src/AsyncFramework/WaitableEvent.cpp
  src/AsyncFramework/stdafx.cpp
  src/AsyncFramework/CriticalSection.cpp
  src/AsyncFramework/JobScheduler.cpp
  src/AsyncFramework/Thread.cpp
  src/AsyncFramework/JobBase.cpp

  FileCopyEx3.rc
)

if(MSVC)

set(FILECOPYEX_SOURCES ${FILECOPYEX_SOURCES}
  src/vc10wrapper.cpp
  plugin.def
)

if(PLATFORM STREQUAL "Win32")

FIND_PROGRAM (MASM_EXECUTABLE ml)
# message("masm: ${MASM_EXECUTABLE}")
set(ASM_OBJECTS)
foreach(src vc10)
  set(ASM_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/${src}.asm)
  set(ASM_OBJECT ${CMAKE_CURRENT_BINARY_DIR}/${src}.obj)
  set(ASM_OBJECTS ${ASM_OBJECTS} ${ASM_OBJECT})
  add_custom_command(
    OUTPUT ${ASM_OBJECT}
    COMMAND ${MASM_EXECUTABLE}
    ARGS   /c /Fo ${ASM_OBJECT} ${ASM_SOURCE}
    DEPENDS ${ASM_SOURCE}
  )
endforeach(src)

set(FILECOPYEX_SOURCES ${FILECOPYEX_SOURCES}
  src/vc10wrapper.cpp
  ${ASM_OBJECTS}
)

endif()

else()
set(FILECOPYEX_SOURCES ${FILECOPYEX_SOURCES}
  plugin.gcc.def
)
endif()

#-------------------------------------------------------------------------------

set(FILECOPYEX_LIBRARIES
  kernel32.lib
  user32.lib
  advapi32.lib
  Mpr.lib
)

#if(MINGW)
#set(FILECOPYEX_LIBRARIES "${FILECOPYEX_LIBRARIES} msvcr100"
#)
#endif()

if(MSVC)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")

set(FILECOPYEX_LIBRARIES ${FILECOPYEX_LIBRARIES}
  libcmtd.lib
  libcpmtd.lib
)

else()

set(FILECOPYEX_LIBRARIES ${FILECOPYEX_LIBRARIES}
  libcmt.lib
  libcpmt.lib
)

endif()

endif()

add_library(FileCopyEx3 SHARED ${FILECOPYEX_SOURCES})

set_target_properties(FileCopyEx3
  PROPERTIES
  COMPILE_FLAGS "${CMAKE_CXX_FLAGS}"
  LINK_FLAGS ${CMAKE_LINK_FLAGS}
)

#-------------------------------------------------------------------------------

string(REPLACE "/" "\\" BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")
message("BUILD_DIR: ${BUILD_DIR}")

target_link_libraries(FileCopyEx3 ${FILECOPYEX_LIBRARIES})

string(REPLACE "/" "\\" OUTPUT_DIR ${OUTPUT_DIR})
string(REPLACE "/" "\\" DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")

if(MSVC)

add_custom_command(TARGET FileCopyEx3
  POST_BUILD
  COMMAND if not exist "${OUTPUT_DIR}" ( mkdir "${OUTPUT_DIR}" )
  COMMAND cp -f -R --target-directory=${OUTPUT_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/data/*
  COMMAND cp -f --target-directory=${OUTPUT_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/changelog.txt
  COMMAND cp -f --target-directory=${OUTPUT_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt
  COMMAND cp -f --target-directory=${OUTPUT_DIR} FileCopyEx3.dll
  COMMAND if exist FileCopyEx3.pdb cp -f --target-directory=${OUTPUT_DIR} FileCopyEx3.pdb
  COMMAND if exist FileCopyEx3.map cp -f --target-directory=${OUTPUT_DIR} FileCopyEx3.map
  WORKING_DIRECTORY ${BUILD_DIR}
  VERBATIM
)

else()

add_custom_command(TARGET FileCopyEx3
  POST_BUILD
  COMMAND if not exist "${OUTPUT_DIR}" ( mkdir "${OUTPUT_DIR}" )
  COMMAND if not exist "${OUTPUT_DIR}\\doc" ( mkdir "${OUTPUT_DIR}\\doc" )
  COMMAND if not exist "${OUTPUT_DIR}\\resource" ( mkdir "${OUTPUT_DIR}\\resource" )
  COMMAND copy /Y FileCopyEx3.dll ${OUTPUT_DIR}\\FileCopyEx3.dll
  COMMAND copy /Y ${DATA_DIR}\\*.hlf ${OUTPUT_DIR}
  COMMAND copy /Y ${DATA_DIR}\\*.lng ${OUTPUT_DIR}
  COMMAND copy /Y ${DATA_DIR}\\*.diz ${OUTPUT_DIR}
#  COMMAND copy /Y ${DATA_DIR}\\doc\\*.* ${OUTPUT_DIR}\\doc
  COMMAND copy /Y ${DATA_DIR}\\resource\\*.* ${OUTPUT_DIR}\\resource
  WORKING_DIRECTORY ${BUILD_DIR}
  VERBATIM
)

endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")

add_custom_command(TARGET FileCopyEx3
  POST_BUILD
  DEPENDS ${BUILD_DIR}\\FileCopyEx3.dll
  COMMAND make_dist.cmd ${FAR_VERSION} ${CONF}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

endif()

