# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(gc-c-jloxTests LANGUAGES C)

add_executable(gc-c-jlox_test source/gc-c-jlox_test.c)
target_link_libraries(gc-c-jlox_test PRIVATE gc-c-jlox_lib)
target_compile_features(gc-c-jlox_test PRIVATE c_std_11)

add_test(NAME gc-c-jlox_test COMMAND gc-c-jlox_test)
