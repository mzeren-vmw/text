#ifndef BOOST_TEXT_TEXT_VIEW_HPP
#define BOOST_TEXT_TEXT_VIEW_HPP

#include <boost/text/detail/utility.hpp>
#include <boost/text/detail/iterator.hpp>

#include <ostream>

#include <cassert>


namespace boost { namespace text {

    struct text;

    struct text_view
    {
        using iterator = char const *;
        using const_iterator = char const *;
        using reverse_iterator = detail::const_reverse_char_iterator;
        using const_reverse_iterator = detail::const_reverse_char_iterator;

        constexpr text_view () noexcept :
            data_ (nullptr),
            size_ (0)
        {}

        constexpr text_view (char const * c_str) noexcept :
            data_ (c_str),
            size_ (detail::strlen(c_str))
        { assert(size_ <= INT_MAX); }

        constexpr text_view (char const * c_str, int len) noexcept :
            data_ (c_str),
            size_ (len)
        { assert(0 <= len); }

        constexpr text_view (text const & t) noexcept;

        constexpr text_view (text_view const & rhs) noexcept :
            data_ (rhs.data_),
            size_ (rhs.size_)
        {}
        constexpr text_view & operator= (text_view const & rhs) noexcept
        {
            data_ = rhs.data_;
            size_ = rhs.size_;
            return *this;
        }

        constexpr const_iterator begin () const noexcept { return data_; }
        constexpr const_iterator end () const noexcept { return data_ + size_; }

        constexpr const_reverse_iterator rbegin () const noexcept { return reverse_iterator(end()); }
        constexpr const_reverse_iterator rend () const noexcept { return reverse_iterator(begin()); }

        constexpr bool empty () const noexcept
        { return size_ == 0; }

        constexpr int size () const noexcept
        { return size_; }

        constexpr char operator[] (int i) const noexcept
        {
            assert(i < size_);
            return data_[i];
        }

        constexpr int max_size () const noexcept
        { return INT_MAX; }

        // TODO: operator<=> () const
        constexpr int compare (text_view rhs) const noexcept
        { return detail::compare_impl(begin(), end(), rhs.begin(), rhs.end()); }

        constexpr bool operator== (text_view rhs) const noexcept
        { return compare(rhs) == 0; }

        constexpr bool operator!= (text_view rhs) const noexcept
        { return compare(rhs) != 0; }

        constexpr bool operator< (text_view rhs) const noexcept
        { return compare(rhs) < 0; }

        constexpr bool operator<= (text_view rhs) const noexcept
        { return compare(rhs) <= 0; }

        constexpr bool operator> (text_view rhs) const noexcept
        { return compare(rhs) > 0; }

        constexpr bool operator>= (text_view rhs) const noexcept
        { return compare(rhs) >= 0; }

        constexpr void swap (text_view & rhs) noexcept
        {
            {
                char const * tmp = data_;
                data_ = rhs.data_;
                rhs.data_ = tmp;
            }
            {
                int tmp = size_;
                size_ = rhs.size_;
                rhs.size_ = tmp;
            }
        }

        friend constexpr iterator begin (text_view v) noexcept
        { return v.begin(); }
        friend constexpr iterator end (text_view v) noexcept
        { return v.end(); }

        friend constexpr reverse_iterator rbegin (text_view v) noexcept
        { return v.rbegin(); }
        friend constexpr reverse_iterator rend (text_view v) noexcept
        { return v.rend(); }

        friend std::ostream & operator<< (std::ostream & os, text_view view)
        { return os.write(view.begin(), view.size()); }

    private:
        char const * data_;
        int size_;
    };

    namespace literals {

        inline constexpr text_view operator"" _tv (char const * str, std::size_t len) noexcept
        {
            assert(len < INT_MAX);
            return text_view(str, len);
        }

        // TODO: constexpr text_view operator"" _tv (std::char16_t const * str, std::size_t len) noexcept
        // TODO: constexpr text_view operator"" _tv (std::char32_t const * str, std::size_t len) noexcept
        // TODO: constexpr text_view operator"" _tv (std::wchar_t const * str, std::size_t len) noexcept

    }

    struct repeated_text_view
    {
        using iterator = detail::const_repeated_chars_iterator;
        using const_iterator = detail::const_repeated_chars_iterator;

        constexpr repeated_text_view () noexcept : count_ (0) {}

        constexpr repeated_text_view (text_view view, std::ptrdiff_t count) noexcept :
            view_ (view),
            count_ (count)
        { assert(view.empty() || view.end()[-1] != '\0'); }

        constexpr text_view view () const noexcept
        { return view_; }
        constexpr std::ptrdiff_t count () const noexcept
        { return count_; }

        constexpr std::ptrdiff_t size () const noexcept
        { return count_ * view_.size(); }

        constexpr const_iterator begin () const noexcept
        { return const_iterator(view_.begin(), view_.end(), count_); }
        constexpr const_iterator end () const noexcept
        { return const_iterator(view_.begin(), view_.end()); }

        friend constexpr iterator begin (repeated_text_view v) noexcept
        { return v.begin(); }
        friend constexpr iterator end (repeated_text_view v) noexcept
        { return v.end(); }

        friend std::ostream & operator<< (std::ostream & os, repeated_text_view rv)
        {
            for (std::ptrdiff_t i = 0; i < rv.count(); ++i) {
                os.write(rv.view().begin(), rv.view().size());
            }
            return os;
        }

    private:
        text_view view_;
        std::ptrdiff_t count_;
    };

    constexpr repeated_text_view repeat (text_view view, std::ptrdiff_t count)
    { return repeated_text_view(view, count); }

} }

#include <boost/text/text.hpp>

namespace boost { namespace text {

    inline constexpr text_view::text_view (text const & t) noexcept :
        data_ (t.begin()),
        size_ (t.size())
    {}

} }

/* Rationale

   1: use of signed types >= sizeof(int) for sizes

   2: including null terminator in strings

   3: Removal of data(), front(), back(), assign()

*/

// Not rationale:

// TODO: Do text_view == using address + size to short-circuit string
// compares.

// TODO: throw when constructing a text_view or text from non UTF-8.

#endif
