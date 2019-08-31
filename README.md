# Wingine

A Vulkan wrapper aiming to make graphics development somewhat enjoyable.

Previously, Wingine has been a part of [HConLib](https://github.com/TheVaffel/HConLib), but has been moved into its own project to ease sustained development.

Current functionality is limited. In fact, it's even more limited than the one in HConLib. This is because I have rewritten it using Khronos' [C++ API for Vulkan](https://github.com/KhronosGroup/Vulkan-Hpp).

Current objectives include making it great again and then making it even greater.

## Differences From HConLib Version

This version of Wingine is
- Written with Khronos' Vulkan C++ API
- Without direct support for GLSL (but you can use it if you want, provided you have some way to convert it into SPIR-V)
- Kinda biased towards game developers, although it's hard to tell at this point
- Under construction