#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <sstream>
#include <string>

/** \brief Convert a Packed Binary Coded Decimal number to byte.
 *
 * \param data The PBCD to convert.
 * \return The converted PBCD to byte.
 */
inline uint8_t PBCDToByte(const uint8_t data)
{
    return (data >> 4) * 10 + (data & 0x0F);
}

/** \brief Convert a byte to Packed Binary Coded Decimal.
 *
 * \param data The byte to convert.
 * \return The converted byte to PBCD.
 *
 * Because PBCD are stored on one byte, if the input is greater than 99,
 * the conversion is modulo 100. e.g. a byte value of 103 or 203 will become 3 in PBCD.
 */
inline uint8_t byteToPBCD(uint8_t data)
{
    data %= 100;
    return ((data / 10) << 4) | (data % 10);
}

/** \brief Sign-extend a number.
 *
 * \param data The number to sign-extend (which type is the first template parameter T).
 * \return The sign-extended number (which type is the second template parameter R).
 */
template<typename T, typename R>
inline R signExtend(const T data)
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
    return !(number & 1);
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
 * \return A pointer to the first occurrence in container of the entire sequence of bytes specified in contained, or nullptr if the sequence is not present in container.
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
    return nullptr;
}

#ifdef DEBUG
#define OPEN_LOG(stream, name)  stream = fopen(name, "w");
#define LOG(content) content;
#define CLOSE_LOG(stream) fclose(stream);
#else
#define OPEN_LOG(stream, name)
#define LOG(content)
#define CLOSE_LOG(stream)
#endif // DEBUG

#endif // UTILS_HPP
