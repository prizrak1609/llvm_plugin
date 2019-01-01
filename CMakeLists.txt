set(LLVM_LINK_COMPONENTS support)

add_clang_executable(loop-convert
  LoopConvert.cpp
  )

llvm_map_components_to_libnames(ReferencedLLVMLibs core executionengine irreader instrumentation interpreter object support native)

target_link_libraries(loop-convert
  PRIVATE
  clangTooling
  clangBasic
  clangASTMatchers
  ${ReferencedLLVMLibs}
  )
