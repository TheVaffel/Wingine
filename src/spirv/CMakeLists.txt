target_sources(Wingine PRIVATE
  header.cpp
  variable_parser.cpp
  Operation.cpp
  SpirvModule.cpp
  decorator_parser.cpp
  ParseResult.cpp
  util.cpp)

add_subdirectory(types)
