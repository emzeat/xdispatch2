# xdispatch2

This is the successor to the original [xdispatch project](http://opensource.mlba-team.de/xdispatch).

## A bit on history

I started the original xdispatch project back in 2011 with an attempt to bring the then new concept of [Grand Central Dispatch](https://en.wikipedia.org/wiki/Grand_Central_Dispatch) to more platforms than just iOS and macOS.

The project aimed to provide three deliverables:

1. Porting the [original libdispatch](https://apple.github.io/swift-corelibs-libdispatch) to Linux and Win32 by using as much of the original code as possible which also implied using ported versions of the libkqueue and libpthread_workqueue interfaces used by libdispatch originally.
2. Offering an object oriented C++ interface to allow use of [lambda functions](https://en.cppreference.com/w/cpp/language/lambda) which only started to get supported back then as a result of the new [C++11 standard](https://en.cppreference.com/w/Template:cpp/compiler_support/11). This was in order to offer a cross platform alternative to the [blocks feature](https://clang.llvm.org/docs/BlockLanguageSpec.html) added by Apple to their clang compiler.
3. Providing a wrapper which tried to fit naturally into other Qt components like [QtConcurrent](https://doc.qt.io/qt-6/qtconcurrent-index.html).

These goals were accomplished with the latest [feature version 0.7.2](http://opensource.mlba-team.de/xdispatch/news.htm) published in January 2013 and development focusing on [maintenance and bug fixes](https://github.com/mlba-team/xdispatch/compare/19c98b6cf9040bd13faf76dc9e1335412d627d1c...master) ever since.

## Back to present

Dial back to June 2020 when I had gathered several lessons learned from using xdispatch in multiple projects from [little university projects](https://github.com/emzeat/steriss) to [full media servers sold on the app store](https://mlba-team.de/shlive/):

1. The concept of queues really makes things easier not only for scaling on multiple cores but also for efficient serialization of acccess to resources like an OpenGL render context.

2. A lot of systems still have thread affinities assuming that certain tasks will always run on the same threads (again such as OpenGL render contexts).

The latter does not fit very well with a concept using a thread pool. Pooling of threads implies there must never be any assumptions on thread affinities.

This is when the idea of xdispatch v2 was born. It would continue to maintain close compatibility with the original xdispatch v1 C++ interface. However instead of trying to chase porting the latest features of the original libdispatch (and hence depending on faking Darwin specific interfaces) it would focus on providing a consistent interface for different thread pool concepts available on the respective platforms and programming environment.

This resulted in setting up the following rules:
1. The xdipsatch2 core featureset must not depend on anything but a C++11 compliant compiler and STL.
2. The entire interface is abstracted such that serial or parallel queues as well as timers and notifiers can be implemented by different backends all while maintaining feature consistency and allowing to mix xdispatch objects powered by different backends as required.
2. Support a minimal, self hosted "naive" implementation as feature reference which might be less efficient but can always be supported within the limits set up in rule 1.

All further decisions are based on these rules. Backends need to implement the `xdispatch::ibackend` interface when they want to provide the full feature set on their own or may alternatively hook into the `xdispatch::naive::ithread` and `xdispatch::naive::ithread_pool` interfaces to augment the naive implementation with more efficient threading primitives.

### Backends

The following backends have been implemented so far:
| Backend  | Wrapped Library | Conditions       |
| -------- | --------------- | ---------------- |
| naive    | std::thread     | Always available |
| dispatch | libdispatch     | dispatch.h header and interface found when building xdispatch2 |
| qt       | Qt5             | Qt5 found when building xdispatch2 |

### Benchmarks

Numbers have been collected on an AMD Ryzen 3900x 12 core processor. They try to quantify the overhead induced by enqueuing and executing preallocated `xdispatch::operation` objects. The operation is dispatched 100 000 times using the respective primitive. Times describe the overhead for a single invocation. Sources can be found in `tests/cxx_benchmark.cpp`.

| Backend  | Serial Queue | Parallel Queue | Group     |
| -------- | ------------ | -------------- | --------- |
| naive    | 187 nsec     | 295 nsec       | 644 nsec  |
| dispatch | 585 nsec     | 448 nsec       | 780 nsec  |
| qt       | 544 nsec     | 411 nsec       | 693 nsec  |

## Getting Started

As in v1 CMake is used as the build system wrapped with my [mz-cmaketools](https://github.com/emzeat/mz-cmaketools) for convenience.

Make sure you have `cmake`, `g++` and `ninja` installed and available in your path. Use cmake to configure and build the project:
```bash
mkdir build/release
cd build/release
cmake -G Ninja -D CMAKE_BUILD_TYPE=Release -S ../.. -B .
cmake --build .
```

The resulting binaries can be found in the `bin` directory of the build folder created above. You simply need to pull the `include` directory in your compiler's include path and link the `xdispatch2` library with your program.

A (very) minimal use might look like this:
```C++
#include <xdispatch/dispatch>
#include <iostream>

int main()
{
    xdispatch::global_queue().async([]
    {
        std::cout << "PING" << std::endl;

        xdispatch::main_queue().async([]{
            std::cout << "PONG" << std::endl;
        });
    });

    xdispatch::exec();
}
```

Have a look at the implemented `tests` for more elaborated examples.
