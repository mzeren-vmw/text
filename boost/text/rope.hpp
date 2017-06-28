#ifndef BOOST_TEXT_ROPE_HPP
#define BOOST_TEXT_ROPE_HPP

#include <boost/text/detail/rope.hpp>

#ifdef BOOST_TEXT_TESTING
#include <iostream>
#endif


namespace boost { namespace text {

    struct rope_view;
    struct rope;

    namespace detail {
        struct const_rope_iterator;
        struct const_reverse_rope_iterator;
    }

    /** A mutable sequence of char with copy-on-write semantics.  The sequence
        is assumed to be UTF-8 encoded, though it is possible to construct a
        sequence which is not. A rope is non-contiguous and is not
        null-terminated. */
    struct rope
    {
        using iterator = detail::const_rope_iterator;
        using const_iterator = detail::const_rope_iterator;
        using reverse_iterator = detail::const_reverse_rope_iterator;
        using const_reverse_iterator = detail::const_reverse_rope_iterator;

        using size_type = std::ptrdiff_t;

        /** Default ctor.

            \post size() == 0 && begin() == end() */
        rope () noexcept : ptr_ (nullptr) {}

        rope (rope const & rhs) = default;
        rope (rope && rhs) noexcept = default;

        /** Constructs a rope from a rope_view. */
        explicit rope (rope_view rv);

        /** Constructs a rope from a text. */
        explicit rope (text const & t) : ptr_ (detail::make_node(t)) {}

        /** Move-constructs a rope from a text. */
        explicit rope (text && t) : ptr_ (detail::make_node(std::move(t))) {}

        /** Constructs a rope from a text_view. */
        explicit rope (text_view tv) : ptr_ (nullptr)
        { insert(0, tv); }

        /** Constructs a rope from a repeated_text_view. */
        explicit rope (repeated_text_view rtv) : ptr_ (nullptr)
        { insert(0, rtv); }

#ifdef BOOST_TEXT_DOXYGEN

        /** Constructs a rope from a range of char.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8. */
        template <typename CharRange>
        explicit rope (CharRange const & r);

        /** Constructs a rope from a sequence of char.

            The sequence's UTF-8 encoding is not checked.  To check the
            encoding, use a converting iterator.

            This function only participates in overload resolution if Iter
            models the Char_iterator concept. */
        template <typename Iter>
        rope (Iter first, Iter last);

#else

        template <typename CharRange>
        explicit rope (
            CharRange const & r,
            detail::rope_rng_ret_t<int *, CharRange> = 0
        ) : ptr_ ()
        { insert(0, r); }

        template <typename Iter>
        rope (
            Iter first, Iter last,
            detail::char_iter_ret_t<void *, Iter> = 0
        ) : ptr_ ()
        { insert(0, first, last); }

#endif

        rope & operator= (rope const & rhs) = default;
        rope & operator= (rope && rhs) noexcept = default;

        /** Assignment from a text. */
        rope & operator= (text const & t)
        {
            rope temp(t);
            swap(temp);
            return *this;
        }

        /** Assignment from a rope_view. */
        rope & operator= (rope_view rv);

        /** Move-assignment from a text. */
        rope & operator= (text && t)
        {
            rope temp(std::move(t));
            swap(temp);
            return *this;
        }

#ifdef BOOST_TEXT_DOXYGEN

        /** Assignment from a range of char.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8. */
        template <typename CharRange>
        rope & operator= (CharRange const & r);

#else

        template <typename CharRange>
        auto operator= (CharRange const & r)
            -> detail::rope_rng_ret_t<rope &, CharRange>
        { return *this = text_view(&*r.begin(), r.end() - r.begin()); }

#endif

        /** Assignment from a text_view. */
        rope & operator= (text_view tv)
        {
            rope temp(tv);
            swap(temp);
            return *this;
        }

        /** Assignment from a repeated_text_view. */
        rope & operator= (repeated_text_view rtv)
        {
            rope temp(rtv);
            swap(temp);
            return *this;
        }

        const_iterator begin () const noexcept;
        const_iterator end () const noexcept;

