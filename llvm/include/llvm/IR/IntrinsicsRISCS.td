let TargetPrefix = "riscs" in {  // All intrinsics start with "llvm.riscs.".

def int_riscs_flush: ClangBuiltin<"__builtin_riscs_flush">,
  Intrinsic<[], [], [IntrNoMem, IntrHasSideEffects]>;

def int_riscs_rand: ClangBuiltin<"__builtin_riscs_rand">,
  Intrinsic<[llvm_i64_ty], [], [IntrNoMem, IntrHasSideEffects]>;

def int_riscs_quit_event: ClangBuiltin<"__builtin_riscs_quit_event">,
  Intrinsic<[llvm_i64_ty], [], [IntrNoMem, IntrHasSideEffects]>;

def int_riscs_set_pixel: ClangBuiltin<"__builtin_riscs_set_pixel">,
  Intrinsic<[], [llvm_i64_ty, llvm_i64_ty, llvm_i64_ty], [IntrNoMem, IntrHasSideEffects]>;

} // TargetPrefix = "riscs"
