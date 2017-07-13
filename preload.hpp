#ifndef PRELOAD_HPP
#define PRELOAD_HPP

#include "callbacks.hpp"

#define CUDNN_LD_PRELOAD_BOILERPLATE(name)                                     \
  onceActivateCallbacks();                                                     \
  static name##Func real_##name = nullptr;                                     \
  printf("LD_PRELOAD intercept: " #name "\n");                                 \
  if (real_##name == nullptr) {                                                \
    real_##name = (name##Func)dlsym(RTLD_NEXT, #name);                         \
  }                                                                            \
  assert(real_##name && "Will the real " #name " please stand up?");

#define CUBLAS_LD_PRELOAD_BOILERPLATE(name)                                    \
  onceActivateCallbacks();                                                     \
  static name##Func real_##name = nullptr;                                     \
  printf("LD_PRELOAD intercept: " #name "\n");                                 \
  if (real_##name == nullptr) {                                                \
    real_##name = (name##Func)dlsym(RTLD_NEXT, #name "_v2");                   \
  }                                                                            \
  assert(real_##name && "Will the real " #name " please stand up?");

#endif