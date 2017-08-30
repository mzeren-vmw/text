#include <boost/text/text.hpp>
#include <iostream>


boost::text::text string_from_terminal ()
{ return boost::text::text(""); }

int main ()
{

//[ start_with_encoding

    // We can start with a literal or other user-verified encoded string.
    boost::text::text_view tv = "всем привет!\n";

    // Or we can explicitly run over the entire string and check its encoding.
    boost::text::text t = boost::text::checked_encoding(string_from_terminal());
//]
    (void)t;

//[ encoding_affecting_operations
    try {
        // If we try to slice the encoding, we get an exception.
        boost::text::text_view slice_0 = tv(0, 1);
        assert(!"We should never evaluate this assert.");
    } catch (...) {}

    // But as long as we slice along code point boundaries, everything works.
    boost::text::text_view slice_1 = tv(0, 2);
//]
    (void)slice_1;

//[ slicing_on_purpose
    boost::text::text text_slice_0(tv.begin(), tv.begin() + 1);
    boost::text::text text_slice_1(tv.begin() + 1, tv.end());

    std::cout << text_slice_0 << "\n"; // prints "?\n" or some other garbage indicator
//]

//[ repairing_on_purpose_slices
    text_slice_0.insert(
        text_slice_0.end(),
        text_slice_1.begin(), text_slice_1.end()
    );

    std::cout << text_slice_0; // prints "всем привет!\n"
//]
}
