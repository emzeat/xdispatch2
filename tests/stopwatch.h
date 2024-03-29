/*
 * stopwatch.h
 *
 * Copyright (c) 2011 - 2023 Marius Zwicker
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef STOPWATCH_H_
#define STOPWATCH_H_

#include <stdint.h>
#include <chrono>

#if _WIN32

/* Reduces build time by omitting extra system headers */
#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

class Stopwatch {

  public:
    Stopwatch()
      : _active(false) {
      QueryPerformanceFrequency(&_frequency);
    }
    void start() {
      _active = true;
      QueryPerformanceCounter(&_start);
    }
    void stop() {
      _active = false;
      QueryPerformanceCounter(&_end);
    }
    inline void reset() {
      stop();
      start();
    }
    /**
     * @returns the elapsed time in usec
     */
    std::chrono::microseconds elapsed() {
      if( _active ) {
        QueryPerformanceCounter(&_end);
      }

      LARGE_INTEGER diff;
      diff.QuadPart = _end.QuadPart - _start.QuadPart;

      auto usec = (uint64_t)( (double)diff.QuadPart/(double)_frequency.QuadPart * 1000000UL );
      return std::chrono::microseconds(usec);
    }


  private:
    bool _active;
    LARGE_INTEGER _frequency;
    LARGE_INTEGER _start, _end;

};

#else
# include <sys/time.h>

class Stopwatch {

  public:
    Stopwatch()
      : _active(false) {}
    void start() {
      _active = true;
      gettimeofday( &_start, NULL );
    }
    void stop() {
      _active = false;
      gettimeofday( &_end, NULL );
    }
    inline void reset() {
      stop();
      start();
    }
    /**
     * @returns the elapsed time in usec
     */
    std::chrono::microseconds elapsed() {
      if( _active ) {
        gettimeofday( &_end, NULL );
      }

      auto usec = (_end.tv_sec - _start.tv_sec ) * 1000000UL
        + ( _end.tv_usec - _start.tv_usec );
      return std::chrono::microseconds(usec);
    }


  private:
    bool _active;
    struct timeval _start, _end;

};

#endif

#endif /* STOPWATCH_H_ */
