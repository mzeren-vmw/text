#include <boost/text/rope.hpp>

#include <gtest/gtest.h>

#include <list>


using namespace boost;

TEST(rope, test_empty)
{
    text::rope t;

    EXPECT_EQ(t.begin(), t.end());
    EXPECT_EQ(t.rbegin(), t.rend());

    EXPECT_TRUE(t.empty());
    EXPECT_EQ(t.size(), 0);
    EXPECT_EQ(t.begin(), t.end());

    EXPECT_EQ(t.max_size(), PTRDIFF_MAX);

    EXPECT_EQ(t.compare(t), 0);
    EXPECT_TRUE(t == t);
    EXPECT_FALSE(t != t);
    EXPECT_FALSE(t < t);
    EXPECT_TRUE(t <= t);
    EXPECT_FALSE(t > t);
    EXPECT_TRUE(t >= t);

    t.swap(t);
    EXPECT_TRUE(t == t);

    EXPECT_EQ(t.begin(), begin(t));
    EXPECT_EQ(t.end(), end(t));

    EXPECT_EQ(t.rbegin(), rbegin(t));
    EXPECT_EQ(t.rend(), rend(t));

    t.clear();

    std::cout << "t=\"" << t << "\"\n";

    {
        using namespace text::literals;
        text::rope t2(""_t);
        EXPECT_TRUE(t == t2);

        text::rope t3(u8""_t);
        EXPECT_TRUE(t == t3);

        text::rope t4(u""_t);
        EXPECT_TRUE(t == t4);

        text::rope t5(U""_t);
        EXPECT_TRUE(t == t5);
    }
}

TEST(rope, test_non_empty_const_interface)
{
    text::rope t_a("a");
    text::rope t_ab("ab");

    EXPECT_EQ(t_a.begin() + t_a.size(), t_a.end());
    EXPECT_EQ(t_a.rbegin() + t_a.size(), t_a.rend());

    EXPECT_FALSE(t_a.empty());
    EXPECT_EQ(t_a.size(), 1);

    text::rope_view tv_a = t_a;
    EXPECT_EQ(tv_a, t_a);

    EXPECT_EQ(t_a, t_a(0, 1));
    EXPECT_EQ(t_a, t_a(1));
    EXPECT_EQ(t_a, t_a(-1));

    EXPECT_EQ("", t_a(1, 1));

    EXPECT_FALSE(t_ab.empty());
    EXPECT_EQ(t_ab.size(), 2);

    text::rope_view tv_ab = t_ab;
    EXPECT_EQ(tv_ab, t_ab);

    EXPECT_EQ(t_ab, t_ab(0, 2));
    EXPECT_EQ(t_ab, t_ab(2));
    EXPECT_EQ(t_ab, t_ab(-2));

    EXPECT_EQ(t_ab[1], 'b');

    EXPECT_EQ(t_a.max_size(), PTRDIFF_MAX);
    EXPECT_EQ(t_ab.max_size(), PTRDIFF_MAX);

    EXPECT_EQ(t_a.compare(t_ab), -1);
    EXPECT_FALSE(t_a == t_ab);
    EXPECT_TRUE(t_a != t_ab);
    EXPECT_TRUE(t_a < t_ab);
    EXPECT_TRUE(t_a <= t_ab);
    EXPECT_FALSE(t_a > t_ab);
    EXPECT_FALSE(t_a >= t_ab);

    EXPECT_FALSE(t_a == t_ab(2));
    EXPECT_TRUE(t_a != t_ab(2));
    EXPECT_TRUE(t_a < t_ab(2));
    EXPECT_TRUE(t_a <= t_ab(2));
    EXPECT_FALSE(t_a > t_ab(2));
    EXPECT_FALSE(t_a >= t_ab(2));

    EXPECT_FALSE(t_a(1) == t_ab);
    EXPECT_TRUE(t_a(1) != t_ab);
    EXPECT_TRUE(t_a(1) < t_ab);
    EXPECT_TRUE(t_a(1) <= t_ab);
    EXPECT_FALSE(t_a(1) > t_ab);
    EXPECT_FALSE(t_a(1) >= t_ab);

    EXPECT_FALSE(t_a == "ab");
    EXPECT_TRUE(t_a != "ab");
    EXPECT_TRUE(t_a < "ab");
    EXPECT_TRUE(t_a <= "ab");
    EXPECT_FALSE(t_a > "ab");
    EXPECT_FALSE(t_a >= "ab");

    EXPECT_FALSE("a" == t_ab);
    EXPECT_TRUE("a" != t_ab);
    EXPECT_TRUE("a" < t_ab);
    EXPECT_TRUE("a" <= t_ab);
    EXPECT_FALSE("a" > t_ab);
    EXPECT_FALSE("a" >= t_ab);

    EXPECT_EQ(t_a.compare(text::rope("ab")), -1);
    EXPECT_EQ(t_a, "a");

    text::rope const old_t_a(t_a);
    text::rope const old_t_ab(t_ab);
    t_a.swap(t_ab);
    EXPECT_EQ(t_a, old_t_ab);
    EXPECT_EQ(t_ab, old_t_a);
    t_a.swap(t_ab);

    EXPECT_EQ(t_a.begin(), begin(t_a));
    EXPECT_EQ(t_a.end(), end(t_a));

    EXPECT_EQ(t_a.rbegin(), rbegin(t_a));
    EXPECT_EQ(t_a.rend(), rend(t_a));

    {
        using namespace text::literals;
        EXPECT_EQ(t_a, "a"_t);
        EXPECT_EQ(t_ab, "ab"_t);
    }
}