        const_reverse_iterator rbegin () const noexcept;
        const_reverse_iterator rend () const noexcept;

        bool empty () const noexcept
        { return size() == 0; }

        size_type size () const noexcept
        { return detail::size(ptr_.get()); }

        /** Returns the i-th char of *this (not a reference).

            \pre 0 <= i && i < size() */
        char operator[] (size_type n) const noexcept
        {
            assert(ptr_);
            assert(n < size());
            detail::found_char found;
            find_char(ptr_, n, found);
            return found.c_;
        }

        /** Returns a substring of *this as a rope_view, taken from the range
            of chars at offsets [lo, hi).  If either of lo or hi is a negative
            value x, x is taken to be an offset from the end, and so x +
            size() is used instead.

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= size()
            \pre 0 <= hi && lhi <= size()
            \pre lo <= hi
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        rope_view operator() (int lo, int hi) const;

        /** Returns a substring of *this as a rope_view, taken from the first
            cut chars when cut => 0, or the last -cut chars when cut < 0.

            \pre 0 <= cut && cut <= size() || 0 <= -cut && -cut <= size()
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        rope_view operator() (int cut) const;

        /** Returns the maximum size a text can have. */
        size_type max_size () const noexcept
        { return PTRDIFF_MAX; }

        // TODO: Tests!
        /** Returns a substring of *this as a new rope, taken from the range
            of chars at offsets [lo, hi).  If either of lo or hi is a negative
            value x, x is taken to be an offset from the end, and so x +
            size() is used instead.

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= size()
            \pre 0 <= hi && lhi <= size()
            \pre lo <= hi
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        rope substr (size_type lo, size_type hi) const;

        /** Returns a substring of *this, taken from the first cut chars when
            cut => 0, or the last -cut chars when cut < 0.

            \pre 0 <= cut && cut <= size() || 0 <= -cut && -cut <= size()
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        rope substr (size_type cut) const;

        /** Visits each segment s of *this and calls f(s).  Each segment is a
            text const &, text_view, or repeated_text_view.  Depending of the
            operation performed on each segment, this may be more efficient
            than iterating over [begin(), end()).

            \pre Fn is an Invocable accepting a single argument of any of the
            types listed above. */
        template <typename Fn>
        void foreach_segment (Fn && f) const
        {
            detail::foreach_leaf(ptr_, [&](detail::leaf_node_t const * leaf) {
                switch (leaf->which_) {
                case detail::node_t::which::t:
                    f(leaf->as_text());
                    break;
                case detail::node_t::which::tv:
                    f(leaf->as_text_view());
                    break;
                case detail::node_t::which::rtv:
                    f(leaf->as_repeated_text_view());
                    break;
                case detail::node_t::which::ref:
                    f(leaf->as_reference().ref_);
                    break;
                default: assert(!"unhandled rope node case"); break;
                }
                return true;
            });
        }

        /** Lexicographical compare.  Returns a value < 0 when *this is
            lexicographically less than rhs, 0 if *this == rhs, and a value >
            0 if *this is lexicographically greater than rhs. */
        int compare (rope rhs) const noexcept;

        bool operator== (rope rhs) const noexcept
        { return compare(rhs) == 0; }

        bool operator!= (rope rhs) const noexcept
        { return compare(rhs) != 0; }

        bool operator< (rope rhs) const noexcept
        { return compare(rhs) < 0; }

        bool operator<= (rope rhs) const noexcept
        { return compare(rhs) <= 0; }

        bool operator> (rope rhs) const noexcept
        { return compare(rhs) > 0; }

        bool operator>= (rope rhs) const noexcept
        { return compare(rhs) >= 0; }

        void clear ()
        { ptr_ = detail::node_ptr(); }

        // TODO: Document the creation of individual segments based on the
        // ctor, assignment operator, insert, or replace overload.

        // TODO: Test all these overloads for insert(), replace(), and
        // erase(), including for texts.  Include verification of encoding
        // checks being done (or not).
        /** Inserts the sequence of char from rv into *this starting at offset
            at.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding. */
        rope & insert (size_type at, rope_view rv);

