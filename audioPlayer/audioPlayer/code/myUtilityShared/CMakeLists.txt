# CMakeLists.txt :myUtilityShared

file(GLOB allCode
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/${OScodeFolder}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/${OScodeFolder}/*.cpp"
)

if(${TEST})
message(${allCode})
endif()

ADD_LIBRARY(${myUtilityShared} ${allCode})

target_link_libraries(${myUtilityShared}
	PRIVATE ${systemErrorInterfaceShared}
	)

if(WIN32 AND MSVC)
target_link_libraries(${myUtilityShared}
	PRIVATE ${win32ApiWrapperShared}
	)
else()
target_link_libraries(${myUtilityShared}
	PRIVATE ${linuxApiWrapperShared}
	)
endif()

SET_TARGET_PROPERTIES(${myUtilityShared} PROPERTIES OUTPUT_NAME myUtility)

target_include_directories(${myUtilityShared} 
INTERFACE ${CMAKE_CURRENT_LIST_DIR}
PUBLIC "${CMAKE_CURRENT_LIST_DIR}/${OScodeFolder}"
)


