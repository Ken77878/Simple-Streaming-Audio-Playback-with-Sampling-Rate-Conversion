# CMakeLists.txt :audioBufferPoolInterfaceShared

file(GLOB allCode
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

if(${TEST})
message(${allCode})
endif()

ADD_LIBRARY(${audioBufferPoolInterfaceShared} ${allCode})

target_link_libraries(${audioBufferPoolInterfaceShared}
	PUBLIC ${primaryBufferInterfaceShared}
	PRIVATE ${myUtilityShared}
)

SET_TARGET_PROPERTIES(${audioBufferPoolInterfaceShared} PROPERTIES OUTPUT_NAME audioBufferPoolInterface)

get_property(headerDirList1 TARGET ${primaryBufferInterfaceShared} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(headerDirList2 TARGET ${myUtilityShared} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(${audioBufferPoolInterfaceShared} 
INTERFACE ${CMAKE_CURRENT_LIST_DIR}
INTERFACE  ${headerDirList1}
INTERFACE  ${headerDirList2}
)


