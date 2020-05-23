# CMakeLists.txt :simpleStreamingAudioPlayerExe

file(GLOB allCode
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

if(${TEST})
message(${allCode})
endif()

add_executable(${simpleStreamingAudioPlayerExe} ${allCode})
target_link_libraries(${simpleStreamingAudioPlayerExe} 
	PRIVATE "${audioTestShared}"
)

SET_TARGET_PROPERTIES(${simpleStreamingAudioPlayerExe} PROPERTIES OUTPUT_NAME simpleStreamingAudioPlayer)

set_property(DIRECTORY  PROPERTY VS_STARTUP_PROJECT test)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT laserSystemEx)



