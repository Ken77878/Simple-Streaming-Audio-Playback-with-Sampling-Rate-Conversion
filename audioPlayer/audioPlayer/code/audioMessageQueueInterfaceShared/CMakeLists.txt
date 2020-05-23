# CMakeLists.txt :audioMessageQueueInterfaceShared

file(GLOB allCode
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

if(${TEST})
message(${allCode})
endif()

ADD_LIBRARY(${audioMessageQueueInterfaceShared} ${allCode})

target_link_libraries(${audioMessageQueueInterfaceShared}
	PRIVATE ${secondaryBufferBaseShared}
)

SET_TARGET_PROPERTIES(${audioMessageQueueInterfaceShared} PROPERTIES OUTPUT_NAME audioMessageQueueInterface)

get_property(headerDirList1 TARGET ${secondaryBufferBaseShared} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(${audioMessageQueueInterfaceShared} 
INTERFACE ${CMAKE_CURRENT_LIST_DIR}
INTERFACE  ${headerDirList1}
)


