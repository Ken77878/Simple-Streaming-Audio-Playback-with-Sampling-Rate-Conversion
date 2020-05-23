# CMakeLists.txt :linuxApiWrapperShared

file(GLOB allCode
	"${CMAKE_CURRENT_LIST_DIR}/linuxApiWrapper/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/linuxApiWrapper/*.cpp"
)

if(${TEST})
message(${allCode})
endif()

ADD_LIBRARY(${linuxApiWrapperShared} ${allCode})

target_link_libraries(${linuxApiWrapperShared}	
 PRIVATE "${systemErrorInterfaceShared}")

SET_TARGET_PROPERTIES(${linuxApiWrapperShared} PROPERTIES OUTPUT_NAME linuxApiWrapper)


target_include_directories(${linuxApiWrapperShared} 
INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

