/**
 * Copyright 2015 Kurt Kanzenbach
 * Copyright 2016-2017 Andreas Schäfer
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef FLAT_ARRAY_DETAIL_SHORT_VEC_AVX512_DOUBLE_16_HPP
#define FLAT_ARRAY_DETAIL_SHORT_VEC_AVX512_DOUBLE_16_HPP

#if LIBFLATARRAY_WIDEST_VECTOR_ISA == LIBFLATARRAY_AVX512F

// disable certain warnings from system headers when compiling with
// Microsoft Visual Studio:
#ifdef _MSC_BUILD
#pragma warning( push )
#pragma warning( disable : 4514 )
#endif

#include <immintrin.h>

#ifdef LIBFLATARRAY_WITH_CPP14
#include <initializer_list>
#endif

#ifdef _MSC_BUILD
#pragma warning( pop )
#endif

#include <libflatarray/detail/sqrt_reference.hpp>
#include <libflatarray/detail/short_vec_helpers.hpp>
#include <libflatarray/config.h>
#include <libflatarray/short_vec_base.hpp>

namespace LibFlatArray {

template<typename CARGO, std::size_t ARITY>
class short_vec;

#ifdef __ICC
// disabling this warning as implicit type conversion is exactly our goal here:
#pragma warning push
#pragma warning (disable: 2304)
#endif

template<>
class short_vec<double, 16> : public short_vec_base<double, 16>
{
public:
    static const std::size_t ARITY = 16;
    typedef unsigned short mask_type;

    typedef short_vec_strategy::avx512f strategy;

    template<typename _CharT, typename _Traits>
    friend std::basic_ostream<_CharT, _Traits>& operator<<(
        std::basic_ostream<_CharT, _Traits>& __os,
        const short_vec<double, 16>& vec);

    inline
    short_vec(const double data = 0) :
        val{_mm512_set1_pd(data),
            _mm512_set1_pd(data)}
    {}

    inline
    short_vec(const double *data)
    {
        load(data);
    }

    inline
    short_vec(const __m512d& val1, const __m512d& val2) :
        val{val1,
            val2}
    {}

#ifdef LIBFLATARRAY_WITH_CPP14
    inline
    short_vec(const std::initializer_list<double>& il)
    {
        const double *ptr = static_cast<const double *>(&(*il.begin()));
        load(ptr);
    }
#endif

    inline
    bool any() const
    {
        __m512d buf0 = _mm512_or_pd(val[ 0], val[ 1]);
        return _mm512_test_epi64_mask(
            _mm512_castpd_si512(buf0),
            _mm512_castpd_si512(buf0));
    }

    inline
    double operator[](int i) const
    {
        __m512d buf0 = val[i >> 3];
        i &= 7;

        __m128d buf1;
        if (i < 4) {
            if (i < 2) {
                buf1 = _mm512_extractf64x2_pd(buf0, 0);
            } else {
                buf1 = _mm512_extractf64x2_pd(buf0, 1);
            }
        } else {
            if (i < 6) {
                buf1 = _mm512_extractf64x2_pd(buf0, 2);
            } else {
                buf1 = _mm512_extractf64x2_pd(buf0, 3);
            }
        }

        i &= 1;

        if (i == 0) {
            return _mm_cvtsd_f64(buf1);
        }

        buf1 = _mm_shuffle_pd(buf1, buf1, 1);
        return _mm_cvtsd_f64(buf1);
    }

    inline
    void operator-=(const short_vec<double, 16>& other)
    {
        val[ 0] = _mm512_sub_pd(val[ 0], other.val[ 0]);
        val[ 1] = _mm512_sub_pd(val[ 1], other.val[ 1]);
    }

    inline
    short_vec<double, 16> operator-(const short_vec<double, 16>& other) const
    {
        return short_vec<double, 16>(
            _mm512_sub_pd(val[ 0], other.val[ 0]),
            _mm512_sub_pd(val[ 1], other.val[ 1]));
    }

    inline
    void operator+=(const short_vec<double, 16>& other)
    {
        val[ 0] = _mm512_add_pd(val[ 0], other.val[ 0]);
        val[ 1] = _mm512_add_pd(val[ 1], other.val[ 1]);
    }

    inline
    short_vec<double, 16> operator+(const short_vec<double, 16>& other) const
    {
        return short_vec<double, 16>(
            _mm512_add_pd(val[ 0], other.val[ 0]),
            _mm512_add_pd(val[ 1], other.val[ 1]));
    }

    inline
    void operator*=(const short_vec<double, 16>& other)
    {
        val[ 0] = _mm512_mul_pd(val[ 0], other.val[ 0]);
        val[ 1] = _mm512_mul_pd(val[ 1], other.val[ 1]);
    }

    inline
    short_vec<double, 16> operator*(const short_vec<double, 16>& other) const
    {
        return short_vec<double, 16>(
            _mm512_mul_pd(val[ 0], other.val[ 0]),
            _mm512_mul_pd(val[ 1], other.val[ 1]));
    }

    inline
    void operator/=(const short_vec<double, 16>& other)
    {
        val[ 0] = _mm512_div_pd(val[ 0], other.val[ 0]);
        val[ 1] = _mm512_div_pd(val[ 1], other.val[ 1]);
    }

    inline
    short_vec<double, 16> operator/(const short_vec<double, 16>& other) const
    {
        return short_vec<double, 16>(
            _mm512_div_pd(val[ 0], other.val[ 0]),
            _mm512_div_pd(val[ 1], other.val[ 1]));
    }

    inline
    mask_type operator<(const short_vec<double, 16>& other) const
    {
        return
            (_mm512_cmp_pd_mask(val[ 0], other.val[ 0], _CMP_LT_OS) <<  0) +
            (_mm512_cmp_pd_mask(val[ 1], other.val[ 1], _CMP_LT_OS) <<  8);
    }

    inline
    mask_type operator<=(const short_vec<double, 16>& other) const
    {
        return
            (_mm512_cmp_pd_mask(val[ 0], other.val[ 0], _CMP_LE_OS) <<  0) +
            (_mm512_cmp_pd_mask(val[ 1], other.val[ 1], _CMP_LE_OS) <<  8);
    }

    inline
    mask_type operator==(const short_vec<double, 16>& other) const
    {
        return
            (_mm512_cmp_pd_mask(val[ 0], other.val[ 0], _CMP_EQ_OQ) <<  0) +
            (_mm512_cmp_pd_mask(val[ 1], other.val[ 1], _CMP_EQ_OQ) <<  8);
    }

    inline
    mask_type operator>(const short_vec<double, 16>& other) const
    {
        return
            (_mm512_cmp_pd_mask(val[ 0], other.val[ 0], _CMP_GT_OS) <<  0) +
            (_mm512_cmp_pd_mask(val[ 1], other.val[ 1], _CMP_GT_OS) <<  8);
    }

    inline
    mask_type operator>=(const short_vec<double, 16>& other) const
    {
        return
            (_mm512_cmp_pd_mask(val[ 0], other.val[ 0], _CMP_GE_OS) <<  0) +
            (_mm512_cmp_pd_mask(val[ 1], other.val[ 1], _CMP_GE_OS) <<  8);
    }

    inline
    short_vec<double, 16> sqrt() const
    {
        return short_vec<double, 16>(
            _mm512_sqrt_pd(val[ 0]),
            _mm512_sqrt_pd(val[ 1]));
    }

    inline
    void load(const double *data)
    {
        val[ 0] = _mm512_loadu_pd(data + 0);
        val[ 1] = _mm512_loadu_pd(data + 8);
    }

    inline
    void load_aligned(const double *data)
    {
        SHORTVEC_ASSERT_ALIGNED(data, 64);
        val[ 0] = _mm512_load_pd(data + 0);
        val[ 1] = _mm512_load_pd(data + 8);
    }

    inline
    void store(double *data) const
    {
        _mm512_storeu_pd(data + 0, val[ 0]);
        _mm512_storeu_pd(data + 8, val[ 1]);
    }

    inline
    void store_aligned(double *data) const
    {
        SHORTVEC_ASSERT_ALIGNED(data, 64);
        _mm512_store_pd(data + 0, val[ 0]);
        _mm512_store_pd(data + 8, val[ 1]);
    }

    inline
    void store_nt(double *data) const
    {
        SHORTVEC_ASSERT_ALIGNED(data, 64);
        _mm512_stream_pd(data + 0, val[ 0]);
        _mm512_stream_pd(data + 8, val[ 1]);
    }

    inline
    void gather(const double *ptr, const int *offsets)
    {
        __m256i indices;
        indices = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(offsets));
        val[ 0]    = _mm512_i32gather_pd(indices, ptr, 8);
        indices = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(offsets + 8));
        val[ 1]    = _mm512_i32gather_pd(indices, ptr, 8);
    }

    inline
    void scatter(double *ptr, const int *offsets) const
    {
        __m256i indices;
        indices = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(offsets));
        _mm512_i32scatter_pd(ptr, indices, val[ 0], 8);
        indices = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(offsets + 8));
        _mm512_i32scatter_pd(ptr, indices, val[ 1], 8);
    }

    inline
    void blend(const mask_type& mask, const short_vec<double, 16>& other)
    {
        val[ 0] = _mm512_mask_blend_pd((mask >>  0)      , val[ 0], other.val[ 0]);
        val[ 1] = _mm512_mask_blend_pd((mask >>  8) & 255, val[ 1], other.val[ 1]);
    }

private:
    __m512d val[2];
};

inline
void operator<<(double *data, const short_vec<double, 16>& vec)
{
    vec.store(data);
}

#ifdef __ICC
#pragma warning pop
#endif

inline
short_vec<double, 16> sqrt(const short_vec<double, 16>& vec)
{
    return vec.sqrt();
}

template<typename _CharT, typename _Traits>
std::basic_ostream<_CharT, _Traits>&
operator<<(std::basic_ostream<_CharT, _Traits>& __os,
           const short_vec<double, 16>& vec)
{
    const double *data1 = reinterpret_cast<const double *>(&vec.val[ 0]);
    const double *data2 = reinterpret_cast<const double *>(&vec.val[ 1]);

    __os << "["  << data1[0] << ", " << data1[1] << ", " << data1[2] << ", " << data1[3]
         << ", " << data1[4] << ", " << data1[5] << ", " << data1[6] << ", " << data1[7]
         << ", " << data2[0] << ", " << data2[1] << ", " << data2[2] << ", " << data2[3]
         << ", " << data2[4] << ", " << data2[5] << ", " << data2[6] << ", " << data2[7]
         << "]";
    return __os;
}

}

#endif

#endif
