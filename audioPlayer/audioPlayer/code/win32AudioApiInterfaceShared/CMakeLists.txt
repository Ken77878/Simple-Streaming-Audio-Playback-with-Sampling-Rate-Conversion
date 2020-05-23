# CMakeLists.txt :win32AudioApiInterfaceShared

file(GLOB allCode
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

if(${TEST})
message(${allCode})
endif()

ADD_LIBRARY(${win32AudioApiInterfaceShared} ${allCode})

target_link_libraries(${win32AudioApiInterfaceShared}	
 PRIVATE "Winmm.dll"
 PRIVATE "${win32ApiWrapperShared}"
 PRIVATE "${myUtilityShared}")

SET_TARGET_PROPERTIES(${win32AudioApiInterfaceShared} PROPERTIES OUTPUT_NAME win32AudioApiInterface)

get_property(headerDirList1 TARGET ${myUtilityShared} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)



target_include_directories(${win32AudioApiInterfaceShared} 
INTERFACE ${CMAKE_CURRENT_LIST_DIR}
INTERFACE  ${headerDirList1}
)