        /** Inserts the sequence of char from t into *this starting at offset
            at, by moving the contents of t.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding. */
        rope & insert (size_type at, text && t)
        { return insert_impl(at, std::move(t), would_not_allocate); }

        /** Inserts the sequence of char from tv into *this starting at offset
            at.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding. */
        rope & insert (size_type at, text_view tv)
        {
            bool const tv_null_terminated = !tv.empty() && tv.end()[-1] == '\0';
            if (tv_null_terminated)
                tv = tv(0, -1);
            return insert_impl(at, tv, would_not_allocate);
        }

        /** Inserts the sequence of char from rtv into *this starting at offset
            at.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding. */
        rope & insert (size_type at, repeated_text_view rtv)
        {
            bool const rtv_null_terminated =
                !rtv.view().empty() && rtv.view().end()[-1] == '\0';
            if (rtv_null_terminated)
                rtv = repeat(rtv.view()(0, -1), rtv.count());
            return insert_impl(at, rtv, would_not_allocate);
        }

#ifdef BOOST_TEXT_DOXYGEN

        /** Inserts the char range r into *this starting at offset at.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding, or if the ends of the range are not valid
            UTF-8. */
         template <typename CharRange>
        rope & insert (size_type at, CharRange const & r);

        /** Inserts the char sequence [first, last) into *this starting at
            offset at.

            This function only participates in overload resolution if Iter
            models the Char_iterator concept.

            The inserted sequence's UTF-8 encoding is not checked.  To check
            the encoding, use a converting iterator.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding. */
        template <typename Iter>
        rope & insert (size_type at, Iter first, Iter last);

        /** Inserts the char sequence [first, last) into *this starting at
            position at.

            This function only participates in overload resolution if Iter
            models the Char_iterator concept.

            No check is made to determine if insertion at position at would
            break UTF-8 encoding, and the inserted sequence's UTF-8 encoding
            is not checked.  To check the inserted sequence's encoding, use a
            converting iterator. */
        template <typename Iter>
        rope & insert (const_iterator at, Iter first, Iter last);

#else

        template <typename CharRange>
        auto insert (size_type at, CharRange const & r)
            -> detail::rope_rng_ret_t<rope &, CharRange>
        { return insert(at, text_view(&*r.begin(), r.end() - r.begin())); }

        template <typename Iter>
        auto insert (size_type at, Iter first, Iter last)
            -> detail::char_iter_ret_t<rope &, Iter>;

        template <typename Iter>
        auto insert (const_iterator at, Iter first, Iter last)
            -> detail::char_iter_ret_t<rope &, Iter>;

#endif

        /** Erases the portion of *this delimited by rv.

            \pre tv.begin() <= rv.begin() && rvend() <= end() */
        rope & erase (rope_view rv);

