#ifndef VFPM_HPP
#define VFPM_HPP

#include <cstdint>
#include <type_traits>

#ifdef USE_CPP20_FEATUERS
#define USE_THREE_WAY_COMPARISON
#endif

#ifdef ADD_FP_IOSTREAM
#include <iostream>
#endif

namespace emt
{
    template <uint32_t Precision, typename BaseType = int32_t>
    class fixed_point;

    template <typename T>
    struct is_fixed_point_type
    {
        constexpr static bool value = false;
        constexpr operator bool() { return value; }
    };

    template <uint32_t P, typename U>
    struct is_fixed_point_type<fixed_point<P,U>>
    {
        constexpr static bool value = true;
        constexpr operator bool() { return value; }
    };

    template <typename T>
    constexpr T mask_bits(size_t how_many)
    {
        T rval = ~0;
        rval = rval >> (how_many - (sizeof(T) * 8));
        rval = rval << (how_many - (sizeof(T) * 8));
        return rval;
    }

    template <typename T>
    constexpr T mask_high_bits(size_t how_many)
    {
        T rval = ~0;
        rval = rval >> ((sizeof(T) * 8) - how_many);
        rval = rval << ((sizeof(T) * 8) - how_many);
        return rval;
    }

}

namespace emt
{
    template <uint32_t Precision, typename BaseType>
    class fixed_point
    {
        public:
            constexpr static uint32_t integer_bits = sizeof(BaseType) - Precision;
            constexpr static uint32_t fractional_bits = Precision;
            using underlying_type = BaseType;

            /*
             *  Construction / Destruction
             */
            constexpr fixed_point() noexcept = default;
            constexpr fixed_point(const fixed_point&) noexcept = default;
            constexpr fixed_point(fixed_point&&) noexcept = default;

            // for arithmetic types
//            template <typename T, typename std::enable_if_t<std::is_arithmetic<T>::value>>
            template <typename T>
            constexpr operator T() const noexcept;

            template <uint32_t P, typename U>
            constexpr operator fixed_point<P,U>() const noexcept;
            //std::strong_ordering operator<=>(const fixed_point& rhs) const = default;

            /*
             * Arithmetic operations
             */
            template <typename T> constexpr fixed_point& operator +=(const T& rhs) noexcept;
            template <typename T> constexpr fixed_point operator +(const T& rhs) noexcept;
            template <typename T> constexpr fixed_point& operator -=(const T& rhs) noexcept;
            template <typename T> constexpr fixed_point operator -(const T& rhs) noexcept;

            template <typename T> constexpr fixed_point& operator *=(const T& rhs) noexcept;
            template <typename T> constexpr fixed_point operator *(const T& rhs) noexcept;

        private:
            friend struct std::hash<fixed_point>;
            
            template <uint32_t P, typename U>
            friend class fixed_point;

            #ifdef ADD_FP_IOSTREAM
            friend std::ostream& operator<<(std::ostream& out, const fixed_point& val)
            {
                constexpr uint32_t divider = (1 << Precision);
                constexpr uint32_t mask = divider - 1;

                out << (int)val << " + " << (mask & val.value) << "/" << divider << "(" << (double)val << ")";
                return out;
            }
            #endif
            BaseType value = 0;
    };
    /*
            // this is a lot prettier with ArithmeticType concept
            template <typename T>
            explicit constexpr operator T()
            {
                static_assert(std::is_arithmetic<T>::value);
                if constexpr(std::is_interal<T>::value)
                    return (T)(value >> Precision);
                else if constexpr(std::is_floating_point<T>::value)
                    return (T)((T)value) / (1 << Precision);
            }

            // I wish there were IntegralType and FloatingPointType concepts for this...
            template <uint32_t P, template T>
            operator fixed_point<P>()
            {
                if constexpr (P > Precision)
                    return (value >> (P - Precision));
                else
                    return (value << (Precision - P));
            }
    */
    using fixed = fixed_point<16, int>;

