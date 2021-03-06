
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

#pragma once

#include <cstddef>


template<typename Generator, typename DataType = typename Generator::result_type, std::size_t CacheLineSize = 64>
struct generator_cache : private Generator {

    using result_type = typename Generator::result_type;
    using size_type = std::size_t;

    generator_cache ( ) : Generator ( ) { }
    generator_cache ( result_type v ) : Generator ( v ) { }
    template<typename SeedSeq>
    generator_cache ( const SeedSeq & seq ) : Generator ( seq ) { }
    template<typename It>
    generator_cache ( It & first_, It last ) : Generator ( first_, last ) { }

    result_type operator ( ) ( ) noexcept { // can still be improved (do something else than shift if sizes differ)
        if ( m_index ) {
            return m_data [ m_index-- ] >> ( 8 * ( sizeof ( DataType ) - sizeof ( typename Generator::result_type ) ) ); // makes the order of the decrement free to schedule for the compiler.
        }
        else {
            Generator::generate ( std::begin ( m_data ), std::end ( m_data ) );
            m_index = data_size ( ) - 2;
            return m_data [ data_size ( ) - 1 ] >> ( 8 * ( sizeof ( DataType ) - sizeof ( typename Generator::result_type ) ) );
        }

    }

    static inline constexpr result_type min ( ) noexcept { return Generator::min ( ); }
    static inline constexpr result_type max ( ) noexcept { return Generator::max ( ); }
    static inline constexpr std::size_t const data_size ( ) noexcept { return CacheLineSize / sizeof ( DataType ); }

    DataType m_data [ data_size ( ) ];
    size_type m_index = data_size ( );
};
