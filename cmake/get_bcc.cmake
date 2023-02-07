include(FetchContent)

FetchContent_Declare(bcc
  GIT_REPOSITORY https://github.com/iovisor/bcc.git
  GIT_TAG        504180036f2e928181bc946572908f0d3aa25fa6
)

set(ENABLE_EXAMPLES OFF)
set(ENABLE_MAN OFF)
set(ENABLE_TESTS OFF)

FetchContent_MakeAvailable(bcc)
add_library(libbcc ALIAS bcc)
