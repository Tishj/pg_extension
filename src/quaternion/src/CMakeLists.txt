add_library(
  quaternion_src OBJECT
  quaternion.c)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:quaternion_src>
    PARENT_SCOPE)
