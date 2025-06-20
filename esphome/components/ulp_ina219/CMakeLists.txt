FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)
idf_component_register(SRCS ${app_sources} )

# ULP Setup
# program name and folder structure cannot be altered due to platformio restrictions
set(ulp_app_name ulp_main)
set(ulp_s_sources "../ulp/main.c" )
set(ulp_exp_dep_srcs "esphome/components/ulp_ina219/ulp_ina219.cpp")
ulp_embed_binary(${ulp_app_name} "${ulp_s_sources}" "${ulp_exp_dep_srcs}")
