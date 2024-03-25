add_library(
  quack_src OBJECT
  quaternion.c)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:quack_src>
    PARENT_SCOPE)
