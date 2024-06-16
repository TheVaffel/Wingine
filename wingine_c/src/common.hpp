#pragma once
#include <functional>

#define exitOnFail(lambda) _exitOnFail(std::function(lambda))

template<typename T>
T _exitOnFail(const std::function<T()>& func) {
    try {
        return func();
    } catch (const std::runtime_error& error) {
        std::cerr << "[wingine.cpp] Encountered runtime error: " << error.what() << std::endl;
        exit(-1);
    } catch (...) {
        std::cerr << "[wingine.cpp] Unrecognized error, quitting" << std::endl;
        exit(-1);
    }
}
