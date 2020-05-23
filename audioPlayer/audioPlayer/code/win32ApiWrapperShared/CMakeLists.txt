# CMakeLists.txt :win32ApiWrapperShared

file(GLOB allCode
	"${CMAKE_CURRENT_LIST_DIR}/win32ApiWrapper/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/win32ApiWrapper/*.cpp"
)

if(${TEST})
message(${allCode})
endif()

ADD_LIBRARY(${win32ApiWrapperShared} ${allCode})

target_link_libraries(${win32ApiWrapperShared}	
 PRIVATE "${systemErrorInterfaceShared}"
 PRIVATE "Avrt.lib")

SET_TARGET_PROPERTIES(${win32ApiWrapperShared} PROPERTIES OUTPUT_NAME win32ApiWrapper)


target_include_directories(${win32ApiWrapperShared} 
INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