        /** Erases the portion of *this delimited by [first, last).

            No check is made to determine whether erasing [first, last) breaks
            UTF-8 encoding.

            \pre first <= last */
        rope & erase (const_iterator first, const_iterator last);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from rv.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        rope & replace (rope_view old_substr, rope_view rv);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from t by moving the contents of t.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        rope & replace (rope_view old_substr, text && t);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from tv.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        rope & replace (rope_view old_substr, text_view tv);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from rtv.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        rope & replace (rope_view old_substr, repeated_text_view rtv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Replaces the portion of *this delimited by old_substr with the
            char range r.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8.
            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template <typename CharRange>
        rope & replace (rope_view old_substr, CharRange const & r);

        /** Replaces the portion of *this delimited by old_substr with the
            char sequence [first, last).

            This function only participates in overload resolution if Iter
            models the Char_iterator concept.

            The inserted sequence's UTF-8 encoding is not checked.  To check
            the encoding, use a converting iterator.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template <typename Iter>
        rope & replace (rope_view old_substr, Iter first, Iter last);

        /** Replaces the portion of *this delimited by [old_first, old_last)
            with the char sequence [new_first, new_last).

            This function only participates in overload resolution if Iter
            models the Char_iterator concept.

            No check is made to determine if removing [old_first, old_last)
            would break UTF-8 encoding, and the inserted sequence's UTF-8
            encoding is not checked.  To check the inserted sequence's
            encoding, use a converting iterator.

           \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template <typename Iter>
        rope & replace (const_iterator old_first, const_iterator old_last, Iter new_first, Iter new_last);

#else

        template <typename CharRange>
        auto replace (rope_view old_substr, CharRange const & r)
            -> detail::rope_rng_ret_t<rope &, CharRange>;

        template <typename Iter>
        auto replace (rope_view old_substr, Iter first, Iter last)
            -> detail::char_iter_ret_t<rope &, Iter>;

        template <typename Iter>
        auto replace (const_iterator old_first, const_iterator old_last, Iter new_first, Iter new_last)
            -> detail::char_iter_ret_t<rope &, Iter>;

#endif

        /** Swaps *this with rhs. */
        void swap (rope & rhs)
        { ptr_.swap(rhs.ptr_); }

        // TODO: Test all these overloads.
        /** Appends rv to *this. */
        rope & operator+= (rope_view rv);

        /** Appends r to *this, by moving its contents into *this. */
        rope & operator+= (rope && r)
        {
            detail::interior_node_t * new_root = nullptr;
            detail::node_ptr new_root_ptr(new_root = detail::new_interior_node());
            new_root->keys_.push_back(size());
            new_root->keys_.push_back(size() + r.size());
            new_root->children_.push_back(std::move(ptr_));
            new_root->children_.push_back(std::move(r.ptr_));
            ptr_ = std::move(new_root_ptr);
            return *this;
        }

        /** Appends t to *this, by moving its contents into *this. */
        rope & operator+= (text && t);

        /** Appends tv to *this. */
        rope & operator+= (text_view tv);

        /** Appends rtv to *this. */
        rope & operator+= (repeated_text_view rtv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Appends the char range r to *this.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8. */
        template <typename CharRange>
        rope & operator+= (CharRange const & r);

#else

        template <typename CharRange>
        auto operator+= (CharRange const & r)
            -> detail::rope_rng_ret_t<rope &, CharRange>;

#endif

        /** Stream inserter; performs formatted output. */
        friend std::ostream & operator<< (std::ostream & os, rope r)
        {
            if (os.good()) {
                detail::pad_width_before(os, r.size());
                r.foreach_segment(detail::segment_inserter{os});
                if (os.good())
                    detail::pad_width_after(os, r.size());
            }
            return os;
        }

#ifdef BOOST_TEXT_TESTING
        friend void dump_tree (rope const & r)
        {
            if (r.empty())
                std::cout << "[EMPTY]\n";
            else
                detail::dump_tree(r.ptr_);
        }
#endif

#ifndef BOOST_TEXT_DOXYGEN

    private:
        enum allocation_note_t { would_allocate, would_not_allocate };

        explicit rope (detail::node_ptr const & node) : ptr_ (node) {}

        bool self_reference (rope_view rv) const;

        struct text_insertion
        {
            explicit operator bool () const
            { return text_ != nullptr; }

            text * text_;
            std::ptrdiff_t offset_;
        };

        text_insertion mutable_insertion_leaf (size_type at, size_type size, allocation_note_t allocation_note)
        {
            if (!ptr_)
                return text_insertion{nullptr};

            detail::found_leaf found;
            find_leaf(ptr_, at, found);

            for (auto node : found.path_) {
                if (1 < node->refs_)
                    return text_insertion{nullptr};
            }

            if (found.leaf_->as_leaf()->which_ == detail::node_t::which::t) {
                text & t = const_cast<text &>(found.leaf_->as_leaf()->as_text());
                auto const inserted_size = t.size() + size;
                if (inserted_size <= t.capacity())
                    return text_insertion{&t, found.offset_};
                else if (allocation_note == would_allocate && inserted_size <= detail::text_insert_max)
                    return text_insertion{&t, found.offset_};
            }

            return text_insertion{nullptr};
        }

        template <typename T>
        rope & insert_impl (
            size_type at,
            T && t,
            allocation_note_t allocation_note,
            detail::encoding_note_t encoding_note = detail::check_encoding_breakage
        ) {
            if (t.empty())
                return *this;

            if (text_insertion insertion = mutable_insertion_leaf(at, t.size(), allocation_note)) {
                if (encoding_note == detail::encoding_breakage_ok)
                    insertion.text_->insert(insertion.text_->begin() + insertion.offset_, t.begin(), t.end());
                else
                    insertion.text_->insert(insertion.offset_, t);
            } else {
                ptr_ = detail::btree_insert(
                    ptr_,
                    at,
                    detail::make_node(std::forward<T &&>(t)),
                    encoding_note
                );
            }

            return *this;
        }

        detail::node_ptr ptr_;

        friend struct detail::const_rope_iterator;
        friend struct rope_view;

#endif

    };


    /** Forwards r when it is entirely UTF-8 encoded; throws otherwise.

        \throw std::invalid_argument when r is not UTF-8 encoded. */
    inline rope const & checked_encoding (rope const & r);

    /** Forwards r when it is entirely UTF-8 encoded; throws otherwise.

        \throw std::invalid_argument when r is not UTF-8 encoded. */
    inline rope && checked_encoding (rope && r);

} }

#include <boost/text/detail/rope_iterator.hpp>
#include <boost/text/rope_view.hpp>

namespace boost { namespace text {

#ifndef BOOST_TEXT_DOXYGEN

