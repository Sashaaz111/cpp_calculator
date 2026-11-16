function(add_calc_plugin target_name source_file)
    add_library(${target_name} SHARED ${source_file})    
   
    set_target_properties(${target_name} PROPERTIES PREFIX "")
    
   
    set_target_properties(${target_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins"
    )
    

    foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER)
        set_target_properties(${target_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/bin/${CONFIG_TYPE}/plugins"
            LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/bin/${CONFIG_TYPE}/plugins"
        )
    endforeach()
endfunction()

add_calc_plugin(funcsin funcsin.cpp)
add_calc_plugin(funcdeg funcdeg.cpp)
add_calc_plugin(funcln  funcln.cpp)