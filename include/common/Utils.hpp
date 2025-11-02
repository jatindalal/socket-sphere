#include <random>
#include <string>

inline std::string random_string(size_t length)
{
    static constexpr std::string_view chars =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "01234567890";
    static thread_local std::mt19937 random_number_generator(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, chars.size() - 1);

    std::string s;
    s.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        s += chars[dist(random_number_generator)];
    }
    return s;
}