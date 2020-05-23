# CMakeLists.txt :secondaryBufferBaseShared

file(GLOB allCode
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

if(${TEST})
message(${allCode})
endif()

ADD_LIBRARY(${secondaryBufferBaseShared} ${allCode})

target_link_libraries(${secondaryBufferBaseShared}
	PUBLIC ${samplingRateConversionInterfaceShared}
	)

SET_TARGET_PROPERTIES(${secondaryBufferBaseShared} PROPERTIES OUTPUT_NAME secondaryBufferBase)

get_property(headerDirList1 TARGET ${samplingRateConversionInterfaceShared} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)


target_include_directories(${secondaryBufferBaseShared} 
INTERFACE ${CMAKE_CURRENT_LIST_DIR}
INTERFACE  ${headerDirList1}
)


