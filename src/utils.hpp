#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <sstream>
#include <string>

/** \brief Convert the Packed Binary Coded Decimal number to a byte output.
 *
 * \param data The PBCD to convert.
 * \return The converted value of data.
 */
inline uint8_t convertPBCD(const uint8_t data)
{
    return (data >> 4) * 10 + (data & 0x0F);
}

/** \brief Sign-extend an 8-bits number to 32-bits.
 *
 * \param data The 8-bits number to sign-extend to 32-bits.
 * \return The sign-extended 8-bits number to 32-bits.
 */
inline int32_t signExtend8(const int8_t data)
{
    return data;
}

/** \brief Sign-extend a 8-bits number to 16-bits.
 *
 * \param data The 8-bits number to sign-extend to 16-bits.
 * \return The sign-extended 8-bits number to 16-bits.
 */
inline int16_t signExtend816(const int8_t data)
{
    return data;
}

/** \brief Sign-extend a 16-bits number to 32-bits.
 *
 * \param data The 16-bits number to sign-extend to 32-bits.
 * \return The sign-extended 16-bits number to 32-bits.
 */
inline int32_t signExtend16(const int16_t data)
{
    return data;
}

/** \brief Checks if a number is even.
 *
 * \param number The number to check.
 * \return true if the number is even, false if it is odd.
 */
inline bool isEven(const int number)
{
    if(number & 1)
        return false;
    else
        return true;
}

/** \brief Converts a number to a hexadecimal string.
 *
 * \param number The number to convert.
 * \return The converted number as a string.
 */
inline std::string toHex(const uint32_t number)
{
    std::stringstream ss;
    ss << std::hex << number;
    return ss.str();
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

/** \brief Truncate the input if is greater or lower than the int16_t range.
 *
 * \param data The input to truncate.
 * \return the input if if fits in the range, INT16_MIN if input is lower, INT16_MAX if input is greater.
 */
inline int16_t lim16(const int32_t data)
{
    if(data > INT16_MAX)
        return INT16_MAX;

    if(data < INT16_MIN)
        return INT16_MIN;

    return data;
}

/** \brief Checks if an array is contained in another.
 *
 * \param container The array to be scanned.
 * \param containerSize The size of the array to be scanned.
 * \param contained The array containing the sequence of bytes to match.
 * \param containedSize The size of the array containing the sequence of bytes to match.
 *
 * \return A pointer to the first occurrence in container of the entire sequence of bytes specified in contained, or a null pointer if the sequence is not present in container.
 */
inline const void* subarrayOfArray(const void* container, size_t containerSize, const void* contained, size_t containedSize)
{
    const uint8_t* ner = (const uint8_t*)container;
    const uint8_t* ned = (const uint8_t*)contained;
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
    return NULL;
}

#ifdef DEBUG
#define OPEN_LOG(stream, name)  stream.open(name);
#define LOG(content) content;
#else
#define OPEN_LOG(stream, name)
#define LOG(content)
#endif // DEBUG

#endif // UTILS_HPP
