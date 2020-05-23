# CMakeLists.txt :samplingRateConversionInterfaceShared

file(GLOB allCode
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

if(${TEST})
message(${allCode})
endif()

ADD_LIBRARY(${samplingRateConversionInterfaceShared} ${allCode})

target_link_libraries(${samplingRateConversionInterfaceShared}
	PRIVATE ${myUtilityShared}
)

SET_TARGET_PROPERTIES(${samplingRateConversionInterfaceShared} PROPERTIES OUTPUT_NAME samplingRateConversionInterface)

target_include_directories(${samplingRateConversionInterfaceShared} 
INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)


