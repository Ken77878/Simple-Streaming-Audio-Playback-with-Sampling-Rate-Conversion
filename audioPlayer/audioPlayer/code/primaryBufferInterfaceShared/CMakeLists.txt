# CMakeLists.txt :primaryBufferInterfaceShared

file(GLOB allCode
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

if(${TEST})
message(${allCode})
endif()

ADD_LIBRARY(${primaryBufferInterfaceShared} ${allCode})

target_link_libraries(${primaryBufferInterfaceShared}
	PRIVATE ${myUtilityShared}
	PRIVATE ${audioMessageQueueInterfaceShared}
	PRIVATE ${secondaryBufferBaseShared}
	PRIVATE ${oggInterfaceShared}
	PRIVATE ${wavInterfaceShared}
)

SET_TARGET_PROPERTIES(${primaryBufferInterfaceShared} PROPERTIES OUTPUT_NAME primaryBufferInterface)

get_property(headerDirList1 TARGET ${audioMessageQueueInterfaceShared} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

get_property(headerDirList2 TARGET ${secondaryBufferBaseShared} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(${primaryBufferInterfaceShared} 
INTERFACE ${CMAKE_CURRENT_LIST_DIR}
INTERFACE  ${headerDirList1}
INTERFACE  ${headerDirList1}
)


