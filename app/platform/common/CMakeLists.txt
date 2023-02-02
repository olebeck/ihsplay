target_sources(ihsplay PRIVATE app_common.c client_info_common.c)

if(WINRT)
    target_sources(ihsplay PRIVATE os_info_winrt.c )
else()
    target_sources(ihsplay PRIVATE os_info_linux.c )
endif()
