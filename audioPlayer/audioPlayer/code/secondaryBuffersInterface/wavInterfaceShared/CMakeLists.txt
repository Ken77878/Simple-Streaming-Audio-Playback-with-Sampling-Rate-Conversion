# CMakeLists.txt :wavInterfaceShared

file(GLOB allCode
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

if(${TEST})
message(${allCode})
endif()

ADD_LIBRARY(${wavInterfaceShared} ${allCode})

target_link_libraries(${wavInterfaceShared}
	PRIVATE ${secondaryBufferBaseShared}
	PRIVATE ${myUtilityShared}
	PRIVATE ${systemErrorInterfaceShared}
	)

SET_TARGET_PROPERTIES(${wavInterfaceShared} PROPERTIES OUTPUT_NAME wavInterface)

#get_property(headerDirList1 TARGET ${sceneTitleLib} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

#get_property(headerDirList2 TARGET ${scenePrologueLib} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

#get_property(headerDirList3 TARGET ${sceneMapLib} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

#get_property(headerDirList4 TARGET ${sceneBattleLib} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(${wavInterfaceShared} 
INTERFACE ${CMAKE_CURRENT_LIST_DIR}
#INTERFACE  ${headerDirList1}
)

#get_property(propertyTest1 TARGET ${operationScreenBaseShared} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
#get_property(propertyTest2 TARGET ${operationScreenBaseShared} PROPERTY LINK_LIBRARIES)

#message("diretory test================${headerDirList1}==================\n")
#message("================${headerDirList2}==================\n")
#message("================${headerDirList3}==================\n")
#message("================${headerDirList4}==================test end\n")
