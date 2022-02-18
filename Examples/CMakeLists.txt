file(GLOB QRHIT_EXAMPLE_LIST RELATIVE  ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/*)

foreach(QRHIT_EXAMPLE_NAME ${QRHIT_EXAMPLE_LIST})
    if(IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${QRHIT_EXAMPLE_NAME})
        add_example(${QRHIT_EXAMPLE_NAME})
    endif()
endforeach()

add_library(QRhiWindow
    STATIC
    00-QRhiWindow/QRhiWindow.h
    00-QRhiWindow/QRhiWindow.cpp
)