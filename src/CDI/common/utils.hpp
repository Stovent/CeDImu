/** \file utils.hpp
 * \brief Utility functions.
 */

#ifndef CDI_COMMON_UTILS_HPP
#define CDI_COMMON_UTILS_HPP

#include <algorithm>
#include <bit>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <format>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

/** \brief Shortcut name for `static_cast<R>(data)`.
 * \tparam R the return type.
 * \tparam T The input type.
 *
 * Template deduction should allow directly doing `as<R>()` without specifing T.
 */
template<typename R, typename T>
constexpr R as(T&& data)
{
    return static_cast<R>(std::forward<T>(data));
}

/** \brief Sign-extends a number from type T to type R.
 *
 * This function makes sure the source type is signed.
 */
template<std::signed_integral T, std::integral R>
constexpr auto signExtend = as<R, T>;

/** \brief Zero-extends a number from type T to type R.
 *
 * This function makes sure the source type is unsigned.
 */
template<std::unsigned_integral T, std::integral R>
constexpr auto zeroExtend = as<R, T>;

/** \brief Tests if a bit is set.
 * \tparam BITNUM The bit number to check.
 * \tparam T The type of the data.
 * \return true if the bit is set, false if it is clear.
 */
template<size_t BITNUM, std::integral T>
constexpr bool bit(const T data) noexcept
{
    static_assert(BITNUM < (sizeof(T) * 8));

    return (data & (1 << BITNUM)) != 0;
}

/** \brief Extracts the given bit range inclusive.
 * \tparam BITFIRST The first bit number to extract.
 * \tparam BITLAST The last bit number to extract (inclusive).
 * \tparam T The type of the data.
 * \return The extracted bits.
 */
template<size_t BITFIRST, size_t BITLAST, std::integral T>
constexpr T bits(const T data) noexcept
{
    static_assert(BITFIRST <= BITLAST);
    static_assert(BITLAST < (sizeof(T) * 8));

    constexpr T mask = (1 << (BITLAST + 1 - BITFIRST)) - 1;
    return data >> BITFIRST & mask;
}

/** \brief Convert a Packed Binary Coded Decimal number to byte.
 *
 * \param data The PBCD to convert.
 * \return The converted PBCD to byte.
 */
constexpr uint8_t PBCDToByte(const uint8_t data) noexcept
{
    return bits<4, 7>(data) * 10 + bits<0, 3>(data);
}

/** \brief Convert a byte to Packed Binary Coded Decimal.
 *
 * \param data The byte to convert.
 * \return The converted byte to PBCD.
 *
 * Because PBCD are stored on one byte, if the input is greater than 99,
 * the conversion is modulo 100. e.g. a byte value of 103 or 203 will become 3 in PBCD.
 */
constexpr uint8_t byteToPBCD(uint8_t data) noexcept
{
    data %= 100;
    return ((data / 10) << 4) | (data % 10);
}

/** \brief Checks if a number is even.
 *
 * \param number The number to check.
 * \return true if the number is even, false if it is odd.
 */
template<std::integral T>
constexpr bool isEven(const T number) noexcept
{
    return (number & 1) == 0;
}

/** \brief Converts a number to a hexadecimal string.
 * \tparam T The data type.
 * \param number The number to convert.
 * \return The converted number as a string.
 */
template<std::integral T>
static std::string toHex(const T number)
{
    return std::format("{:X}", number);
}

/** \brief Converts the binary representation of a number to a string.
 *
 * \param value The number to convert.
 * \param lengthInBits The length of the input number to convert.
 * \return The binary representation converted to a string.
 */
inline std::string toBinString(const uint32_t value, uint8_t lengthInBits)
{
    std::string tmp;
    uint32_t mask = 1 << (lengthInBits-1);
    while(lengthInBits)
    {
        if(value & mask)
            tmp += "1";
        else
            tmp += "0";
        mask >>= 1;
        lengthInBits--;
    }
    return tmp;
}

/** \brief Converts a string containing a sequence of bits into its number representation.
 *
 * \param s The sequence of bits as a string.
 * \return The sequence of bits as a number.
 */
