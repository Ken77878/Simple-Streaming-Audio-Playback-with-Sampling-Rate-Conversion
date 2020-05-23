#code main
#CMakeLists.txt

cmake_minimum_required (VERSION 3.09)


option(TEST ON)

#solution name
project("simpleStreamingAudioPlayer" C CXX)


## Active C++11
set (CMAKE_CXX_STANDARD 11)

if(WIN32)
	if(!MSVC)
   	message(FATAL_ERROR "Please use Visual Studio as CMAKE_GENERATOR.")
		return()
	endif()
endif()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../externalProjects")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../externalProjects/libogg")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../externalProjects/libvorbis")


# delete ZERO_CHECK project in MSVC
if(WIN32 AND MSVC)
	set(CMAKE_SUPPRESS_REGENERATION TRUE)
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	if(WIN32 AND MSVC)
		set(CMAKE_CONFIGURATION_TYPES "Debug" CACHE STRING "Configs" FORCE)
	else()
		#-fsanitize=address
		set(CMAKE_CXX_FLAGS "-O0 -g -W -Wall ")
		add_definitions(-D_DEBUG)
	endif()
elseif(${CMAKE_BUILD_TYPE} MATCHES "Release")
	if(WIN32 AND MSVC)
		set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "Configs" FORCE)
	else()
		set(CMAKE_CXX_FLAGS "-O2")
	endif()
else()
	message(FATAL_ERROR "add arguments -DCMAKE_BUILD_TYPE=(\"Debug\" or \"Release\") to cmake command")
	return()
endif()


#make nonstatic library on libogg
set(BUILD_SHARED_LIBS  ON)

#character code
if(WIN32 AND MSVC)
	add_definitions(-D_UNICODE)
	if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		set( win32Directory "win32/x64" )
	else( CMAKE_SIZEOF_VOID_P EQUAL 4 )
		set( win32Directory "win32/x86")
	endif()
endif()

#preprocessor
#add_compile_definitions(domething)

