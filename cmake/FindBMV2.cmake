# To find the BMv2 executables in the BMv2 source tree instead of
# an installed location, set BMV2_SOURCE_DIR to the BMv2 source directory.
set(BMV2_SIMPLE_SWITCH_SEARCH_PATHS
  ${CMAKE_INSTALL_PREFIX}/bin)
if(DEFINED BMV2_SOURCE_DIR)
  set(BMV2_SIMPLE_SWITCH_SEARCH_PATHS
    ${BMV2_SIMPLE_SWITCH_SEARCH_PATHS}
    ${BMV2_SOURCE_DIR}/targets/simple_switch)
endif()

# check for simple_switch
find_program (SIMPLE_SWITCH_CLI simple_switch_CLI
  PATHS ${BMV2_SIMPLE_SWITCH_SEARCH_PATHS} )
if (SIMPLE_SWITCH_CLI)
  find_program (SIMPLE_SWITCH simple_switch
    PATHS ${BMV2_SIMPLE_SWITCH_SEARCH_PATHS} )
  if (SIMPLE_SWITCH)
    set (HAVE_SIMPLE_SWITCH 1)
  endif(SIMPLE_SWITCH)
endif(SIMPLE_SWITCH_CLI)

mark_as_advanced(SIMPLE_SWITCH SIMPLE_SWITCH_CLI)

find_package_handle_standard_args ("BMV2"
  "Program 'simple_switch_CLI' (https://github.com/p4lang/behavioral-model.git) not found;\nSearched ${BMV2_SIMPLE_SWITCH_SEARCH_PATHS}.\nWill not run BMv2 tests."
  SIMPLE_SWITCH SIMPLE_SWITCH_CLI)

set(BMV2_SIMPLE_SWITCH_GRPC_SEARCH_PATHS
  ${CMAKE_INSTALL_PREFIX}/bin)
if(DEFINED BMV2_SOURCE_DIR)
  set(BMV2_SIMPLE_SWITCH_GRPC_SEARCH_PATHS
    ${BMV2_SIMPLE_SWITCH_GRPC_SEARCH_PATHS}
    ${BMV2_SOURCE_DIR}/targets/simple_switch_grpc)
endif()

# check for simple_switch_grpc
find_program (SIMPLE_SWITCH_GRPC simple_switch_grpc PATHS ${BMV2_SIMPLE_SWITCH_GRPC_SEARCH_PATHS} )
if (SIMPLE_SWITCH_GRPC)
  set (HAVE_SIMPLE_SWITCH_GRPC 1)
endif(SIMPLE_SWITCH_GRPC)
mark_as_advanced(SIMPLE_SWITCH_GRPC)

find_package_handle_standard_args ("BMV2"
  "Program 'simple_switch_grpc' (https://github.com/p4lang/behavioral-model.git) not found;\nSearched ${BMV2_SIMPLE_SWITCH_GRPC_SEARCH_PATHS}.\nWill not run BMv2 PTF tests."
  SIMPLE_SWITCH_GRPC)


set(BMV2_PSA_SWITCH_SEARCH_PATHS
  ${CMAKE_INSTALL_PREFIX}/bin)
if(DEFINED BMV2_SOURCE_DIR)
  set(BMV2_PSA_SWITCH_SEARCH_PATHS
    ${BMV2_PSA_SWITCH_SEARCH_PATHS}
    ${BMV2_SOURCE_DIR}/targets/psa_switch)
endif()

# check for psa_switch
find_program (PSA_SWITCH_CLI psa_switch_CLI
  PATHS ${BMV2_PSA_SWITCH_SEARCH_PATHS} )
if (PSA_SWITCH_CLI)
  find_program (PSA_SWITCH psa_switch
    PATHS ${BMV2_PSA_SWITCH_SEARCH_PATHS} )
  if (PSA_SWITCH)
    set (HAVE_PSA_SWITCH 1)
  endif(PSA_SWITCH)
endif(PSA_SWITCH_CLI)

mark_as_advanced(PSA_SWITCH PSA_SWITCH_CLI)

find_package_handle_standard_args ("BMV2"
  "Program 'psa_switch_CLI' (https://github.com/p4lang/behavioral-model.git) not found;\nSearched ${BMV2_PSA_SWITCH_SEARCH_PATHS}.\nWill not run PSA BMv2 tests."
  PSA_SWITCH PSA_SWITCH_CLI)

set(BMV2_PNA_NIC_SEARCH_PATHS
  ${CMAKE_INSTALL_PREFIX}/bin)
if(DEFINED BMV2_SOURCE_DIR)
  set(BMV2_PNA_NIC_SEARCH_PATHS
    ${BMV2_PNA_NIC_SEARCH_PATHS}
    ${BMV2_SOURCE_DIR}/targets/pna_nic)
endif()
  
# check for pna_nic
find_program (PNA_NIC_CLI pna_nic_CLI
  paths ${BMV2_PNA_NIC_SEARCH_PATHS} )
if (PNA_NIC_CLI)
  find_program (PNA_NIC pna_nic
    PATHS ${BMV2_PNA_NIC_SEARCH_PATHS} )
  if (PNA_NIC)
    set (HAVE_PNA_NIC 1)
  endif (PNA_NIC)
endif (PNA_NIC_CLI)

mark_as_advanced(PNA_NIC PNA_NIC_CLI)

find_package_handle_standard_args ("BMV2"
  "Program 'pna_nic_CLI' (https://github.com/p4lang/behavioral-model.git) not found;\nSearched ${BMV2_PNA_NIC_SEARCH_PATHS}.\nWill not run PNA PNA BMv2 tests."
  PNA_NIC PNA_NIC_CLI)
