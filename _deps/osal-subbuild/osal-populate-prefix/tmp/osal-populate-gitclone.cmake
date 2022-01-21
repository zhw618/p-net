
if(NOT "/home/pi/profinet/p-net/_deps/osal-subbuild/osal-populate-prefix/src/osal-populate-stamp/osal-populate-gitinfo.txt" IS_NEWER_THAN "/home/pi/profinet/p-net/_deps/osal-subbuild/osal-populate-prefix/src/osal-populate-stamp/osal-populate-gitclone-lastrun.txt")
  message(STATUS "Avoiding repeated git clone, stamp file is up to date: '/home/pi/profinet/p-net/_deps/osal-subbuild/osal-populate-prefix/src/osal-populate-stamp/osal-populate-gitclone-lastrun.txt'")
  return()
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E rm -rf "/home/pi/profinet/p-net/_deps/osal-src"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: '/home/pi/profinet/p-net/_deps/osal-src'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "/usr/bin/git"  clone --no-checkout "https://github.com/rtlabs-com/osal.git" "osal-src"
    WORKING_DIRECTORY "/home/pi/profinet/p-net/_deps"
    RESULT_VARIABLE error_code
    )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(STATUS "Had to git clone more than once:
          ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/rtlabs-com/osal.git'")
endif()

execute_process(
  COMMAND "/usr/bin/git"  checkout 88784fc --
  WORKING_DIRECTORY "/home/pi/profinet/p-net/_deps/osal-src"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: '88784fc'")
endif()

set(init_submodules TRUE)
if(init_submodules)
  execute_process(
    COMMAND "/usr/bin/git"  submodule update --recursive --init 
    WORKING_DIRECTORY "/home/pi/profinet/p-net/_deps/osal-src"
    RESULT_VARIABLE error_code
    )
endif()
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: '/home/pi/profinet/p-net/_deps/osal-src'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy
    "/home/pi/profinet/p-net/_deps/osal-subbuild/osal-populate-prefix/src/osal-populate-stamp/osal-populate-gitinfo.txt"
    "/home/pi/profinet/p-net/_deps/osal-subbuild/osal-populate-prefix/src/osal-populate-stamp/osal-populate-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: '/home/pi/profinet/p-net/_deps/osal-subbuild/osal-populate-prefix/src/osal-populate-stamp/osal-populate-gitclone-lastrun.txt'")
endif()