    inline rope::rope (rope_view rv) : ptr_ (nullptr)
    { insert(0, rv); }

    inline rope & rope::operator= (rope_view rv)
    {
        detail::node_ptr extra_ref;
        if (self_reference(rv))
            extra_ref = ptr_;

        rope temp(rv);
        swap(temp);
        return *this;
    }

    inline int rope::compare (rope rhs) const noexcept
    { return rope_view(*this).compare(rhs); }

    inline rope & rope::insert (size_type at, rope_view rv)
    {
        if (rv.empty())
            return *this;

        detail::node_ptr extra_ref;
        if (self_reference(rv))
            extra_ref = ptr_;

        bool const rv_null_terminated = !rv.empty() && rv.end()[-1] == '\0';
        if (rv_null_terminated)
            rv = rv(0, -1);

        if (rv.which_ == rope_view::which::tv)
            return insert(at, rv.ref_.tv_);

        if (rv.which_ == rope_view::which::rtv) {
            if (rv.ref_.rtv_.lo_ == 0 && rv.ref_.rtv_.hi_ == rv.ref_.rtv_.rtv_.size())
                return insert(at, rv.ref_.rtv_.rtv_);
            return insert(at, text(rv.begin(), rv.end()));
        }

        rope_view::rope_ref rope_ref = rv.ref_.r_;

        detail::found_leaf found_lo;
        find_leaf(rope_ref.r_->ptr_, rope_ref.lo_, found_lo);
        detail::leaf_node_t const * const leaf_lo = found_lo.leaf_->as_leaf();

        // If the entire rope_view lies within a single segment, slice off
        // the appropriate part of that segment.
        if (found_lo.offset_ + rv.size() <= detail::size(leaf_lo)) {
            ptr_ = detail::btree_insert(
                ptr_,
                at,
                slice_leaf(
                    *found_lo.leaf_,
                    found_lo.offset_,
                    found_lo.offset_ + rv.size(),
                    true,
                    detail::check_encoding_breakage
                )
            );
            return *this;
        }

        {
            detail::node_ptr node;
            if (found_lo.offset_ != 0) {
                node = slice_leaf(
                    *found_lo.leaf_,
                    found_lo.offset_,
                    detail::size(leaf_lo),
                    true,
                    detail::check_encoding_breakage
                );
            } else {
                node = detail::node_ptr(leaf_lo);
            }
            ptr_ = detail::btree_insert(ptr_, at, std::move(node));
        }
        at += detail::size(leaf_lo);

        detail::found_leaf found_hi;
        find_leaf(rope_ref.r_->ptr_, rope_ref.hi_, found_hi);
        detail::leaf_node_t const * const leaf_hi = found_hi.leaf_->as_leaf();

        detail::foreach_leaf(ptr_, [&](detail::leaf_node_t const * leaf) {
            if (leaf == leaf_lo)
                return true; // continue
            if (leaf == leaf_hi)
                return false; // break
            ptr_ = detail::btree_insert(ptr_, at, detail::node_ptr(leaf));
            at += detail::size(leaf);
            return true;
        });

        if (found_hi.offset_ != 0) {
            ptr_ = detail::btree_insert(
                ptr_,
                at,
                slice_leaf(
                    *found_hi.leaf_,
                    0,
                    found_hi.offset_,
                    true,
                    detail::check_encoding_breakage
                )
            );
        }

        return *this;
    }