TEST(rope, test_ctors)
{
    text::rope t;
    EXPECT_EQ(t, "");
    EXPECT_EQ("", t);

    text::rope t2("A nonemtpy string");
    EXPECT_EQ(t2, "A nonemtpy string");
    EXPECT_EQ("A nonemtpy string", t2);

    text::rope t3(t2);
    EXPECT_EQ(t3, "A nonemtpy string");
    EXPECT_EQ("A nonemtpy string", t3);

    text::rope t4(std::move(t2));
    EXPECT_EQ(t4, "A nonemtpy string");
    EXPECT_EQ("A nonemtpy string", t4);
    EXPECT_EQ(t2, "");
    EXPECT_EQ("", t2);

    std::string const s("An old-school string");
    text::rope t5(s);
    EXPECT_EQ(t5, "An old-school string");
    EXPECT_EQ("An old-school string", t5);

    text::text_view const tv("a view ");
    text::rope t6(tv);
    EXPECT_EQ(t6, "a view ");
    EXPECT_EQ("a view ", t6);

    text::repeated_text_view const rtv(tv, 3);
    text::rope t7(rtv);
    EXPECT_EQ(t7, "a view a view a view ");
    EXPECT_EQ("a view a view a view ", t7);

    std::list<char> const char_list = {'a', ' ', 'l', 'i', 's', 't'};
    text::rope t8(char_list.begin(), char_list.end());
    EXPECT_EQ(t8, "a list");
    EXPECT_EQ("a list", t8);
}

