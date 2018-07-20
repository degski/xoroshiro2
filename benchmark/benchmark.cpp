
// MIT License
//
// Copyright (c) 2018 degski
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <ciso646>
#include <cstddef>

#include "iostream.hpp" // <iostream> + nl, sp etc. defined...
#include <random>


#include <benchmark/benchmark.h>

#ifdef _WIN32
#pragma comment ( lib, "Shlwapi.lib" )
#endif

#include "../pcg/pcg_random.hpp"
#include "../xoroshiro.hpp"
#include "../xoroshiro128plusxoshi.hpp"
#include "../splitmix.hpp"
#include "../sfc.hpp"
#include "../lehmer.hpp"


template<class Gen>
void benchmark_generator_clobber ( benchmark::State & state ) noexcept {
    typename Gen::result_type a = 0;
    Gen gen ( 0xBE1C0467EBA5FAC );
    for ( auto _ : state ) {
        benchmark::DoNotOptimize ( &a );
        for ( int i = 0; i < 100; ++i ) {
            a += gen ( );
            benchmark::ClobberMemory ( );
        }
    }
}

template<class Gen>
void benchmark_generator_no_clobber ( benchmark::State & state ) noexcept {
    typename Gen::result_type a = 0;
    Gen gen ( 0xBE1C0467EBA5FAC );
    for ( auto _ : state ) {
        benchmark::DoNotOptimize ( &a );
        for ( int i = 0; i < 100; ++i ) {
            a += gen ( );
        }
    }
}


constexpr int repeats = 8;


BENCHMARK_TEMPLATE ( benchmark_generator_clobber, pcg64 )
->Repetitions ( repeats )
->ReportAggregatesOnly ( true );

BENCHMARK_TEMPLATE ( benchmark_generator_clobber, sfc64 )
->Repetitions ( repeats )
->ReportAggregatesOnly ( true );

BENCHMARK_TEMPLATE ( benchmark_generator_clobber, meo::xoroshiro128plus64 )
->Repetitions ( repeats )
->ReportAggregatesOnly ( true );

BENCHMARK_TEMPLATE ( benchmark_generator_clobber, degski::xoroshiro128plus64xoshi32 )
->Repetitions ( repeats )
->ReportAggregatesOnly ( true );

BENCHMARK_TEMPLATE ( benchmark_generator_clobber, degski::xoroshiro128plus64xoshi32starxoshi32 )
->Repetitions ( repeats )
->ReportAggregatesOnly ( true );

BENCHMARK_TEMPLATE ( benchmark_generator_clobber, mcg128 )
->Repetitions ( repeats )
->ReportAggregatesOnly ( true );

BENCHMARK_TEMPLATE ( benchmark_generator_clobber, mcg128_fast )
->Repetitions ( repeats )
->ReportAggregatesOnly ( true );

BENCHMARK_TEMPLATE ( benchmark_generator_clobber, splitmix64 )
->Repetitions ( repeats )
->ReportAggregatesOnly ( true );


BENCHMARK_TEMPLATE ( benchmark_generator_no_clobber, pcg64 )
->Repetitions ( repeats )
->ReportAggregatesOnly ( true );

BENCHMARK_TEMPLATE ( benchmark_generator_no_clobber, sfc64 )
->Repetitions ( repeats )
->ReportAggregatesOnly ( true );

BENCHMARK_TEMPLATE ( benchmark_generator_no_clobber, meo::xoroshiro128plus64 )
->Repetitions ( repeats )
->ReportAggregatesOnly ( true );

BENCHMARK_TEMPLATE ( benchmark_generator_no_clobber, degski::xoroshiro128plus64xoshi32 )
->Repetitions ( repeats )
->ReportAggregatesOnly ( true );

BENCHMARK_TEMPLATE ( benchmark_generator_no_clobber, degski::xoroshiro128plus64xoshi32starxoshi32 )
->Repetitions ( repeats )
->ReportAggregatesOnly ( true );

BENCHMARK_TEMPLATE ( benchmark_generator_no_clobber, mcg128 )
->Repetitions ( repeats )
->ReportAggregatesOnly ( true );

BENCHMARK_TEMPLATE ( benchmark_generator_no_clobber, mcg128_fast )
->Repetitions ( repeats )
->ReportAggregatesOnly ( true );

BENCHMARK_TEMPLATE ( benchmark_generator_no_clobber, splitmix64 )
->Repetitions ( repeats )
->ReportAggregatesOnly ( true );
