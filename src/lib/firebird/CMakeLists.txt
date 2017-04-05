# ==============================================================================
# Firebird CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(FIREBIRD_OBJ
        FirebirdMainHeader.cpp
        FirebirdDatabase.cpp
        FirebirdTransaction.cpp
        FirebirdBLOB.cpp
        FirebirdItem.cpp
        FirebirdExecuteSQL.cpp
        FirebirdQuerySQL.cpp
        )

# create static library --------------------------------------------------------
IF(LIBS_STATIC)
    ADD_LIBRARY(firebird_static STATIC ${FIREBIRD_OBJ})
    SET_TARGET_PROPERTIES(firebird_static PROPERTIES
                            OUTPUT_NAME firebird
                            CLEAN_DIRECT_OUTPUT 1)
ENDIF(LIBS_STATIC)

# create shared library --------------------------------------------------------
IF(LIBS_SHARED)
    ADD_LIBRARY(firebird_shared SHARED ${FIREBIRD_OBJ})
    TARGET_LINK_LIBRARIES(firebird_shared ${FBCLIENT_LIB_NAME})
    SET_TARGET_PROPERTIES(firebird_shared PROPERTIES
                            OUTPUT_NAME firebird
                            CLEAN_DIRECT_OUTPUT 1
                            VERSION ${FIREBIRD_VERSION}
                            )
    INSTALL(TARGETS
                firebird_shared
            LIBRARY DESTINATION
                lib
            )
ENDIF(LIBS_SHARED)