TEST(rope, test_assignment)
{
    {
        text::rope t;
        EXPECT_EQ(t, "");
        text::rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t = t2;
        EXPECT_EQ(t, "A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");
    }

    {
        text::rope t;
        EXPECT_EQ(t, "");
        text::rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t2 = t;
        EXPECT_EQ(t, "");
        EXPECT_EQ(t2, "");
    }

    {
        text::rope t("small");
        EXPECT_EQ(t, "small");
        text::rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t = t2;
        EXPECT_EQ(t, "A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");
    }

    {
        text::rope t("small");
        EXPECT_EQ(t, "small");
        text::rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t2 = t;
        EXPECT_EQ(t, "small");
        EXPECT_EQ(t2, "small");
    }

    {
        text::rope t;
        EXPECT_EQ(t, "");
        text::rope t2;
        EXPECT_EQ(t2, "");

        t = t2;
        EXPECT_EQ(t, "");
        EXPECT_EQ(t2, "");
    }

    {
        text::rope t;
        EXPECT_EQ(t, "");
        text::rope t2;
        EXPECT_EQ(t2, "");

        t2 = t;
        EXPECT_EQ(t, "");
        EXPECT_EQ(t2, "");
    }

    {
        text::rope t;
        EXPECT_EQ(t, "");
        text::rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t = std::move(t2);
        EXPECT_EQ(t, "A nonemtpy string");
        EXPECT_EQ(t2, "");
    }

    {
        text::rope t;
        EXPECT_EQ(t, "");
        text::rope t2("A nonemtpy string");
        EXPECT_EQ(t2, "A nonemtpy string");

        t = std::move(t2);
        EXPECT_EQ(t, "A nonemtpy string");
        EXPECT_EQ(t2, "");
    }

    {
        std::string const s("An old-school string");
        text::rope t;
        t = s;
        EXPECT_EQ(t, "An old-school string");
    }

    {
        text::text_view const tv("a view ");
        text::rope t;
        t = tv;
        EXPECT_EQ(t, "a view ");

        text::repeated_text_view const rtv(tv, 3);
        text::rope t2;
        t2 = rtv;
        EXPECT_EQ(t2, "a view a view a view ");
    }

    {
        text::rope t("small");
        EXPECT_EQ(t, "small");

        t = t(0, t.size());
        EXPECT_EQ(t, "small");
    }

    {
        text::rope t("small");
        EXPECT_EQ(t, "small");

        t = t(2, t.size());
        EXPECT_EQ(t, "all");
    }

    {
        text::rope t("small");
        EXPECT_EQ(t, "small");

        t = t(0, t.size() - 2);
        EXPECT_EQ(t, "sma");
    }

    {
        text::rope t("small");
        EXPECT_EQ(t, "small");

        t = t(1, t.size() - 1);
        EXPECT_EQ(t, "mal");
    }
}

TEST(rope, test_iterators_and_index)
{
    text::rope empty;
    int size = 0;
    for (auto c : empty) {
        (void)c;
        ++size;
    }
    EXPECT_EQ(size, 0);

    {
        text::rope::iterator first = empty.begin();
        text::rope::iterator last = empty.end();
        while (first != last) {
            ++size;
            ++first;
        }
        EXPECT_EQ(size, 0);
    }

    {
        text::rope::reverse_iterator first = empty.rbegin();
        text::rope::reverse_iterator last = empty.rend();
        while (first != last) {
            ++size;
            ++first;
        }
        EXPECT_EQ(size, 0);
    }

    text::rope non_empty("non-empty");

    {
        std::vector<char> vec;

        text::rope::reverse_iterator const r_it_begin = non_empty.rbegin();
        text::rope::reverse_iterator const r_it_end = non_empty.rend();

        text::rope::reverse_iterator r_it = r_it_begin;
        while (r_it != r_it_end) {
            vec.push_back(*r_it);
            ++r_it;
        }

        std::reverse(vec.begin(), vec.end());
        EXPECT_TRUE(std::equal(r_it_end.base(), r_it_begin.base(), vec.begin(), vec.end()));
     }
}

TEST(rope, test_misc)
{
    {
        text::rope t("some text");
        t.clear();
        EXPECT_EQ(t.size(), 0);
    }

    {
        text::rope t1("some");
        text::rope t2("text");
        t1.swap(t2);
        EXPECT_EQ(t1, "text");
        EXPECT_EQ(t2, "some");
    }
}


