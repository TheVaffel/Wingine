#include <iostream>
#include <exception>

#define catch_error(X) 			\
  [&]{ \
try { \
  return X;			   \
} catch(std::exception &error) {						\
    std::cerr << "[wingine_c] Caught exception at " << __FILE__ << ":" << __LINE__ << ": " << error.what() << std::endl; \
  throw error; \
} \
}()
