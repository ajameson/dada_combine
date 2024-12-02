
add_executable(dada_combine dada_combine.c)

install(
  TARGETS
    dada_combine
  RUNTIME DESTINATION
    bin
)