TEST(rope, test_insert)
{
    text::text_view const tv("a view ");
    text::repeated_text_view const rtv(tv, 3);

    {
        text::rope const ct("string");

        text::rope t0 = ct;
        t0.insert(0, tv);
        EXPECT_EQ(t0, "a view string");

        text::rope t1 = ct;
        t1.insert(1, tv);
        EXPECT_EQ(t1, "sa view tring");

        text::rope t2 = ct;
        t2.insert(2, tv);
        EXPECT_EQ(t2, "sta view ring");

        text::rope t3 = ct;
        t3.insert(3, tv);
        EXPECT_EQ(t3, "stra view ing");

        text::rope t4 = ct;
        t4.insert(4, tv);
        EXPECT_EQ(t4, "stria view ng");

        text::rope t5 = ct;
        t5.insert(5, tv);
        EXPECT_EQ(t5, "strina view g");

        text::rope t6 = ct;
        t6.insert(6, tv);
        EXPECT_EQ(t6, "stringa view ");

        text::rope t7 = ct;
        t7.insert(6, t7(0, 3));
        EXPECT_EQ(t7, "stringstr");

        text::rope t8 = ct;
        t8.insert(2, t8(0, 3));
        EXPECT_EQ(t8, "ststrring");

        text::rope t9 = ct;
        t9.insert(6, t9(3, 6));
        EXPECT_EQ(t9, "stringing");
    }

    {
        text::rope const ct("string");

        text::rope t0 = ct;
        t0.insert(0, rtv);
        EXPECT_EQ(t0, "a view a view a view string");

        text::rope t1 = ct;
        t1.insert(1, rtv);
        EXPECT_EQ(t1, "sa view a view a view tring");

        text::rope t2 = ct;
        t2.insert(2, rtv);
        EXPECT_EQ(t2, "sta view a view a view ring");

        text::rope t3 = ct;
        t3.insert(3, rtv);
        EXPECT_EQ(t3, "stra view a view a view ing");

        text::rope t4 = ct;
        t4.insert(4, rtv);
        EXPECT_EQ(t4, "stria view a view a view ng");

        text::rope t5 = ct;
        t5.insert(5, rtv);
        EXPECT_EQ(t5, "strina view a view a view g");

        text::rope t6 = ct;
        t6.insert(6, rtv);
        EXPECT_EQ(t6, "stringa view a view a view ");

        text::rope t7 = ct;
        t7.insert(6, t7(0, 3));
        EXPECT_EQ(t7, "stringstr");

        text::rope t8 = ct;
        t8.insert(2, t8(0, 3));
        EXPECT_EQ(t8, "ststrring");

        text::rope t9 = ct;
        t9.insert(6, t9(3, 6));
        EXPECT_EQ(t9, "stringing");
    }

    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

    {
        text::rope const ct("string");
        auto const first = text::utf8::from_utf32_iterator<uint32_t const *>(utf32);
        auto const last = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 4);

        text::rope t0 = ct;
        t0.insert(0, first, last);
        EXPECT_EQ(t0, "\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82string");

        text::rope t1 = ct;
        t1.insert(1, first, last);
        EXPECT_EQ(t1, "s\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82tring");

        text::rope t2 = ct;
        t2.insert(2, first, last);
        EXPECT_EQ(t2, "st\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ring");

        text::rope t3 = ct;
        t3.insert(3, first, last);
        EXPECT_EQ(t3, "str\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ing");

        text::rope t4 = ct;
        t4.insert(4, first, last);
        EXPECT_EQ(t4, "stri\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82ng");

        text::rope t5 = ct;
        t5.insert(5, first, last);
        EXPECT_EQ(t5, "strin\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82g");

        text::rope t6 = ct;
        t6.insert(6, first, last);
        EXPECT_EQ(t6, "string\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82");

        // Breaking the encoding is fine with the iterator interface.
        text::rope t7(first, last);
        char const * c_str = "a";
        EXPECT_NO_THROW(t7.insert(t7.end() - 2, c_str, c_str + 1));
    }

    {
        char const * str = "";
        text::text_view const tv(str, 1); // explicitly null-terminated
        text::repeated_text_view const rtv(tv, 3);

        {
            text::rope t("text");
            t.insert(2, tv);
            EXPECT_EQ(t, "text"); // no null in the middle
        }

        {
            text::rope t("text");
            t.insert(2, rtv);
            EXPECT_EQ(t, "text"); // no nulls in the middle
        }
    }

    {
        auto const first = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 3);
        auto const last = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 4);
        text::rope const ct(first, last);
        EXPECT_EQ(ct.size(), 4);

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.insert(0, "something"));
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.insert(1, "something"), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.insert(2, "something"), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.insert(3, "something"), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.insert(4, "something"));
        }

        {
            // Broken encoding in the removed *and* inserted ranges is fine,
            // since the iterator interface is considered unsafe.
            text::rope t = ct;
            auto final_cp_plus_one = first;
            ++final_cp_plus_one;
            EXPECT_NO_THROW(t.insert(4, final_cp_plus_one, last));
        }

        {
            // Broken encoding due to the insertion point *and* inserted
            // ranges is fine, since the iterator interface is considered
            // unsafe.
            text::rope t = ct;
            auto final_cp_plus_one = first;
            ++final_cp_plus_one;
            EXPECT_NO_THROW(t.insert(t.begin() + 1, final_cp_plus_one, last));
        }
    }
}

