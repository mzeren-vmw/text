#ifndef EDITOR_BUFFER_HPP
#define EDITOR_BUFFER_HPP

#include "event.hpp"

#include <boost/text/rope.hpp>
#include <boost/text/segmented_vector.hpp>
#include <boost/filesystem/fstream.hpp>

#include <vector>


struct line_size_t
{
    int code_units_ = 0;
    int code_points_ = 0;
};

struct snapshot_t
{
    boost::text::rope content_;
    boost::text::segmented_vector<line_size_t> line_sizes_;
    int first_row_ = 0;
    int desired_col_ = 0;
    screen_pos_t cursor_pos_;
    std::ptrdiff_t first_char_index_ = 0;
};

struct buffer_t
{
    snapshot_t snapshot_;
    boost::filesystem::path path_;
    std::vector<snapshot_t> history_;
};

inline bool dirty (buffer_t const & b)
{ return !b.snapshot_.content_.equal_root(b.history_.front().content_); }

template <typename Iter>
Iter advance_by_code_point (Iter it, int code_points)
{
    while (code_points) {
        int const bytes = boost::text::utf8::code_point_bytes(*it);
        assert(0 < bytes);
        it += bytes;
        --code_points;
    }
    return it;
}

inline std::ptrdiff_t cursor_line (snapshot_t const & snapshot)
{ return snapshot.first_row_ + snapshot.cursor_pos_.row_; }

struct cursor_offset_t
{
    std::ptrdiff_t rope_offset_;
    line_size_t line_offset_;
};

inline cursor_offset_t cursor_offset (snapshot_t const & snapshot)
{
    std::ptrdiff_t rope_offset = snapshot.first_char_index_;
    for (int i = snapshot.first_row_, end = cursor_line(snapshot);
         i < end;
         ++i) {
        rope_offset += snapshot.line_sizes_[i].code_units_;
    }
    auto const it = snapshot.content_.begin() + rope_offset;
    auto const last = advance_by_code_point(it, snapshot.cursor_pos_.col_);
    int const line_code_units = last - it;
    rope_offset += line_code_units;
    return cursor_offset_t{rope_offset, {line_code_units, snapshot.cursor_pos_.col_}};
}

inline buffer_t load (boost::filesystem::path path, int screen_width)
{
    boost::filesystem::ifstream ifs(path);

    snapshot_t snapshot;
    int line_size = 0;
    int line_cps = 0;
    while (ifs.good()) {
        boost::text::text chunk;
        int const chunk_size = 1 << 16;
        chunk.resize(chunk_size, ' ');
        ifs.read(chunk.begin(), chunk_size);
        if (!ifs.good())
            chunk.resize(ifs.gcount(), ' ');

        auto prev_it = chunk.cbegin();
        auto it = prev_it;

        while (it != chunk.end()) {
            it = std::find(prev_it, chunk.cend(), '\n');
            auto it_for_counting_cps = it;
            if (it != chunk.end() && it != chunk.begin() && it[-1] == '\r')
                --it_for_counting_cps;
            line_cps += std::distance(
                boost::text::utf8::to_utf32_iterator<boost::text::text::const_iterator>(prev_it),
                boost::text::utf8::to_utf32_iterator<boost::text::text::const_iterator>(it_for_counting_cps)
            );
            if (it != chunk.end())
                ++it;
            line_size += it - prev_it;

            auto prev_width_end = prev_it;
            while (screen_width < line_cps) {
                line_cps -= screen_width;
                auto const width_end = advance_by_code_point(prev_width_end, screen_width);
                int const code_units = width_end - prev_width_end;
                line_size -= code_units;
                snapshot.line_sizes_.push_back({code_units, screen_width});
                prev_width_end = width_end;
            }
            snapshot.line_sizes_.push_back({line_size, line_cps});

            prev_it = it;
            line_size = 0;
            line_cps = 0;
        }
        snapshot.content_ += std::move(chunk);
    }

    if (line_size)
        snapshot.line_sizes_.push_back({line_size, line_cps});

    // TODO
    std::ofstream ofs("lines.txt");
    for (auto const width : snapshot.line_sizes_) {
        ofs << width.code_units_ << " " << width.code_points_ << std::endl;
    }

    return buffer_t{snapshot, path, {1, snapshot}};
}

#endif
