# CMakeLists.txt :audioInterfaceShared



file(GLOB allCode
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

if(${TEST})
message(${allCode})
endif()

ADD_LIBRARY(${audioInterfaceShared} ${allCode})


target_link_libraries(${audioInterfaceShared}
	PRIVATE ${audioBufferPoolInterfaceShared}
	PRIVATE ${alsaApiInterfaceShared}
	PRIVATE ${myUtilityShared}
	PRIVATE ${linuxApiWrapperShared}
	)

SET_TARGET_PROPERTIES(${audioInterfaceShared} PROPERTIES OUTPUT_NAME audioInterface)


get_property(headerDirList1 TARGET ${audioBufferPoolInterfaceShared} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)


get_property(headerDirList2 TARGET ${alsaApiInterfaceShared} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

get_property(headerDirList3 TARGET ${primaryBufferInterfaceShared} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(headerDirList4 TARGET ${myUtilityShared} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)


target_include_directories(${audioInterfaceShared} 
INTERFACE ${CMAKE_CURRENT_LIST_DIR}
INTERFACE  ${headerDirList1}
INTERFACE  ${headerDirList2}
INTERFACE  ${headerDirList3}
INTERFACE  ${headerDirList4}
)


