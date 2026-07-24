set(FM_APP_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/app_entry.c
    ${CMAKE_CURRENT_LIST_DIR}/fm_main_event.c
    ${CMAKE_CURRENT_LIST_DIR}/fm_main_input_adapter.c
    ${CMAKE_CURRENT_LIST_DIR}/fm_main_input_recognizer.c
    ${CMAKE_CURRENT_LIST_DIR}/fm_main_presentation_lcd.c
    ${CMAKE_CURRENT_LIST_DIR}/fm_main.c
    PARENT_SCOPE
)

set(FM_APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    PARENT_SCOPE
)
