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
	PRIVATE ${win32AudioApiInterfaceShared}
	PRIVATE ${audioBufferPoolInterfaceShared}
	PRIVATE ${win32ApiWrapperShared}
	PRIVATE ${myUtilityShared}
	PRIVATE "Winmm.lib"
)

SET_TARGET_PROPERTIES(${audioInterfaceShared} PROPERTIES OUTPUT_NAME audioInterface)

get_property(headerDirList1 TARGET ${win32AudioApiInterfaceShared} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

get_property(headerDirList2 TARGET ${audioBufferPoolInterfaceShared} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(headerDirList3 TARGET ${myUtilityShared} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)


target_include_directories(${audioInterfaceShared} 
INTERFACE ${CMAKE_CURRENT_LIST_DIR}
INTERFACE  ${headerDirList1}
INTERFACE  ${headerDirList2}
INTERFACE  ${headerDirList3}
)


