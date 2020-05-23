# CMakeLists.txt :oggInterfaceShared

file(GLOB allCode
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

if(${TEST})
message(${allCode})
endif()

ADD_LIBRARY(${oggInterfaceShared} SHARED ${allCode})


target_link_libraries(${oggInterfaceShared} 
PRIVATE "ogg"
PRIVATE "vorbis"
PRIVATE "vorbisenc"
PRIVATE "vorbisfile"
PRIVATE ${myUtilityShared}
PRIVATE ${systemErrorInterfaceShared}
PRIVATE ${secondaryBufferBaseShared}
)

add_dependencies(${oggInterfaceShared} ${vorbisProject})

SET_TARGET_PROPERTIES(${oggInterfaceShared} PROPERTIES OUTPUT_NAME oggInterface)


target_include_directories(${oggInterfaceShared} 
INTERFACE ${CMAKE_CURRENT_LIST_DIR}
PUBLIC ${vorbisAndOggHeaderDirectoryPath} 
)

