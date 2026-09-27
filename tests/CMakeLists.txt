# Each test file is its own small executable linking cpm_engine directly
# (no window/GL context needed -- see the root CMakeLists.txt's cpm_engine/CPM
# split), registered individually with CTest rather than combined into one
# binary, so a crash in one test file doesn't hide the others' results.

function(cpm_add_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE cpm_engine)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

cpm_add_test(test_docopy)
# docopy() is private; only test_docopy needs the CPM_ENABLE_TEST_HOOKS friend
# accessor (see CellularPotts.h), so it's scoped to just this target rather
# than defined engine-wide.
target_compile_definitions(test_docopy PRIVATE CPM_ENABLE_TEST_HOOKS)

cpm_add_test(test_adhesion)
cpm_add_test(test_gradient_constraints)

# test_scenarios and test_render construct a Simulation, which owns an
# OpenCL member that eagerly loads "kernel.cl" via a cwd-relative ifstream
# (see OpenCL::OpenCL) -- so it must sit next to wherever ctest actually runs
# these two executables from (this directory's build dir, ctest's default
# working directory for tests added here).
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernel.cl
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CPM_DIR}/kernel.cl
        ${CMAKE_CURRENT_BINARY_DIR}/kernel.cl
    DEPENDS ${CPM_DIR}/kernel.cl
)
add_custom_target(test_kernel_cl DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kernel.cl)

cpm_add_test(test_scenarios)
add_dependencies(test_scenarios test_kernel_cl)

cpm_add_test(test_render)
add_dependencies(test_render test_kernel_cl)