    template <typename Iter>
    auto rope::insert (size_type at, Iter first, Iter last)
        -> detail::char_iter_ret_t<rope &, Iter>
    {
        assert(0 <= at && at <= size());

        if (first == last)
            return *this;

        if (!utf8::starts_encoded(begin() + at, end()))
            throw std::invalid_argument("Inserting at that character breaks UTF-8 encoding.");

        ptr_ = detail::btree_insert(ptr_, at, detail::make_node(text(first, last)));

        return *this;
    }

    template <typename Iter>
    auto rope::insert (const_iterator at, Iter first, Iter last)
        -> detail::char_iter_ret_t<rope &, Iter>
    {
        assert(begin() <= at && at <= end());

        if (first == last)
            return *this;

        ptr_ = detail::btree_insert(
            ptr_,
            at - begin(),
            detail::make_node(text(first, last)),
            detail::encoding_breakage_ok
        );

        return *this;
    }

    inline rope & rope::erase (rope_view rv)
    {
        assert(self_reference(rv));

        rope_view::rope_ref rope_ref = rv.ref_.r_;

        assert(0 <= rope_ref.lo_ && rope_ref.lo_ <= size());
        assert(0 <= rope_ref.hi_ && rope_ref.hi_ <= size());
        assert(rope_ref.lo_ <= rope_ref.hi_);

        if (rope_ref.lo_ == rope_ref.hi_)
            return *this;

        bool const rv_null_terminated = !rv.empty() && rv.end()[-1] == '\0';
        if (rv_null_terminated)
            rv = rv(0, -1);

        ptr_ = btree_erase(ptr_, rope_ref.lo_, rope_ref.hi_);

        return *this;
    }

    inline rope & rope::erase (const_iterator first, const_iterator last)
    {
        assert(first <= last);
        assert(begin() <= first && last <= end());

        if (first == last)
            return *this;

        auto const lo = first - begin();
        auto const hi = last - begin();
        ptr_ = btree_erase(ptr_, lo, hi, detail::encoding_breakage_ok);

        return *this;
    }

    template <typename CharRange>
    auto rope::replace (rope_view old_substr, CharRange const & r)
        -> detail::rope_rng_ret_t<rope &, CharRange>
    { return replace(old_substr, text_view(&*r.begin(), r.end() - r.begin())); }

    inline rope & rope::replace (rope_view old_substr, rope_view rv)
    {
        assert(self_reference(old_substr));

        detail::node_ptr extra_ref;
        rope extra_rope;
        if (self_reference(rv)) {
            extra_ref = ptr_;
            extra_rope = rope(extra_ref);
            rope_view::rope_ref rope_ref = rv.ref_.r_;
            rv = rope_view(extra_rope, rope_ref.lo_, rope_ref.hi_);
        }

        return erase(old_substr).insert(old_substr.ref_.r_.lo_, rv);
    }

    inline rope & rope::replace (rope_view old_substr, text && t)
    { return erase(old_substr).insert(old_substr.ref_.r_.lo_, std::move(t)); }

    inline rope & rope::replace (rope_view old_substr, text_view tv)
    { return erase(old_substr).insert(old_substr.ref_.r_.lo_, tv); }

    inline rope & rope::replace (rope_view old_substr, repeated_text_view rtv)
    { return erase(old_substr).insert(old_substr.ref_.r_.lo_, rtv); }