TEST(rope, test_erase)
{
    {
        text::rope t("string");
        text::rope_view const ctv(t, 0, t.size());
        t.erase(ctv);
        EXPECT_EQ(t, "");
    }

    text::rope const ct("string");

    for (int j = 0; j <= ct.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
            text::rope t = ct;
            text::rope_view const before = t(0, i);
            text::rope_view const substr = t(i, j);
            text::rope_view const after = t(j, t.size());

            text::rope expected(before);
            expected += after;

            t.erase(substr);
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '" << substr << "'";
        }
    }

    {
        // Unicode 9, 3.9/D90
        uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

        auto const first = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 3);
        auto const last = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 4);
        text::rope const ct(first, last);
        EXPECT_EQ(ct.size(), 4);

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.erase(t));
        }

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.erase(t(0, 0)));
        }

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.erase(t(1, 1)));
        }

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.erase(t(2, 2)));
        }

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.erase(t(3, 3)));
        }

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.erase(t(4, 4)));
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.erase(t(-1)), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.erase(t(-2)), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.erase(t(-3)), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.erase(t(1)), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.erase(t(2)), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.erase(t(3)), std::invalid_argument);
        }

        {
            // Breaking the encoding is fine with the iterator interface.
            text::rope t = ct;
            EXPECT_NO_THROW(t.erase(t.end() - 2, t.end() - 1));
        }
    }
}

TEST(rope, test_replace)
{
    text::text_view const replacement("REP");
    // Explicitly null-terminated.
    text::text_view const replacement_with_null(replacement.begin(), replacement.size() + 1);

    {
        text::rope t("string");
        text::rope_view const ctv(t, 0, t.size());
        t.replace(ctv, replacement_with_null);
        EXPECT_EQ(t, "REP");
    }

    {
        text::rope t("string");
        text::rope_view const ctv(t, 0, t.size());
        t.replace(ctv, replacement);
        EXPECT_EQ(t, "REP");
    }

    {
        text::rope t("string");
        t.replace(t, replacement);
        EXPECT_EQ(t, "REP");
    }

    {
        text::rope t("string");
        t.replace(t(0, 3), t(2, 6));
        EXPECT_EQ(t, "ringing");
    }

    {
        text::rope t("string");
        t.replace(t(3, 6), t(0, 3));
        EXPECT_EQ(t, "strstr");
    }

    text::rope const ct("string");

    for (int j = 0; j <= ct.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
            text::rope t = ct;
            text::rope_view const before = t(0, i);
            text::rope_view const substr = t(i, j);
            text::rope_view const after = t(j, t.size());

            text::rope expected(before);
            expected += replacement;
            expected += after;

            t.replace(substr, replacement);
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '" << substr << "'";
        }
    }

    text::repeated_text_view const really_long_replacement(replacement, 10);

    for (int j = 0; j <= ct.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
            text::rope t = ct;
            text::rope_view const before = t(0, i);
            text::rope_view const substr = t(i, j);
            text::rope_view const after = t(j, t.size());

            text::rope expected(before);
            expected += really_long_replacement;
            expected += after;

            t.replace(substr, really_long_replacement);
            EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '" << substr << "'";
        }
    }

    {
        // Unicode 9, 3.9/D90
        uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

        auto const first = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 3);
        auto const last = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 4);
        text::rope const ct(first, last);
        EXPECT_EQ(ct.size(), 4);

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.replace(t, "something"));
        }

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.replace(t(0, 0), "something"));
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.replace(t(1, 1), "something"), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.replace(t(2, 2), "something"), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.replace(t(3, 3), "something"), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.replace(t(4, 4), "something"));
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.replace(t(0, -1), "something"), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.replace(t(0, -2), "something"), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.replace(t(0, -3), "something"), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.replace(t(1), "something"), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.replace(t(2), "something"), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.replace(t(3), "something"), std::invalid_argument);
        }
    }
}