#path for output of dll and lib
SET(MY_OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/..")

#current directory
#message("=========${CMAKE_CURRENT_BINARY_DIR}===================\n")
#directory having top CMakeLists.txt
#message("=========${CMAKE_CURRENT_SOURCE_DIR}===================\n")

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	if(WIN32)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${MY_OUTPUT}/debugOutput/${win32Directory}/library")
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${MY_OUTPUT}/debugOutput/${win32Directory}/library")
	else()
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${MY_OUTPUT}/debugOutput/linux/a")
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${MY_OUTPUT}/debugOutput/linux/so")
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${MY_OUTPUT}/debugOutput/linux/so")
	endif()
elseif(${CMAKE_BUILD_TYPE} MATCHES "Release")
	if(WIN32)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${MY_OUTPUT}/releaseOutput/${win32Directory}/library")
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${MY_OUTPUT}/releaseOutput/${win32Directory}/library")
	else()
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${MY_OUTPUT}/releaseOutput/linux/a")
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${MY_OUTPUT}/releaseOutput/linux/so")
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${MY_OUTPUT}/releaseOutput/linux/so")
	endif()
endif()



if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	if(WIN32)
		link_directories("${MY_OUTPUT}/debugOutput/${win32Directory}/library")
		link_directories("${MY_OUTPUT}/debugOutput/${win32Directory}/library")
	else()
		link_directories("${MY_OUTPUT}/debugOutput/linux/a")
		link_directories("${MY_OUTPUT}/debugOutput/linux/so")
	endif()
elseif(${CMAKE_BUILD_TYPE} MATCHES "Release")
	if(WIN32)
		link_directories("${MY_OUTPUT}/releaseOutput/${win32Directory}/library")
		link_directories("${MY_OUTPUT}/releaseOutput/${win32Directory}/library")
	else()
		link_directories("${MY_OUTPUT}/releaseOutput/linux/a")
		link_directories("${MY_OUTPUT}/releaseOutput/linux/so")
	endif()
endif()

#folder name of code by OS
if(WIN32)
 set(OScodeFolder "windows")
else()
 set(OScodeFolder "linux")
endif()


#common header
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/commonHeaders")

set(oggProject "oggProject")
set(vorbisProject "vorbisProject")


set(systemErrorInterfaceShared "systemErrorInterfaceShared")

if(WIN32)
set(win32ApiWrapperShared "win32ApiWrapperShared")
else()
set (linuxApiWrapperShared "linuxApiWrapperShared")
endif()


set(myUtilityShared "myUtilityShared")



set(samplingRateConversionInterfaceShared "samplingRateConversionInterfaceShared")

set(secondaryBufferBaseShared "secondaryBufferBaseShared")
set(oggInterfaceShared "oggInterfaceShared")
set(wavInterfaceShared "wavInterfaceShared")

set(audioMessageQueueInterfaceShared "audioMessageQueueInterfaceShared")
set(primaryBufferInterfaceShared "primaryBufferInterfaceShared")

if(WIN32)
set(win32AudioApiInterfaceShared "win32AudioApiInterfaceShared")
else()
set(alsaApiInterfaceShared "alsaApiInterfaceShared")
endif()

set(audioBufferPoolInterfaceShared "audioBufferPoolInterfaceShared")
set(audioInterfaceShared "audioInterfaceShared")

set(audioTestShared "audioTestShared")
set(simpleStreamingAudioPlayerExe "simpleStreamingAudioPlayerExe")


set(vorbisAndOggHeaderDirectoryPath "${CMAKE_CURRENT_SOURCE_DIR}/vorbisAndOggHeader")


# Load External Project
include(ExternalProject)

ExternalProject_Add(
  # Target name
  "${oggProject}"
  #Directory Options
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../externalProjects/libogg"
  #Download Step Options
  GIT_REPOSITORY git@github.com:xiph/ogg.git
  GIT_TAG 5a234fa51f63d55723abf5a3b940c5573e1c1e18
  #Configure Step Options:
  CMAKE_ARGS -DBUILD_SHARED_LIBS=ON 
  -DCMAKE_INSTALL_PREFIX=install
-DCMAKE_INSTALL_INCLUDEDIR=${vorbisAndOggHeaderDirectoryPath}  -DCMAKE_INSTALL_BINDIR=$<IF:$<CONFIG:Debug>,${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG},$<IF:$<CONFIG:Release>,${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE},programMiss>> -DCMAKE_INSTALL_LIBDIR=$<IF:$<CONFIG:Debug>,${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG},$<IF:$<CONFIG:Release>,${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE},programMiss>>

  #Build Step Options:
  BUILD_COMMAND 
  $<IF:$<AND:$<BOOL:${WIN32}>,$<BOOL:${MSVC}>>,cmake,make>
  $<IF:$<AND:$<BOOL:${WIN32}>,$<BOOL:${MSVC}>>,--build,-w>
  $<IF:$<AND:$<BOOL:${WIN32}>,$<BOOL:${MSVC}>>,.,-w>
  $<IF:$<AND:$<BOOL:${WIN32}>,$<BOOL:${MSVC}>>,--config,-w>
  $<IF:$<AND:$<BOOL:${WIN32}>,$<BOOL:${MSVC}>>,${CMAKE_BUILD_TYPE},-w>
  
  #Install Step Options:
)

if(WIN32 AND MSVC)
set(oggLibraries /ogg.lib)
else()
set(oggLibraries )
endif()

ExternalProject_Add(
  # Target name
  "${vorbisProject}"
  #Directory Options
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../externalProjects/libvorbis"
  #Download Step Options
  GIT_REPOSITORY git@github.com:xiph/vorbis.git
  GIT_TAG 9eadeccdc4247127d91ac70555074239f5ce3529
  #Configure Step Options:
  CMAKE_ARGS -DBUILD_SHARED_LIBS=ON 
  -DCMAKE_INSTALL_PREFIX=install
  -DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_CURRENT_SOURCE_DIR}/vorbisAndOggHeader  -DCMAKE_INSTALL_BINDIR=$<IF:$<CONFIG:Debug>,${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG},$<IF:$<CONFIG:Release>,${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE},programMiss>> -DCMAKE_INSTALL_LIBDIR=$<IF:$<CONFIG:Debug>,${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG},$<IF:$<CONFIG:Release>,${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE},programMiss>> -DOGG_INCLUDE_DIRS=${vorbisAndOggHeaderDirectoryPath}     -DOGG_LIBRARIES=$<IF:$<CONFIG:Debug>,${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}${oggLibraries},$<IF:$<CONFIG:Release>,${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}${oggLibraries},programMiss>>

  #Build Step Options:
  BUILD_COMMAND
  $<IF:$<AND:$<BOOL:${WIN32}>,$<BOOL:${MSVC}>>,cmake,make>
  $<IF:$<AND:$<BOOL:${WIN32}>,$<BOOL:${MSVC}>>,--build,-w>
  $<IF:$<AND:$<BOOL:${WIN32}>,$<BOOL:${MSVC}>>,.,-w>
  $<IF:$<AND:$<BOOL:${WIN32}>,$<BOOL:${MSVC}>>,--config,-w>
  $<IF:$<AND:$<BOOL:${WIN32}>,$<BOOL:${MSVC}>>,${CMAKE_BUILD_TYPE},-w>
 
 #Install Step Options
)

ExternalProject_Add_StepDependencies(${vorbisProject} build ${oggProject})


add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/systemErrorInterfaceShared")

if(WIN32)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/win32ApiWrapperShared")
else()
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/linuxApiWrapperShared")
endif()

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/myUtilityShared")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/samplingRateConversionInterfaceShared") 


add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/secondaryBuffersInterface")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/audioMessageQueueInterfaceShared")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/primaryBufferInterfaceShared")

if(WIN32)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/win32AudioApiInterfaceShared")
else()
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/alsaApiInterfaceShared")
endif()
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/audioBufferPoolInterfaceShared")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/audioInterfaceShared")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/audioTestShared")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/entryPoint")
