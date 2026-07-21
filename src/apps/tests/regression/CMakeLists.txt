set(FM_APP_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/app_entry.c
    ${CMAKE_CURRENT_LIST_DIR}/fm_regression_test.c
    ${CMAKE_CURRENT_LIST_DIR}/../../product/main/fm_main_event.c
    ${CMAKE_CURRENT_LIST_DIR}/../../product/main/fm_main_input_adapter.c
    ${CMAKE_CURRENT_LIST_DIR}/../../product/main/fm_main_input_recognizer.c
    PARENT_SCOPE
)

set(FM_APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../product/main
    PARENT_SCOPE
)
