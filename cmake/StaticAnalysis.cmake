find_program(CLANG_TIDY_EXE NAMES clang-tidy)
find_program(CPPCHECK_EXE NAMES cppcheck)

function(upgrade_guard_enable_clang_tidy target)
  if(CLANG_TIDY_EXE)
    set_property(TARGET ${target} PROPERTY CXX_CLANG_TIDY ${CLANG_TIDY_EXE})
  endif()
endfunction()
