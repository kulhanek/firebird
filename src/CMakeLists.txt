# ==============================================================================
# Firebird CMake File
# ==============================================================================

# this is required for firebird library ------------------------------------------
INCLUDE_DIRECTORIES(lib/firebird SYSTEM)

# include subdirectories -------------------------------------------------------
ADD_SUBDIRECTORY(lib)

