
# xoroshiro128plusxoshi and xoroshiro128plusxoshi32starxoshi32

`xoroshiro128plus (as per Vigna & Blackman 2018) + xoshi(N)-back-end`, better (fails `practrand` at 64 gigabytes) and slightly faster (2%) (Intel Ci3-5005U (Broadwell) CPU), compiler [`LLVM-7.0.0-r336178-win64`](https://llvm.org/builds/).

`xoroshiro128plus (as per Vigna & Blackman 2018) + xoshi32starxoshi(N)-back-end`, does not fail 'practrand' up til 32 terabytes, while testing in one way results in that this generator is 8% faster, while being tested differently 31% slower, as compared to `xoroshiro128plus`. A real world situation is probably somewhere in between the 2 testing methods.

Included in this project are 2 back-ended `xoroshiro128plus` implementations of the vanilla (my reference) implementation by Melissa E. O'Neill, the author of the [pcg](http://www.pcg-random.org/) familiy of prng's. Don't miss her [blog](http://www.pcg-random.org/blog/), which is a throve of information (and the code, of most this project is written by her and did, for me, away with worrying about DIY-implementations) and is updated regulary.


## TL;DR

### xoroshiro128plusxoshi

The output function of `xoroshiro128plus` has been modified as per below (xoroshiro128plus + back-end):

    rtype operator ( ) ( )
    {
        const itype result = base::s0_ + base::s1_;

        base::advance ( );

        // Vigna & Blackman:
        // return result >> ( base::ITYPE_BITS - base::RTYPE_BITS );

        // degski:
        return ( ( result >> 32 ) ^ result ) >> ( base::ITYPE_BITS - base::RTYPE_BITS );
    }

The above modification of `xoroshiro128plus64-v1` is designated `xoroshiro128plusxoshi32`.

This generator fails `practrand` systematically at 64 gigabytes [BRank(12)], also with different shift (above 32) values.

### xoroshiro128plusxoshi32starxoshi32

I have "designed" (based on the idea of [invertible integer hash functions](https://gist.github.com/degski/6e2069d6035ae04d5d6f64981c995ec2)) and tested another variant of the `xoroshiro128plus` generator, `xoroshiro128plusxoshi32starxoshi32` (xoroshiro128plus + back-end):

    rtype operator ( ) ( )
    {
        itype result = base::s0_ + base::s1_;

        base::advance ( );

        // Vigna & Blackman:
        // return result >> ( base::ITYPE_BITS - base::RTYPE_BITS );

        // degski:
        result = ( ( result >> 32 ) ^ result ) * itype { 0x1AEC805299990163 };
        return ( ( result >> 32 ) ^ result ) >> ( base::ITYPE_BITS - base::RTYPE_BITS );
    }

The actual implementation tested deviates slightly (see [code](https://github.com/degski/xoroshiro128plusxoshi/blob/master/xoroshiro128plusxoshi.hpp)) from the above (order of operations), but my guess is that it all don't matter after optimization.
This generator does not fail the `practrand` test up til and including 32 terabytes, after which (8.4 days of testing) I have stopped the test.


## Results

### Speed-test

#### Context

* Intel Ci3-5005U (Broadwell) CPU
* Windows 10-1803-x64 in `safe mode` (minimal).
* Compiler: [`LLVM-7.0.0-r336178-win64`](http://prereleases.llvm.org/win-snapshots/LLVM-7.0.0-r336178-win64.exe)
* Command-line: `clang-cl.exe -fuse-ld=lld -flto=thin  /D "NDEBUG" /D "_CONSOLE" /D "NOMINMAX" /D "_UNICODE" /D "UNICODE" -Xclang -fcxx-exceptions /Ox /Oi /MT benchmark.cpp -Xclang -std=c++2a -Xclang -ffast-math -mmmx  -msse  -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mavx -mavx2`

#### Speed-test `google benchmark` results

**Test functions:**

     void bm_generator_clobber ( benchmark::State & state ) noexcept {
        static std::uint64_t seed = 0xBE1C0467EBA5FAC;
        seed *= 0x1AEC805299990163;
        seed ^= ( seed >> 32 );
        Gen gen ( seed );
        typename Gen::result_type a = 0;
        benchmark::DoNotOptimize ( &a );
        for ( auto _ : state ) {
            for ( int i = 0; i < 128; ++i ) {
                a += gen ( );
                benchmark::ClobberMemory ( );
            }
        }
    }

    template<class Gen>
    void bm_generator_no_clobber ( benchmark::State & state ) noexcept {
        static std::uint64_t seed = 0xBE1C0467EBA5FAC;
        seed *= 0x1AEC805299990163;
        seed ^= ( seed >> 32 );
        Gen gen ( seed );
        typename Gen::result_type a = 0;
        benchmark::DoNotOptimize ( &a );
        for ( auto _ : state ) {
            for ( int i = 0; i < 128; ++i ) {
                a += gen ( );
            }
        }
    }

**The numbers:** (ordered by descending `Time`)

    07/20/18 15:31:50
    Running benchmark.exe
    Run on (4 X 1995 MHz CPU s)
    CPU Caches:
      L1 Data 32K (x2)
      L1 Instruction 32K (x2)
      L2 Unified 262K (x2)
      L3 Unified 3145K (x1)
    -------------------------------------------------------------------------------------------------------------------------------
    Benchmark                                                                                        Time           CPU Iterations
    -------------------------------------------------------------------------------------------------------------------------------
    bm_generator_clobber<std::mt19937_64>/repeats:16_mean                                          966 ns        967 ns     746667
    bm_generator_clobber<std::mt19937_64>/repeats:16_median                                        966 ns        963 ns     746667
    bm_generator_clobber<std::mt19937_64>/repeats:16_stddev                                          1 ns          8 ns     746667
    bm_generator_clobber<sfc64>/repeats:16_mean                                                    562 ns        561 ns    1120000
    bm_generator_clobber<sfc64>/repeats:16_median                                                  561 ns        558 ns    1120000
    bm_generator_clobber<sfc64>/repeats:16_stddev                                                    1 ns          8 ns    1120000
    bm_generator_clobber<pcg64>/repeats:16_mean                                                    506 ns        507 ns    1000000
    bm_generator_clobber<pcg64>/repeats:16_median                                                  506 ns        500 ns    1000000
    bm_generator_clobber<pcg64>/repeats:16_stddev                                                    0 ns          8 ns    1000000
    bm_generator_clobber<meo::xoroshiro128plus64>/repeats:16_mean                                  448 ns        448 ns    1544828
    bm_generator_clobber<meo::xoroshiro128plus64>/repeats:16_median                                447 ns        445 ns    1544828
    bm_generator_clobber<meo::xoroshiro128plus64>/repeats:16_stddev                                  5 ns          8 ns    1544828
    bm_generator_clobber<degski::xoroshiro128plus64xoshi32>/repeats:16_mean                        442 ns        443 ns    1659259
    bm_generator_clobber<degski::xoroshiro128plus64xoshi32>/repeats:16_median                      443 ns        443 ns    1659259
    bm_generator_clobber<degski::xoroshiro128plus64xoshi32>/repeats:16_stddev                        5 ns          7 ns    1659259
    bm_generator_clobber<mcg128>/repeats:16_mean                                                   414 ns        413 ns    1659259
    bm_generator_clobber<mcg128>/repeats:16_median                                                 414 ns        414 ns    1659259
    bm_generator_clobber<mcg128>/repeats:16_stddev                                                   0 ns          3 ns    1659259
    bm_generator_clobber<splitmix64>/repeats:16_mean                                               412 ns        412 ns    1723077
    bm_generator_clobber<splitmix64>/repeats:16_median                                             412 ns        408 ns    1723077
    bm_generator_clobber<splitmix64>/repeats:16_stddev                                               0 ns          5 ns    1723077
    bm_generator_clobber<degski::xoroshiro128plus64xoshi32starxoshi32>/repeats:16_mean             410 ns        410 ns    1723077
    bm_generator_clobber<degski::xoroshiro128plus64xoshi32starxoshi32>/repeats:16_median           410 ns        408 ns    1723077
    bm_generator_clobber<degski::xoroshiro128plus64xoshi32starxoshi32>/repeats:16_stddev             0 ns          4 ns    1723077
    bm_generator_clobber<mcg128_fast>/repeats:16_mean                                              410 ns        409 ns    1723077
    bm_generator_clobber<mcg128_fast>/repeats:16_median                                            410 ns        408 ns    1723077
    bm_generator_clobber<mcg128_fast>/repeats:16_stddev                                              0 ns          5 ns    1723077
    
    bm_generator_no_clobber<std::mt19937_64>/repeats:16_mean                                       957 ns        957 ns     746667
    bm_generator_no_clobber<std::mt19937_64>/repeats:16_median                                     957 ns        963 ns     746667
    bm_generator_no_clobber<std::mt19937_64>/repeats:16_stddev                                       1 ns          9 ns     746667
    bm_generator_no_clobber<pcg64>/repeats:16_mean                                                 515 ns        515 ns    1120000
    bm_generator_no_clobber<pcg64>/repeats:16_median                                               515 ns        516 ns    1120000
    bm_generator_no_clobber<pcg64>/repeats:16_stddev                                                 1 ns          3 ns    1120000
    bm_generator_no_clobber<degski::xoroshiro128plus64xoshi32starxoshi32>/repeats:16_mean          377 ns        378 ns    1866667
    bm_generator_no_clobber<degski::xoroshiro128plus64xoshi32starxoshi32>/repeats:16_median        377 ns        377 ns    1866667
    bm_generator_no_clobber<degski::xoroshiro128plus64xoshi32starxoshi32>/repeats:16_stddev          0 ns          3 ns    1866667
    bm_generator_no_clobber<meo::xoroshiro128plus64>/repeats:16_mean                               288 ns        287 ns    2357895
    bm_generator_no_clobber<meo::xoroshiro128plus64>/repeats:16_median                             287 ns        285 ns    2357895
    bm_generator_no_clobber<meo::xoroshiro128plus64>/repeats:16_stddev                               0 ns          4 ns    2357895
    bm_generator_no_clobber<sfc64>/repeats:16_mean                                                 287 ns        287 ns    2488889
    bm_generator_no_clobber<sfc64>/repeats:16_median                                               287 ns        289 ns    2488889
    bm_generator_no_clobber<sfc64>/repeats:16_stddev                                                 0 ns          3 ns    2488889
    bm_generator_no_clobber<degski::xoroshiro128plus64xoshi32>/repeats:16_mean                     281 ns        281 ns    2488889
    bm_generator_no_clobber<degski::xoroshiro128plus64xoshi32>/repeats:16_median                   280 ns        283 ns    2488889
    bm_generator_no_clobber<degski::xoroshiro128plus64xoshi32>/repeats:16_stddev                     0 ns          3 ns    2488889
    bm_generator_no_clobber<splitmix64>/repeats:16_mean                                            270 ns        270 ns    2635294
    bm_generator_no_clobber<splitmix64>/repeats:16_median                                          270 ns        270 ns    2635294
    bm_generator_no_clobber<splitmix64>/repeats:16_stddev                                            0 ns          3 ns    2635294
    bm_generator_no_clobber<mcg128>/repeats:16_mean                                                262 ns        261 ns    2635294
    bm_generator_no_clobber<mcg128>/repeats:16_median                                              262 ns        261 ns    2635294
    bm_generator_no_clobber<mcg128>/repeats:16_stddev                                                0 ns          2 ns    2635294
    bm_generator_no_clobber<mcg128_fast>/repeats:16_mean                                           219 ns        219 ns    3200000
    bm_generator_no_clobber<mcg128_fast>/repeats:16_median                                         214 ns        212 ns    3200000
    bm_generator_no_clobber<mcg128_fast>/repeats:16_stddev                                          12 ns         11 ns    3200000


### Practrand results

#### pcg64

pcg64 [passes](http://www.pcg-random.org/posts/pcg-passes-practrand.html) the `practrand` test.


#### sfc64

Not tested.


#### xoroshiro128plus64-v1

    RNG_test using PractRand version 0.93
    RNG = RNG_stdin64, seed = 0x6259a10f
    test set = normal, folding = standard (64 bit)

    rng=RNG_stdin64, seed=0x6259a10f
    length= 128 megabytes (2^27 bytes), time= 3.9 seconds
      Test Name                         Raw       Processed     Evaluation
      [Low1/64]BRank(12):256(2)         R= +3748  p~=  3e-1129    FAIL !!!!!!!!
      [Low1/64]BRank(12):384(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      ...and 146 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x6259a10f
    length= 256 megabytes (2^28 bytes), time= 9.3 seconds
      Test Name                         Raw       Processed     Evaluation
      [Low4/64]BRank(12):768(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
      [Low1/64]BRank(12):256(2)         R= +3748  p~=  3e-1129    FAIL !!!!!!!!
      [Low1/64]BRank(12):384(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low1/64]BRank(12):512(1)         R= +8161  p~=  1e-2457    FAIL !!!!!!!!
      ...and 155 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x6259a10f
    length= 512 megabytes (2^29 bytes), time= 18.4 seconds
      Test Name                         Raw       Processed     Evaluation
      [Low4/64]BRank(12):768(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
      [Low4/64]BRank(12):1K(1)          R= +2650  p~=  9.8e-799   FAIL !!!!!!!
      [Low1/64]Gap-16:A                 R=  -3.5  p =1-1.5e-3   unusual
      [Low1/64]BRank(12):256(2)         R= +3748  p~=  3e-1129    FAIL !!!!!!!!
      [Low1/64]BRank(12):384(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low1/64]BRank(12):512(1)         R= +8161  p~=  1e-2457    FAIL !!!!!!!!
      ...and 163 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x6259a10f
    length= 1 gigabyte (2^30 bytes), time= 35.7 seconds
      Test Name                         Raw       Processed     Evaluation
      [Low4/64]BRank(12):768(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
      [Low4/64]BRank(12):1K(1)          R= +2650  p~=  9.8e-799   FAIL !!!!!!!
      [Low1/64]DC6-9x1Bytes-1           R=  +5.4  p =  8.8e-3   unusual
      [Low1/64]BRank(12):256(2)         R= +3748  p~=  3e-1129    FAIL !!!!!!!!
      [Low1/64]BRank(12):384(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low1/64]BRank(12):512(2)         R=+11541  p~=  2e-3475    FAIL !!!!!!!!
      [Low1/64]BRank(12):768(1)         R=+13672  p~=  1e-4116    FAIL !!!!!!!!
      ...and 173 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x6259a10f
    length= 2 gigabytes (2^31 bytes), time= 63.2 seconds
      Test Name                         Raw       Processed     Evaluation
      [Low4/64]BRank(12):768(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
      [Low4/64]BRank(12):1K(2)          R= +3748  p~=  3e-1129    FAIL !!!!!!!!
      [Low4/64]BRank(12):1536(1)        R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low1/64]BRank(12):256(2)         R= +3748  p~=  3e-1129    FAIL !!!!!!!!
      [Low1/64]BRank(12):384(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low1/64]BRank(12):512(2)         R=+11541  p~=  2e-3475    FAIL !!!!!!!!
      [Low1/64]BRank(12):768(1)         R=+13672  p~=  1e-4116    FAIL !!!!!!!!
      [Low1/64]BRank(12):1K(1)          R=+19183  p~=  1e-5775    FAIL !!!!!!!!
      ...and 183 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x6259a10f
    length= 4 gigabytes (2^32 bytes), time= 121 seconds
      Test Name                         Raw       Processed     Evaluation
      [Low16/64]BRank(12):3K(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
      [Low4/64]BRank(12):768(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
      [Low4/64]BRank(12):1K(2)          R= +3748  p~=  3e-1129    FAIL !!!!!!!!
      [Low4/64]BRank(12):1536(1)        R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low4/64]BRank(12):2K(1)          R= +8161  p~=  1e-2457    FAIL !!!!!!!!
      [Low1/64]BRank(12):256(2)         R= +3748  p~=  3e-1129    FAIL !!!!!!!!
      [Low1/64]BRank(12):384(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low1/64]BRank(12):512(2)         R=+11541  p~=  2e-3475    FAIL !!!!!!!!
      [Low1/64]BRank(12):768(1)         R=+13672  p~=  1e-4116    FAIL !!!!!!!!
      [Low1/64]BRank(12):1K(1)          R=+19183  p~=  1e-5775    FAIL !!!!!!!!
      ...and 191 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x6259a10f
    length= 8 gigabytes (2^33 bytes), time= 248 seconds
      Test Name                         Raw       Processed     Evaluation
      [Low16/64]FPF-14+6/16:cross       R=  +4.0  p =  1.0e-3   unusual
      [Low16/64]BRank(12):3K(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
      [Low16/64]BRank(12):4K(1)         R= +2650  p~=  9.8e-799   FAIL !!!!!!!
      [Low4/64]DC6-9x1Bytes-1           R=  -4.1  p =1-5.6e-3   unusual
      [Low4/64]BRank(12):768(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
      [Low4/64]BRank(12):1K(2)          R= +3748  p~=  3e-1129    FAIL !!!!!!!!
      [Low4/64]BRank(12):1536(1)        R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low4/64]BRank(12):2K(1)          R= +8161  p~=  1e-2457    FAIL !!!!!!!!
      [Low1/64]BRank(12):256(4)         R= +5300  p~=  1e-2819    FAIL !!!!!!!!
      [Low1/64]BRank(12):384(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low1/64]BRank(12):512(2)         R=+11541  p~=  2e-3475    FAIL !!!!!!!!
      [Low1/64]BRank(12):768(1)         R=+13672  p~=  1e-4116    FAIL !!!!!!!!
      [Low1/64]BRank(12):1K(2)          R=+27129  p~=  1e-8167    FAIL !!!!!!!!
      [Low1/64]BRank(12):1536(1)        R=+30205  p~=  1e-9093    FAIL !!!!!!!!
      ...and 198 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x6259a10f
    length= 16 gigabytes (2^34 bytes), time= 467 seconds
      Test Name                         Raw       Processed     Evaluation
      [Low16/64]BRank(12):3K(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
      [Low16/64]BRank(12):4K(1)         R= +2650  p~=  9.8e-799   FAIL !!!!!!!
      [Low4/64]BRank(12):768(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
      [Low4/64]BRank(12):1K(2)          R= +3748  p~=  3e-1129    FAIL !!!!!!!!
      [Low4/64]BRank(12):1536(1)        R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low4/64]BRank(12):2K(2)          R=+11541  p~=  2e-3475    FAIL !!!!!!!!
      [Low4/64]BRank(12):3K(1)          R=+13672  p~=  1e-4116    FAIL !!!!!!!!
      [Low1/64]BRank(12):256(4)         R= +5300  p~=  1e-2819    FAIL !!!!!!!!
      [Low1/64]BRank(12):384(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low1/64]BRank(12):512(2)         R=+11541  p~=  2e-3475    FAIL !!!!!!!!
      [Low1/64]BRank(12):768(1)         R=+13672  p~=  1e-4116    FAIL !!!!!!!!
      [Low1/64]BRank(12):1K(2)          R=+27129  p~=  1e-8167    FAIL !!!!!!!!
      [Low1/64]BRank(12):1536(1)        R=+30205  p~=  1e-9093    FAIL !!!!!!!!
      [Low1/64]BRank(12):2K(1)          R=+41227  p~= 0           FAIL !!!!!!!!
      ...and 209 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x6259a10f
    length= 32 gigabytes (2^35 bytes), time= 767 seconds
      Test Name                         Raw       Processed     Evaluation
      [Low16/64]BRank(12):3K(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
      [Low16/64]BRank(12):4K(2)         R= +3748  p~=  3e-1129    FAIL !!!!!!!!
      [Low16/64]BRank(12):6K(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low4/64]BRank(12):768(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
      [Low4/64]BRank(12):1K(2)          R= +3748  p~=  3e-1129    FAIL !!!!!!!!
      [Low4/64]BRank(12):1536(1)        R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low4/64]BRank(12):2K(2)          R=+11541  p~=  2e-3475    FAIL !!!!!!!!
      [Low4/64]BRank(12):3K(1)          R=+13672  p~=  1e-4116    FAIL !!!!!!!!
      [Low4/64]BRank(12):4K(1)          R=+19183  p~=  1e-5775    FAIL !!!!!!!!
      [Low1/64]BRank(12):256(4)         R= +5300  p~=  1e-2819    FAIL !!!!!!!!
      [Low1/64]BRank(12):384(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low1/64]BRank(12):512(2)         R=+11541  p~=  2e-3475    FAIL !!!!!!!!
      [Low1/64]BRank(12):768(1)         R=+13672  p~=  1e-4116    FAIL !!!!!!!!
      [Low1/64]BRank(12):1K(2)          R=+27129  p~=  1e-8167    FAIL !!!!!!!!
      [Low1/64]BRank(12):1536(1)        R=+30205  p~=  1e-9093    FAIL !!!!!!!!
      [Low1/64]BRank(12):2K(1)          R=+41227  p~= 0           FAIL !!!!!!!!
      ...and 217 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x6259a10f
    length= 64 gigabytes (2^36 bytes), time= 1370 seconds
      Test Name                         Raw       Processed     Evaluation
      BRank(12):12K(1)                  R= +1272  p~=  5.4e-384   FAIL !!!!!!!
      [Low16/64]BRank(12):3K(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
      [Low16/64]BRank(12):4K(2)         R= +3748  p~=  3e-1129    FAIL !!!!!!!!
      [Low16/64]BRank(12):6K(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low16/64]BRank(12):8K(1)         R= +8161  p~=  1e-2457    FAIL !!!!!!!!
      [Low4/64]BRank(12):768(1)         R= +1272  p~=  5.4e-384   FAIL !!!!!!!
      [Low4/64]BRank(12):1K(2)          R= +3748  p~=  3e-1129    FAIL !!!!!!!!
      [Low4/64]BRank(12):1536(1)        R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low4/64]BRank(12):2K(2)          R=+11541  p~=  2e-3475    FAIL !!!!!!!!
      [Low4/64]BRank(12):3K(1)          R=+13672  p~=  1e-4116    FAIL !!!!!!!!
      [Low4/64]BRank(12):4K(1)          R=+19183  p~=  1e-5775    FAIL !!!!!!!!
      [Low1/64]BRank(12):256(4)         R= +5300  p~=  1e-2819    FAIL !!!!!!!!
      [Low1/64]BRank(12):384(1)         R= +5405  p~=  3e-1628    FAIL !!!!!!!!
      [Low1/64]BRank(12):512(4)         R=+16322  p~=  1e-8681    FAIL !!!!!!!!
      [Low1/64]BRank(12):768(1)         R=+13672  p~=  1e-4116    FAIL !!!!!!!!
      [Low1/64]BRank(12):1K(2)          R=+27129  p~=  1e-8167    FAIL !!!!!!!!
      [Low1/64]BRank(12):1536(1)        R=+30205  p~=  1e-9093    FAIL !!!!!!!!
      [Low1/64]BRank(12):2K(2)          R=+58303  p~= 0           FAIL !!!!!!!!
      [Low1/64]BRank(12):3K(1)          R=+63271  p~= 0           FAIL !!!!!!!!
      ...and 225 test result(s) without anomalies


#### xoroshiro128plusxoshi32

    RNG_test using PractRand version 0.93
    RNG = RNG_stdin64, seed = 0x47700e2c
    test set = normal, folding = standard (64 bit)

    rng=RNG_stdin64, seed=0x47700e2c
    length= 128 megabytes (2^27 bytes), time= 2.3 seconds
      Test Name                         Raw       Processed     Evaluation
      [Low1/64]BCFN(2+0,13-6,T)         R=  -5.7  p =1-1.1e-3   unusual
      ...and 147 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x47700e2c
    length= 256 megabytes (2^28 bytes), time= 5.4 seconds
      no anomalies in 159 test result(s)

    rng=RNG_stdin64, seed=0x47700e2c
    length= 512 megabytes (2^29 bytes), time= 10.6 seconds
      no anomalies in 169 test result(s)

    rng=RNG_stdin64, seed=0x47700e2c
    length= 1 gigabyte (2^30 bytes), time= 20.9 seconds
      no anomalies in 180 test result(s)

    rng=RNG_stdin64, seed=0x47700e2c
    length= 2 gigabytes (2^31 bytes), time= 42.4 seconds
      no anomalies in 191 test result(s)

    rng=RNG_stdin64, seed=0x47700e2c
    length= 4 gigabytes (2^32 bytes), time= 77.5 seconds
      no anomalies in 201 test result(s)

    rng=RNG_stdin64, seed=0x47700e2c
    length= 8 gigabytes (2^33 bytes), time= 149 seconds
      Test Name                         Raw       Processed     Evaluation
      BCFN(2+2,13-0,T)                  R=  +9.4  p =  1.4e-4   mildly suspicious
      Gap-16:B                          R=  +4.4  p =  1.1e-3   unusual
      ...and 210 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x47700e2c
    length= 16 gigabytes (2^34 bytes), time= 291 seconds
      no anomalies in 223 test result(s)

    rng=RNG_stdin64, seed=0x47700e2c
    length= 32 gigabytes (2^35 bytes), time= 577 seconds
      no anomalies in 233 test result(s)

    rng=RNG_stdin64, seed=0x47700e2c
    length= 64 gigabytes (2^36 bytes), time= 1162 seconds
      Test Name                         Raw       Processed     Evaluation
      BRank(12):12K(1)                  R= +1272  p~=  5.4e-384   FAIL !!!!!!!
      ...and 243 test result(s) without anomalies


#### xoroshiro128plusxoshi32starxoshi32

    RNG_test using PractRand version 0.93
    RNG = RNG_stdin64, seed = 0x9b09af3
    test set = normal, folding = standard (64 bit)

    rng=RNG_stdin64, seed=0x9b09af3
    length= 128 megabytes (2^27 bytes), time= 2.3 seconds
      no anomalies in 148 test result(s)

    rng=RNG_stdin64, seed=0x9b09af3
    length= 256 megabytes (2^28 bytes), time= 5.5 seconds
      no anomalies in 159 test result(s)

    rng=RNG_stdin64, seed=0x9b09af3
    length= 512 megabytes (2^29 bytes), time= 10.7 seconds
      no anomalies in 169 test result(s)

    rng=RNG_stdin64, seed=0x9b09af3
    length= 1 gigabyte (2^30 bytes), time= 20.6 seconds
      no anomalies in 180 test result(s)

    rng=RNG_stdin64, seed=0x9b09af3
    length= 2 gigabytes (2^31 bytes), time= 39.4 seconds
      no anomalies in 191 test result(s)

    rng=RNG_stdin64, seed=0x9b09af3
    length= 4 gigabytes (2^32 bytes), time= 75.1 seconds
      no anomalies in 201 test result(s)

    rng=RNG_stdin64, seed=0x9b09af3
    length= 8 gigabytes (2^33 bytes), time= 149 seconds
      Test Name                         Raw       Processed     Evaluation
      BCFN(2+7,13-2,T)                  R=  -8.3  p =1-1.3e-4   unusual
      [Low4/64]Gap-16:B                 R=  +5.4  p =  1.3e-4   mildly suspicious
      ...and 210 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x9b09af3
    length= 16 gigabytes (2^34 bytes), time= 294 seconds
      no anomalies in 223 test result(s)

    rng=RNG_stdin64, seed=0x9b09af3
    length= 32 gigabytes (2^35 bytes), time= 581 seconds
      no anomalies in 233 test result(s)

    rng=RNG_stdin64, seed=0x9b09af3
    length= 64 gigabytes (2^36 bytes), time= 1159 seconds
      no anomalies in 244 test result(s)

    rng=RNG_stdin64, seed=0x9b09af3
    length= 128 gigabytes (2^37 bytes), time= 2328 seconds
      no anomalies in 255 test result(s)
    
    rng=RNG_stdin64, seed=0x9b09af3
    length= 256 gigabytes (2^38 bytes), time= 4597 seconds
      no anomalies in 265 test result(s)

    rng=RNG_stdin64, seed=0x9b09af3
    length= 512 gigabytes (2^39 bytes), time= 9355 seconds
      Test Name                         Raw       Processed     Evaluation
      DC6-9x1Bytes-1                    R=  +5.5  p =  8.8e-3   unusual
      ...and 275 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x9b09af3
    length= 1 terabyte (2^40 bytes), time= 20022 seconds
      no anomalies in 287 test result(s)

    rng=RNG_stdin64, seed=0x9b09af3
    length= 2 terabytes (2^41 bytes), time= 39005 seconds
      no anomalies in 297 test result(s)

    rng=RNG_stdin64, seed=0x9b09af3
    length= 4 terabytes (2^42 bytes), time= 80902 seconds
      no anomalies in 308 test result(s)

    rng=RNG_stdin64, seed=0x9b09af3
    length= 8 terabytes (2^43 bytes), time= 177391 seconds
      no anomalies in 319 test result(s)
    
    rng=RNG_stdin64, seed=0x9b09af3
    length= 16 terabytes (2^44 bytes), time= 355419 seconds
      no anomalies in 329 test result(s)

    rng=RNG_stdin64, seed=0x9b09af3
    length= 32 terabytes (2^45 bytes), time= 727512 seconds
      no anomalies in 339 test result(s)

32 terabytes barrier broken! giving up ...


#### mcg128

    RNG_test using PractRand version 0.94
    RNG = RNG_stdin64, seed = unknown
    test set = core, folding = standard (64 bit)

    rng=RNG_stdin64, seed=unknown
    length= 128 megabytes (2^27 bytes), time= 2.6 seconds
      no anomalies in 193 test result(s)

    rng=RNG_stdin64, seed=unknown
    length= 256 megabytes (2^28 bytes), time= 6.0 seconds
      no anomalies in 210 test result(s)

    rng=RNG_stdin64, seed=unknown
    length= 512 megabytes (2^29 bytes), time= 11.8 seconds
      no anomalies in 226 test result(s)

    rng=RNG_stdin64, seed=unknown
    length= 1 gigabyte (2^30 bytes), time= 22.9 seconds
      no anomalies in 243 test result(s)

    rng=RNG_stdin64, seed=unknown
    length= 2 gigabytes (2^31 bytes), time= 44.4 seconds
      Test Name                         Raw       Processed     Evaluation
      [Low1/64]mod3n(5):(1,9-3)         R=  +9.1  p =  7.2e-5   unusual
      ...and 260 test result(s) without anomalies

    rng=RNG_stdin64, seed=unknown
    length= 4 gigabytes (2^32 bytes), time= 84.9 seconds
      no anomalies in 277 test result(s)

    rng=RNG_stdin64, seed=unknown
    length= 8 gigabytes (2^33 bytes), time= 171 seconds
      no anomalies in 294 test result(s)

    rng=RNG_stdin64, seed=unknown
    length= 16 gigabytes (2^34 bytes), time= 330 seconds
      no anomalies in 310 test result(s)

    rng=RNG_stdin64, seed=unknown
    length= 32 gigabytes (2^35 bytes), time= 653 seconds
      no anomalies in 325 test result(s)

    rng=RNG_stdin64, seed=unknown
    length= 64 gigabytes (2^36 bytes), time= 1338 seconds
      Test Name                         Raw       Processed     Evaluation
      TMFn(2+11):wl                     R= +62.0  p~=   3e-30     FAIL !!!
      [Low16/64]TMFn(2+9):wl            R= +62.7  p~=   1e-30     FAIL !!!
      [Low4/64]TMFn(2+7):wl             R= +62.9  p~=   8e-31     FAIL !!!
      ...and 337 test result(s) without anomalies


#### mcg128_fast

    RNG_test using PractRand version 0.94
    RNG = RNG_stdin64, seed = unknown
    test set = core, folding = standard (64 bit)

    rng=RNG_stdin64, seed=unknown
    length= 128 megabytes (2^27 bytes), time= 2.6 seconds
      no anomalies in 193 test result(s)

    rng=RNG_stdin64, seed=unknown
    length= 256 megabytes (2^28 bytes), time= 6.0 seconds
      no anomalies in 210 test result(s)

    rng=RNG_stdin64, seed=unknown
    length= 512 megabytes (2^29 bytes), time= 11.7 seconds
      no anomalies in 226 test result(s)

    rng=RNG_stdin64, seed=unknown
    length= 1 gigabyte (2^30 bytes), time= 23.1 seconds
      no anomalies in 243 test result(s)

    rng=RNG_stdin64, seed=unknown
    length= 2 gigabytes (2^31 bytes), time= 44.1 seconds
      no anomalies in 261 test result(s)

    rng=RNG_stdin64, seed=unknown
    length= 4 gigabytes (2^32 bytes), time= 87.0 seconds
      no anomalies in 277 test result(s)

    rng=RNG_stdin64, seed=unknown
    length= 8 gigabytes (2^33 bytes), time= 168 seconds
      no anomalies in 294 test result(s)

    rng=RNG_stdin64, seed=unknown
    length= 16 gigabytes (2^34 bytes), time= 328 seconds
      Test Name                         Raw       Processed     Evaluation
      [Low16/64]DC6-9x1Bytes-1          R=  +5.8  p =  3.9e-3   unusual
      ...and 309 test result(s) without anomalies

    rng=RNG_stdin64, seed=unknown
    length= 32 gigabytes (2^35 bytes), time= 639 seconds
      no anomalies in 325 test result(s)

    rng=RNG_stdin64, seed=unknown
    length= 64 gigabytes (2^36 bytes), time= 1327 seconds
      Test Name                         Raw       Processed     Evaluation
      TMFn(2+11):wl                     R= +59.5  p~=   9e-29     FAIL !!
      [Low16/64]TMFn(2+9):wl            R= +59.4  p~=   1e-28     FAIL !!
      [Low4/64]TMFn(2+7):wl             R= +61.0  p~=   1e-29     FAIL !!!
      ...and 337 test result(s) without anomalies


#### splitmix64

splitmix64 [passes (link and work by meo)](https://pastebin.com/ut80JMMz) `bigcrush`. With `practrand` [`splitmix64` passes at least 64TB (in the comments)](https://lemire.me/blog/2017/08/22/testing-non-cryptographic-random-number-generators-my-results/).

    RNG_test using PractRand version 0.93
    RNG = RNG_stdin64, seed = 0x7c5c6b6d
    test set = normal, folding = standard (64 bit)

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 128 megabytes (2^27 bytes), time= 2.8 seconds
      Test Name                         Raw       Processed     Evaluation
      [Low16/64]Gap-16:B                R=  -4.1  p =1-1.7e-3   unusual
      ...and 147 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 256 megabytes (2^28 bytes), time= 6.3 seconds
      no anomalies in 159 test result(s)

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 512 megabytes (2^29 bytes), time= 12.9 seconds
      Test Name                         Raw       Processed     Evaluation
      [Low4/64]DC6-9x1Bytes-1           R=  -3.9  p =1-7.4e-3   unusual
      ...and 168 test result(s) without anomalies

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 1 gigabyte (2^30 bytes), time= 25.6 seconds
      no anomalies in 180 test result(s)

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 2 gigabytes (2^31 bytes), time= 49.2 seconds
      no anomalies in 191 test result(s)

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 4 gigabytes (2^32 bytes), time= 92.7 seconds
      no anomalies in 201 test result(s)

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 8 gigabytes (2^33 bytes), time= 185 seconds
      no anomalies in 212 test result(s)

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 16 gigabytes (2^34 bytes), time= 363 seconds
      no anomalies in 223 test result(s)

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 32 gigabytes (2^35 bytes), time= 706 seconds
      no anomalies in 233 test result(s)

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 64 gigabytes (2^36 bytes), time= 1438 seconds
      no anomalies in 244 test result(s)

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 128 gigabytes (2^37 bytes), time= 2871 seconds
      no anomalies in 255 test result(s)

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 256 gigabytes (2^38 bytes), time= 5548 seconds
      no anomalies in 265 test result(s)

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 512 gigabytes (2^39 bytes), time= 10984 seconds
      no anomalies in 276 test result(s)

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 1 terabyte (2^40 bytes), time= 20369 seconds
      no anomalies in 287 test result(s)

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 2 terabytes (2^41 bytes), time= 38575 seconds
      no anomalies in 297 test result(s)

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 4 terabytes (2^42 bytes), time= 84321 seconds
      no anomalies in 308 test result(s)

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 8 terabytes (2^43 bytes), time= 179099 seconds
      no anomalies in 319 test result(s)

    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 16 terabytes (2^44 bytes), time= 354333 seconds
      no anomalies in 329 test result(s)
    
    rng=RNG_stdin64, seed=0x7c5c6b6d
    length= 32 terabytes (2^45 bytes), time= 715044 seconds
      no anomalies in 339 test result(s)

Testing stopped (I declare it good for pong, maybe packman).
