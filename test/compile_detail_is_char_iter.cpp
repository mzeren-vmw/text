#include <boost/text/algorithm.hpp>
#include <boost/text/text_view.hpp>
#include <boost/text/text.hpp>

#include <array>
#include <list>
#include <string>
#include <vector>

using namespace boost;

static_assert(text::detail::is_char_iter<char *>{}, "");
static_assert(text::detail::is_char_iter<char const *>{}, "");

static_assert(text::detail::is_char_iter<text::text_view::iterator>{}, "");
static_assert(text::detail::is_char_iter<text::text_view::const_iterator>{}, "");
static_assert(text::detail::is_char_iter<text::text_view::reverse_iterator>{}, "");
static_assert(text::detail::is_char_iter<text::text_view::const_reverse_iterator>{}, "");

static_assert(text::detail::is_char_iter<text::text::iterator>{}, "");
static_assert(text::detail::is_char_iter<text::text::const_iterator>{}, "");
static_assert(text::detail::is_char_iter<text::text::reverse_iterator>{}, "");
static_assert(text::detail::is_char_iter<text::text::const_reverse_iterator>{}, "");

static_assert(text::detail::is_char_iter<std::string::iterator>{}, "");
static_assert(text::detail::is_char_iter<std::string::const_iterator>{}, "");
static_assert(text::detail::is_char_iter<std::string::reverse_iterator>{}, "");
static_assert(text::detail::is_char_iter<std::string::const_reverse_iterator>{}, "");

static_assert(text::detail::is_char_iter<std::vector<char>::iterator>{}, "");
static_assert(text::detail::is_char_iter<std::vector<char>::const_iterator>{}, "");
static_assert(text::detail::is_char_iter<std::vector<char>::reverse_iterator>{}, "");
static_assert(text::detail::is_char_iter<std::vector<char>::const_reverse_iterator>{}, "");

static_assert(text::detail::is_char_iter<std::array<char, 5>::iterator>{}, "");
static_assert(text::detail::is_char_iter<std::array<char, 5>::const_iterator>{}, "");
static_assert(text::detail::is_char_iter<std::array<char, 5>::reverse_iterator>{}, "");
static_assert(text::detail::is_char_iter<std::array<char, 5>::const_reverse_iterator>{}, "");

static_assert(text::detail::is_char_iter<std::list<char>::iterator>{}, "");
static_assert(text::detail::is_char_iter<std::list<char>::const_iterator>{}, "");
static_assert(text::detail::is_char_iter<std::list<char>::reverse_iterator>{}, "");
static_assert(text::detail::is_char_iter<std::list<char>::const_reverse_iterator>{}, "");

static_assert(!text::detail::is_char_iter<char>{}, "");
static_assert(!text::detail::is_char_iter<int>{}, "");
static_assert(!text::detail::is_char_iter<wchar_t *>{}, "");
static_assert(!text::detail::is_char_iter<std::vector<int>::iterator>{}, "");
static_assert(!text::detail::is_char_iter<wchar_t[5]>{}, "");
static_assert(!text::detail::is_char_iter<int[5]>{}, "");
