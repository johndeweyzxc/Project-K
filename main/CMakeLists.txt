idf_component_register(
    SRCS "CmdParser.cpp" "WifiCtl.cpp" "WebServer.cpp" "Injector.cpp" "Exploits.cpp" "main.cpp"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_event esp_netif esp_http_server json esp_wifi
)

target_link_libraries(${COMPONENT_LIB} -Wl,-zmuldefs)