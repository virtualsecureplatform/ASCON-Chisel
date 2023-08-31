
add_executable(cpptest 
    cpptest.cpp
    ${PROJECT_SOURCE_DIR}/thirdparties/ascon-hardware/software/ascon_ref/crypto_hash/asconhashv12/ref/printstate.c)
target_include_directories(cpptest
  PUBLIC ${PROJECT_SOURCE_DIR}/thirdparties/ascon-hardware/software/ascon_ref/crypto_hash/asconhashv12/ref
  ${CMAKE_CURRENT_SOURCE_DIR}
)
verilate(cpptest SOURCES ../../../ASCONXOF.v THREADS 1 TRACE_FST)