    template <typename Iter>
    auto rope::replace (rope_view old_substr, Iter first, Iter last)
        -> detail::char_iter_ret_t<rope &, Iter>
    {
        assert(self_reference(old_substr));
        assert(0 <= old_substr.size());
        const_iterator const old_first = old_substr.begin().as_rope_iter();
        return replace(old_first, old_first + old_substr.size(), first, last);
    }

    template <typename Iter>
    auto rope::replace (const_iterator old_first, const_iterator old_last, Iter new_first, Iter new_last)
        -> detail::char_iter_ret_t<rope &, Iter>
    {
        assert(old_first <= old_last);
        assert(begin() <= old_first && old_last <= end());
        return erase(old_first, old_last).insert(old_first, new_first, new_last);
    }

    inline rope & rope::operator+= (rope_view rv)
    { return insert(size(), rv); }

    inline rope & rope::operator+= (text && t)
    { return insert(size(), std::move(t)); }

    inline rope & rope::operator+= (text_view tv)
    { return insert(size(), tv); }

    inline rope & rope::operator+= (repeated_text_view rtv)
    { return insert(size(), rtv); }

    template <typename CharRange>
    auto rope::operator+= (CharRange const & r)
        -> detail::rope_rng_ret_t<rope &, CharRange>
    { return insert(size(), text_view(&*r.begin(), r.end() - r.begin())); }

    inline rope::const_iterator rope::begin () const noexcept
    { return const_iterator(*this, 0); }
    inline rope::const_iterator rope::end () const noexcept
    { return const_iterator(*this, size()); }

    inline rope::const_reverse_iterator rope::rbegin () const noexcept
    { return const_reverse_iterator(const_iterator(*this, size() - 1)); }
    inline rope::const_reverse_iterator rope::rend () const noexcept
    { return const_reverse_iterator(const_iterator(*this, -1)); }

    inline rope_view rope::operator() (int lo, int hi) const
    {
        if (lo < 0)
            lo += size();
        if (hi < 0)
            hi += size();
        assert(0 <= lo && lo <= size());
        assert(0 <= hi && hi <= size());
        assert(lo <= hi);
        return rope_view(*this, lo, hi);
    }

    inline rope_view rope::operator() (int cut) const
    {
        int lo = 0;
        int hi = cut;
        if (cut < 0) {
            lo = cut + size();
            hi = size();
        }
        assert(0 <= lo && lo <= size());
        assert(0 <= hi && hi <= size());
        return rope_view(*this, lo, hi);
    }

    inline rope rope::substr (size_type lo, size_type hi) const
    {
        if (lo < 0)
            lo += size();
        if (hi < 0)
            hi += size();
        assert(0 <= lo && lo <= size());
        assert(0 <= hi && hi <= size());
        assert(lo <= hi);

        if (lo == hi)
            return rope();

        auto const check_ends = (*this)(lo, hi);
        (void)check_ends;

        // If the entire substring falls within a single segment, slice
        // off the appropriate part of that segment.
        detail::found_leaf found;
        detail::find_leaf(ptr_, lo, found);
        if (found.offset_ + hi - lo <= detail::size(found.leaf_->get())) {
            return rope(
                slice_leaf(
                    *found.leaf_,
                    found.offset_,
                    found.offset_ + hi - lo,
                    true,
                    detail::check_encoding_breakage
                )
            );
        }

        // Take an extra ref to the root, which will force all a clone of
        // all the interior nodes.
        detail::node_ptr new_root = ptr_;

        new_root = detail::btree_erase(new_root, hi, size());
        new_root = detail::btree_erase(new_root, 0, lo);

        return rope(new_root);
    }

    inline rope rope::substr (size_type cut) const
    {
        int lo = 0;
        int hi = cut;
        if (cut < 0) {
            lo = cut + size();
            hi = size();
        }
        assert(0 <= lo && lo <= size());
        assert(0 <= hi && hi <= size());
        return substr(lo, hi);
    }

    inline rope::const_iterator begin (rope const & r) noexcept
    { return r.begin(); }
    inline rope::const_iterator end (rope const & r) noexcept
    { return r.end(); }

    inline rope::const_reverse_iterator rbegin (rope const & r) noexcept
    { return r.rbegin(); }
    inline rope::const_reverse_iterator rend (rope const & r) noexcept
    { return r.rend(); }