    /*
     *  Conversions
     */
    template <uint32_t Precision, typename Underlying_Type>
    template <typename T>//, typename std::enable_if_t<std::is_arithmetic<T>::value>>
    constexpr fixed_point<Precision,Underlying_Type>::operator T() const noexcept
    {
        if constexpr(std::is_integral<T>::value)
            return (T)(value >> Precision);
        else if constexpr(std::is_floating_point<T>::value)
            return (T)((T)value) / (1 << Precision);
        else
            static_assert(std::is_same<T,T>::value, "Unknown arithmetic type for fixed point conversion");
    }

    template <uint32_t Precision, typename Underlying_Type>
    template <uint32_t P, typename U>
    constexpr fixed_point<Precision, Underlying_Type>::operator fixed_point<P,U>() const noexcept
    {
        fixed_point<P,U> rval;
        if constexpr (P > Precision)
            rval.value = (value << (P - Precision));
        else
            rval.value = (value >> (Precision - P));
        return rval;
    }

    /*
     *  Addition / substraction
     */
    template <uint32_t P, typename U> template <typename T>
    constexpr fixed_point<P,U>& fixed_point<P,U>::operator+=(const T& rhs) noexcept
    {
        if constexpr (std::is_integral<T>::value)
            value += (rhs << P);
        else if constexpr (std::is_floating_point<T>::value)
            value += rhs * (1 << P);
        else if constexpr (std::is_same<fixed_point<P,U>, T>::value)
            value += rhs.value;
        else
            *this += static_cast<fixed_point<P,U>>(rhs);
        return *this;
    }
    
    template <uint32_t P, typename U> template <typename T>
    constexpr fixed_point<P,U>& fixed_point<P,U>::operator-=(const T& rhs) noexcept
    {
        if constexpr (std::is_integral<T>::value)
            value -= (rhs << P);
        else if constexpr (std::is_floating_point<T>::value)
            value -= rhs * (1 << P);
        else if constexpr (std::is_same<fixed_point<P,U>, T>::value)
            value -= rhs.value;
        else
            *this -= static_cast<fixed_point<P,U>>(rhs);
        return *this;
    }

    template <uint32_t P, typename U> template <typename T>
    constexpr fixed_point<P,U> fixed_point<P,U>::operator+(const T& rhs) noexcept
    {
        return (fixed_point<P,U>(*this)) += rhs;
    }

    template <uint32_t P, typename U> template <typename T>
    constexpr fixed_point<P,U> fixed_point<P,U>::operator-(const T& rhs) noexcept
    {
        return (fixed_point<P,U>(*this)) -= rhs;
    }

    /*
     *  Multiplication / division
     */
    template <uint32_t P, typename U> template <typename T>
    constexpr fixed_point<P,U>& fixed_point<P,U>::operator*=(const T& rhs) noexcept
    {
        if constexpr (std::is_integral<T>::value)
            value *= (rhs << P);
        else if constexpr (std::is_floating_point<T>::value)
            value *= rhs * (1 << P);
        else if constexpr (is_fixed_point_type<T>::value)
        {
            const U rv = static_cast<fixed_point<P,U>>(rhs).value;

            const U lhs_int = value >> fractional_bits;
            const U lhs_frac = (value & ~mask_bits<U>(P));

            const U rhs_int = rv >> fractional_bits;
            const U rhs_frac = (rv & ~mask_bits<U>(fractional_bits));

            const U x1 = lhs_int * rhs_int;
            const U x2 = lhs_int * rhs_frac;
            const U x3 = lhs_frac * rhs_int;
            const U x4 = lhs_frac * rhs_frac;

            value = (x1 << P) + (x2 + x3) + (x4 >> P);
        }
        else
            *this *= static_cast<fixed_point<P,U>>(rhs);
        return *this;
    }
}

// set up stuff that expects us to
namespace std
{
    // Add ourselves to is_arithmetic, because we are.
    template <uint32_t Precision, typename BaseType>
    struct is_arithmetic<emt::fixed_point<Precision, BaseType>>
    {
        constexpr static bool value = true;
        constexpr operator bool() { return value; }
    };

    // Add hash specialisation
    template <uint32_t P, typename T>
    struct hash<emt::fixed_point<P,T>>
    {
        size_t operator()(const emt::fixed_point<P,T>& __v) const noexcept
        {
            return __v.value;
        }
    };
}


#endif
