# CMakeLists.txt :alsaApiInterfaceShared

file(GLOB allCode
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

if(${TEST})
message(${allCode})
endif()

ADD_LIBRARY(${alsaApiInterfaceShared} ${allCode})

target_link_libraries(${alsaApiInterfaceShared}
	PRIVATE "libasound.so"	
	PRIVATE ${myUtilityShared}
	)

SET_TARGET_PROPERTIES(${alsaApiInterfaceShared} PROPERTIES OUTPUT_NAME alsaApiInterface)

target_include_directories(${alsaApiInterfaceShared} 
INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

