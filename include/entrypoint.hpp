#pragma once

#if defined(_WIN32)
// TODO: Windows entrypoint
#else
/// Standard C entrypoint
int main(int argc, char** argv);
#endif

namespace kuiper {
/// Actual entrypoint, as far as the engine is concerned.
/// Once we reach the real entrypoint, `WinMain` should've converted any UTF-16 wchar strings into UTF-8 C-style strings
int entrypoint(int argc, char** argv);
} // namespace kuiper