TEST(rope, test_replace_iter)
{
    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    auto const first = text::utf8::from_utf32_iterator<uint32_t const *>(utf32);
    auto const final_cp = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 3);
    auto const last = text::utf8::from_utf32_iterator<uint32_t const *>(utf32 + 4);

    text::rope const ct_string("string");
    text::rope const ct_text("text");

    {
        text::rope t = ct_string;
        t.replace(t, final_cp, last);
        EXPECT_EQ(t, "\xf0\x90\x8c\x82");
    }

    {
        text::rope t = ct_text;
        t.replace(t, final_cp, last);
        EXPECT_EQ(t, "\xf0\x90\x8c\x82");
    }

    {
        text::rope t = ct_string;
        t.replace(t, first, last);
        EXPECT_EQ(t, "\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82");
    }

    for (int j = 0; j <= ct_string.size(); ++j) {
        for (int i = 0; i <= j; ++i) {
            {
                text::rope t = ct_string;
                text::rope_view const before = t(0, i);
                text::rope_view const substr = t(i, j);
                text::rope_view const after = t(j, t.size());

                text::rope expected(before);
                expected.insert(expected.size(), final_cp, last);
                expected += after;

                t.replace(substr, final_cp, last);
                EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '" << substr << "'";
            }

            {
                text::rope t = ct_string;
                text::rope_view const before = t(0, i);
                text::rope_view const substr = t(i, j);
                text::rope_view const after = t(j, t.size());

                text::rope expected(before);
                expected.insert(expected.size(), first, last);
                expected += after;

                t.replace(substr, first, last);
                EXPECT_EQ(t, expected) << "i=" << i << " j=" << j << " erasing '" << substr << "'";
            }
        }
    }

    {
        text::rope const ct(final_cp, last);
        EXPECT_EQ(ct.size(), 4);

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.replace(t, final_cp, last));
        }

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.replace(t(0, 0), final_cp, last));
        }

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.replace(t(1, 1), final_cp, last));
        }

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.replace(t(2, 2), final_cp, last));
        }

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.replace(t(3, 3), final_cp, last));
        }

        {
            text::rope t = ct;
            EXPECT_NO_THROW(t.replace(t(4, 4), final_cp, last));
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.replace(t(0, -1), final_cp, last), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.replace(t(0, -2), final_cp, last), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.replace(t(0, -3), final_cp, last), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.replace(t(1), final_cp, last), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.replace(t(2), final_cp, last), std::invalid_argument);
        }

        {
            text::rope t = ct;
            EXPECT_THROW(t.replace(t(3), final_cp, last), std::invalid_argument);
        }

        {
            // Broken encoding in the inserted range is fine, since the
            // iterator interface is considered unsafe.
            text::rope t = ct;
            auto final_cp_plus_one = final_cp;
            ++final_cp_plus_one;
            EXPECT_NO_THROW(t.replace(t, final_cp_plus_one, last));
        }

        {
            // Broken encoding in the removed *and* inserted ranges is fine,
            // since the iterator interface is considered unsafe.
            text::rope t = ct;
            auto final_cp_plus_one = final_cp;
            ++final_cp_plus_one;
            EXPECT_NO_THROW(t.replace(t.begin() + 1, t.end(), final_cp_plus_one, last));
            EXPECT_EQ(t, ct);
        }
    }
}

TEST(rope, test_replace_iter_large_insertions)
{
    // Unicode 9, 3.9/D90
    uint32_t const utf32[] = {0x004d, 0x0430, 0x4e8c, 0x10302};

    std::vector<uint32_t> utf32_repeated;
    for (int i = 0; i < 5000; ++i) {
        utf32_repeated.insert(utf32_repeated.end(), utf32, utf32 + 4);
    }
    auto const first = text::utf8::from_utf32_iterator<std::vector<uint32_t>::iterator>(utf32_repeated.begin());
    auto const last = text::utf8::from_utf32_iterator<std::vector<uint32_t>::iterator>(utf32_repeated.end());

    {
        text::rope t("string");
        t.replace(t, first, last);
        text::rope const expected(first, last);
        EXPECT_EQ(t, expected);
    }

    {
        text::rope t;
        t.replace(t, first, last);
        text::rope const expected(first, last);
        EXPECT_EQ(t, expected);
    }
}

// TODO: Add out-of-memory tests (in another file).  These should especially
// test the Iter interfaces.
