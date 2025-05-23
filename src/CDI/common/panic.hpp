#include <print>
#include <source_location>
#include <stdexcept>
#include <type_traits>

/** \brief Structure that holds the format string and the automatic source location.
 * Implementation thanks to this article: https://buildingblock.ai/panic
 */
template<typename... Args>
class PanicFormat
{
public:
    template<typename T>
    consteval PanicFormat(const T& fmt, std::source_location src = std::source_location::current()) : m_fmt{fmt}, m_src{src} {}

    std::format_string<Args...> m_fmt;
    std::source_location m_src;
};

/** \brief Prints the given std::format_string message then throws std::runtime_error.
 * \tparam Args The arguments of the format_string
 * \param fmt A std::format_string (automatically converts to PanicFormat with the file location already known).
 * \param args The std::format_string arguments.
 * \throw std::runtime_error to terminate the program.
 */
template<typename... Args>
[[noreturn]]
constexpr void panic(PanicFormat<std::type_identity_t<Args>...> fmt, Args&&... args)
{
    std::print(stderr, "{}:{} panic: ", fmt.m_src.function_name(), fmt.m_src.line());
    std::println(stderr, fmt.m_fmt, args...);
    throw std::runtime_error("panic");
}
