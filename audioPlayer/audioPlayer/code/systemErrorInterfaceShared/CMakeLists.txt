# CMakeLists.txt :systemErrorInterfaceShared

file(GLOB allCode
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

if(${TEST})
message(${allCode})
endif()

ADD_LIBRARY(${systemErrorInterfaceShared} ${allCode})

SET_TARGET_PROPERTIES(${systemErrorInterfaceShared} PROPERTIES OUTPUT_NAME systemErrorInterface)


target_include_directories(${systemErrorInterfaceShared} 
INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)


