set(LP_SOLVE_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})


set(LPSOLVE_INCLUDE_DIR "/opt/homebrew/Cellar/lp_solve/5.5.2.11/include")
set(LPSOLVE_LIBRARY "/opt/homebrew/Cellar/lp_solve/5.5.2.11/lib/liblpsolve55.dylib")

function(GetLPSolve)
    message(STATUS "Using system-installed lp_solve")

    include_directories(${LPSOLVE_INCLUDE_DIR})

    add_library(lp_solve SHARED IMPORTED)
    set_target_properties(lp_solve PROPERTIES
        IMPORTED_LOCATION ${LPSOLVE_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${LPSOLVE_INCLUDE_DIR}
    )
endfunction()