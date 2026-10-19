add_executable(gb_tests
    main.cpp
    smoke_test.cpp
    opcodes_test.cpp
)

target_link_libraries(gb_tests PRIVATE gameboy doctest::doctest)

add_test(NAME gb_tests COMMAND gb_tests)
