# Allow test source files generated from template to find test headers:

set(frontend_dir ${CMAKE_SOURCE_DIR}/frontend)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(unit_testing unit_test.cpp)
target_link_libraries(unit_testing frontend_lib)

function(add_unit_tests name test_header)

  set(test_function_call_code "")
  foreach(function IN LISTS ARGN)
    set(code "    if(test_name == \"${function}\") return ${function}();\n" )
    set(test_function_call_code "${test_function_call_code}${code}")
  endforeach()

  set(test_driver_src ${name}.driver.cpp)
  configure_file(unit_test_driver.cpp.template ${test_driver_src})

  set(test_driver "unit_test_${name}")
  add_executable(${test_driver} ${test_driver_src})

  target_link_libraries(${test_driver} unit_testing)

  foreach(function IN LISTS ARGN)
    set(test_name ${function})
    add_test(NAME ${test_name} COMMAND ${test_driver} "${test_name}")
  endforeach()

endfunction()

set(semantic_test_funcs
"binop::ii"
"binop::ir"
"binop::ri"
"binop::rr"
"binop::iR"
"binop::rR"
"binop::Ri"
"binop::Rr"
"binop::RR"
"binop::iS"
"binop::rS"
"binop::Si"
"binop::Sr"
"binop::SS"
)

add_unit_tests(semantics test_semantics.hpp ${semantic_test_funcs})