add_library(
  quack_src OBJECT
  quack_hooks.cpp
  quack_scan.cpp
  quack_tableam.cpp
  quack_utility.cpp
  quack_write_manager.cpp
  quack.cpp
)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:quack_src>
    PARENT_SCOPE)