inline uint32_t binStringToInt(const std::string& s)
{
    uint32_t ret = 0;
    uint32_t base = 1 << (s.length()-1);
    for(uint8_t i = 0; i < s.length(); i++)
        if(s[i] == '1')
            ret |= base >> i;
    return ret;
}

/** \brief Limits the input to the uint8_t range.
 *
 * \param d The input to limit.
 * \return the input if if fits in the range, 0 if input is lower, 255 if input is greater.
 */
template<std::integral T>
constexpr uint8_t limu8(const T d) noexcept
{
    return static_cast<uint8_t>(std::clamp<T>(d, 0, UINT8_MAX));
}

/** \brief Limits the input to the int16_t range.
 *
 * \param d The input to limit.
 * \return the input if if fits in the range, INT16_MIN if input is lower, INT16_MAX if input is greater.
 */
template<std::integral T>
constexpr int16_t lims16(const T d) noexcept
{
    return static_cast<int16_t>(std::clamp<T>(d, INT16_MIN, INT16_MAX));
}

/** \brief Checks if an array is contained in another.
 *
 * \param container The array to be scanned.
 * \param containerSize The size of the array to be scanned.
 * \param contained The array containing the sequence of bytes to match.
 * \param containedSize The size of the array containing the sequence of bytes to match.
 *
 * \return A pointer to the first occurrence in container of the entire sequence of bytes specified in contained, or nullptr if the sequence is not present in container.
 */
inline const void* subarrayOfArray(const void* container, size_t containerSize, const void* contained, size_t containedSize)
{
    const uint8_t* ner = as<const uint8_t*>(container);
    const uint8_t* ned = as<const uint8_t*>(contained);
    for(size_t j = 0; j < containerSize; j++)
    {
        if(ner[j] == ned[0])
        {
            size_t count = 0;
            for(size_t i = 0; i < containedSize && (j + i) < containerSize; i++)
            {
                if(ner[j+i] != ned[i])
                    break;

                count++;
            }
            if(count == containedSize)
                return &ner[j];
        }
    }
    return nullptr;
}

/** \brief Makes a uint16_t from the two given uint8_t. */
constexpr uint16_t makeU16(uint8_t bits0_7, uint8_t bits8_15) noexcept
{
    return static_cast<uint16_t>(bits8_15) << 8 | bits0_7;
}

/** \brief Makes a uint32_t from the four given uint8_t. */
constexpr uint32_t makeU32(uint8_t bits0_7, uint8_t bits8_15, uint8_t bits16_23, uint8_t bits24_31) noexcept
{
    return static_cast<uint32_t>(bits24_31) << 24 | static_cast<uint32_t>(bits16_23) << 16 |
           static_cast<uint32_t>(bits8_15) << 8 | bits0_7;
}

template<typename T, typename VALUE>
concept Indexable = requires(T a, size_t index)
{
    a[index];
    requires std::same_as<std::remove_cvref_t<decltype(a[index])>, VALUE>;
};

/** \brief Reads a big-endian uint16_t starting at the given index in the given array.
 * Overload specialized for trivial arrays.
 */
inline uint16_t getArray16(const uint8_t* array, size_t index) noexcept
{
    uint16_t x;
    std::memcpy(&x, array + index, sizeof x);
    return std::byteswap(x);
}

/** \brief Reads a big-endian uint16_t starting at the given index in the given array. */
template<Indexable<uint8_t> T>
constexpr uint16_t getArray16(T& array, size_t index) noexcept(noexcept(array[index]))
{
    return makeU16(array[index + 1], array[index]);
}

/** \brief Reads a big-endian uint32_t starting at the given index in the given array.
 * Overload specialized for trivial arrays.
 */
inline uint32_t getArray32(const uint8_t* array, size_t index) noexcept
{
    uint32_t x;
    std::memcpy(&x, array + index, sizeof x);
    return std::byteswap(x);
}

/** \brief Reads a big-endian uint32_t starting at the given index in the given array. */
template<Indexable<uint8_t> T>
constexpr uint32_t getArray32(T& array, size_t index) noexcept(noexcept(array[index]))
{
    return makeU32(array[index + 3], array[index + 2], array[index + 1], array[index]);
}

#endif // CDI_COMMON_UTILS_HPP
