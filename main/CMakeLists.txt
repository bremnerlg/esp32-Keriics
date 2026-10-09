idf_component_register(SRCS "command_parser.cpp"
                       PRIV_REQUIRES console esp_driver_uart fatfs console
                                     esp_usb_cdc_rom_console nvs_flash cmd_system cmd_wifi cmd_nvs
                       INCLUDE_DIRS ".")