    inline bool rope::self_reference (rope_view rv) const
    { return rv.which_ == rope_view::which::r && rv.ref_.r_.r_ == this; }

#endif

    /** Creates a new rope object that is the concatenation of r and rv. */
    inline rope operator+ (rope r, rope_view rv)
    { return r.insert(r.size(), rv); }

    /** Creates a new rope object that is the concatenation of rv and r. */
    inline rope operator+ (rope_view rv, rope r)
    { return r.insert(0, rv); }

    /** Creates a new rope object that is the concatenation of r and r2, by
        moving the contents of r2 into the result. */
    inline rope operator+ (rope r, rope && r2)
    { return r += std::move(r2); }

    // TODO: Test all these overloads.
    /** Creates a new rope object that is the concatenation of r2 and r, by
        moving the contents of r2 into the result. */
    inline rope operator+ (rope && r2, rope r)
    { return r2 += std::move(r); }

    /** Creates a new rope object that is the concatenation of r and t, by
        moving the contents of t into the result. */
    inline rope operator+ (rope r, text && t)
    { return r.insert(r.size(), std::move(t)); }

    /** Creates a new rope object that is the concatenation of t and r, by
        moving the contents of t into the result. */
    inline rope operator+ (text && t, rope r)
    { return r.insert(0, std::move(t)); }

    /** Creates a new rope object that is the concatenation of r and tv. */
    inline rope operator+ (rope r, text_view tv)
    { return r.insert(r.size(), tv); }

    /** Creates a new rope object that is the concatenation of tv and r. */
    inline rope operator+ (text_view tv, rope r)
    { return r.insert(0, tv); }

    /** Creates a new rope object that is the concatenation of r and rtv. */
    inline rope operator+ (rope r, repeated_text_view rtv)
    { return r.insert(r.size(), rtv); }

    /** Creates a new rope object that is the concatenation of rtv and r. */
    inline rope operator+ (repeated_text_view rtv, rope r)
    { return r.insert(0, rtv); }

#ifdef BOOST_TEXT_DOXYGEN

    /** Creates a new rope object that is the concatenation of r and range.

        This function only participates in overload resolution if CharRange
        models the Char_range concept.

        \throw std::invalid_argument if the ends of the range are not valid
        UTF-8. */
    template <typename CharRange>
    rope operator+ (rope r, CharRange const & range);

    /** Creates a new rope object that is the concatenation of range and r.

        This function only participates in overload resolution if CharRange
        models the Char_range concept.

        \throw std::invalid_argument if the ends of the range are not valid
        UTF-8. */
    template <typename CharRange>
    rope operator+ (CharRange const & range, rope r);

#else

    template <typename CharRange>
    auto operator+ (rope r, CharRange const & range)
        -> detail::rope_rng_ret_t<rope, CharRange>
    { return r.insert(r.size(), text_view(&*range.begin(), range.end() - range.begin())); }

    template <typename CharRange>
    auto operator+ (CharRange const & range, rope r)
        -> detail::rope_rng_ret_t<rope, CharRange>
    { return r.insert(0, text_view(&*range.begin(), range.end() - range.begin())); }

#endif


    inline rope const & checked_encoding (rope const & r)
    {
        r.foreach_segment(detail::segment_encoding_checker{});
        return r;
    }

    inline rope && checked_encoding (rope && r)
    {
        r.foreach_segment(detail::segment_encoding_checker{});
        return std::move(r);
    }


    namespace detail {

#ifdef BOOST_TEXT_TESTING
        inline void dump_tree (node_ptr const & root, int key, int indent)
        {
            std::cout << repeated_text_view("    ", indent)
                      << (root->leaf_ ? "LEAF" : "INTR")
                      << " @0x" << std::hex << root.get();
            if (key != -1)
                std::cout << " < " << key;
            std::cout << " (" << root->refs_ << " refs)\n";
            if (!root->leaf_) {
                int i = 0;
                for (auto const & child : children(root)) {
                    dump_tree(child, keys(root)[i++], indent + 1);
                }
            }
        }
#endif
    }

} }

#endif
