# CMakeLists.txt :audioTestShared


file(GLOB allCode
    "${CMAKE_CURRENT_LIST_DIR}/${OScodeFolder}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/${OScodeFolder}/*.cpp"
)

if(${TEST})
message(${allCode})
endif()

ADD_LIBRARY(${audioTestShared} ${allCode})

target_link_libraries(${audioTestShared}
	PRIVATE ${audioInterfaceShared}
)

SET_TARGET_PROPERTIES(${audioTestShared} PROPERTIES OUTPUT_NAME audioTest)

target_include_directories(${audioTestShared} 
INTERFACE "${CMAKE_CURRENT_LIST_DIR}/${OScodeFolder}"
)

