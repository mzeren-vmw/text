#ifndef BOOST_TEXT_ROPE_HPP
#define BOOST_TEXT_ROPE_HPP

#include <boost/text/text_view.hpp>
#include <boost/text/text.hpp>

#include <boost/container/static_vector.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>

#include <vector>


namespace boost { namespace text {

    namespace detail {

        // TODO: Encoding breakage checks.
        // TODO: Leaf link fixups when erasing.
        // TOOD: Evaluate exception guarantee.

        struct node_t;
        struct leaf_node_t;
        struct interior_node_t;
        struct node_ptr;

        struct mutable_node_ptr
        {
            ~mutable_node_ptr ();

            explicit operator bool () const
            { return ptr_; }

            node_t * operator-> ()
            { return ptr_; }

            leaf_node_t * as_leaf ();
            interior_node_t * as_interior ();

        private:
            mutable_node_ptr (node_ptr & np, node_t * ptr) : node_ptr_ (np), ptr_ (ptr) {}
            node_ptr & node_ptr_;
            node_t * ptr_;
            friend node_ptr;
        };

        struct node_ptr
        {
            node_ptr () {}
            node_ptr (node_t const * node) : ptr_ (node) {}

            explicit operator bool () const
            { return ptr_.get(); }

            node_t const * operator-> () const
            { return ptr_.get(); }

            leaf_node_t const * as_leaf () const;
            interior_node_t const * as_interior () const;

            node_t const * get () const
            { return ptr_.get(); }

            mutable_node_ptr write () const;

            void swap (node_ptr & rhs)
            { ptr_.swap(rhs.ptr_); }

        private:
            boost::intrusive_ptr<node_t const> ptr_;
            friend mutable_node_ptr;
        };

        struct reference
        {
            reference (node_ptr text_node, text_view ref) noexcept;

            node_ptr text_;
            text_view ref_;
        };

        constexpr int node_buf_size () noexcept
        {
            int alignment = alignof(text);
            alignment = alignment < alignof(text_view) ? alignof(text_view) : alignment;
            alignment = alignment < alignof(repeated_text_view) ? alignof(repeated_text_view) : alignment;
            alignment = alignment < alignof(reference) ? alignof(reference) : alignment;
            int size = sizeof(text);
            size = size < sizeof(text_view) ? alignof(text_view) : size;
            size = size < sizeof(repeated_text_view) ? alignof(repeated_text_view) : size;
            size = size < sizeof(reference) ? alignof(reference) : size;
            return alignment + size;
        }

        template <typename T>
        constexpr void * placement_address (void * buf, std::size_t buf_size) noexcept
        {
            std::size_t const alignment = alignof(T);
            std::size_t const size = sizeof(T);
            return std::align(alignment, size, buf, buf_size);
        }

        struct node_t
        {
            enum class which : char { t, tv, rtv, ref };

            explicit node_t (bool leaf) : refs_ (0), leaf_ (leaf) {}
            node_t (node_t const & rhs) : refs_ (0), leaf_ (rhs.leaf_) {}
            node_t & operator= (node_t const & rhs) = delete;

            mutable int refs_;
            bool leaf_;
        };

        constexpr int min_children = 4;
        constexpr int max_children = 8;
        constexpr int text_insert_max = 512;

        inline std::ptrdiff_t size (node_t const * node);

        using keys_t = boost::container::static_vector<std::ptrdiff_t, max_children>;
        using children_t = boost::container::static_vector<node_ptr, max_children>;

        struct interior_node_t : node_t
        {
            interior_node_t () : node_t (false) {}

            bool full () const
            { return children_.size() == max_children; }

            std::ptrdiff_t offset (int i) const
            {
                assert(0 <= i);
                assert(i <= keys_.size());
                if (i == 0)
                    return 0;
                return keys_[i - 1];
            }

            alignas(64) keys_t keys_;
            children_t children_;
        };

        struct leaf_node_t : node_t
        {
            leaf_node_t () :
                node_t (false),
                buf_ptr_ (nullptr),
                prev_ (nullptr),
                next_ (nullptr),
                size_ (0)
            {}

            ~leaf_node_t ()
            {
                if (!buf_ptr_)
                    return;

                switch (which_) {
                case which::t: as_text().~text(); break;
                case which::tv: as_text_view().~text_view (); break;
                case which::rtv: as_repeated_text_view().~repeated_text_view(); break;
                case which::ref: as_reference().~reference(); break;
                default: assert(!"unhandled rope node case"); break;
                }
            }

            text const & as_text () const
            {
                assert(which_ == which::t);
                return *static_cast<text *>(buf_ptr_);
            }

            text_view const & as_text_view () const
            {
                assert(which_ == which::tv);
                return *static_cast<text_view *>(buf_ptr_);
            }

            repeated_text_view const & as_repeated_text_view () const
            {
                assert(which_ == which::rtv);
                return *static_cast<repeated_text_view *>(buf_ptr_);
            }

            reference const & as_reference () const
            {
                assert(which_ == which::ref);
                return *static_cast<reference *>(buf_ptr_);
            }

            text & as_text ()
            {
                assert(which_ == which::t);
                return *static_cast<text *>(buf_ptr_);
            }

            text_view & as_text_view ()
            {
                assert(which_ == which::tv);
                return *static_cast<text_view *>(buf_ptr_);
            }

            repeated_text_view & as_repeated_text_view ()
            {
                assert(which_ == which::rtv);
                return *static_cast<repeated_text_view *>(buf_ptr_);
            }

            reference & as_reference ()
            {
                assert(which_ == which::ref);
                return *static_cast<reference *>(buf_ptr_);
            }

            char buf_[node_buf_size()];
            void * buf_ptr_;
            leaf_node_t * prev_;
            leaf_node_t * next_;
            int size_;
            which which_;
        };

        mutable_node_ptr::~mutable_node_ptr ()
        { node_ptr_.ptr_ = ptr_; }

        leaf_node_t * mutable_node_ptr::as_leaf ()
        {
            assert(ptr_);
            assert(ptr_->leaf_);
            return static_cast<leaf_node_t *>(ptr_);
        }

        interior_node_t * mutable_node_ptr::as_interior ()
        {
            assert(ptr_);
            assert(!ptr_->leaf_);
            return static_cast<interior_node_t *>(ptr_);
        }

        leaf_node_t const * node_ptr::as_leaf () const
        {
            assert(ptr_);
            assert(ptr_->leaf_);
            return static_cast<leaf_node_t const *>(ptr_.get());
        }

        interior_node_t const * node_ptr::as_interior () const
        {
            assert(ptr_);
            assert(!ptr_->leaf_);
            return static_cast<interior_node_t const *>(ptr_.get());
        }

        mutable_node_ptr node_ptr::write () const
        {
            auto this_ref = const_cast<node_ptr &>(*this);
            if (ptr_->refs_ == 1)
                return mutable_node_ptr(this_ref, const_cast<node_t *>(ptr_.get()));
            if (ptr_->leaf_)
                return mutable_node_ptr(this_ref, new leaf_node_t(*as_leaf()));
            else
                return mutable_node_ptr(this_ref, new interior_node_t(*as_interior()));
        }

        inline void intrusive_ptr_add_ref (node_t const * node)
        { ++node->refs_; }
        inline void intrusive_ptr_release (node_t const * node)
        {
            if (!--node->refs_) {
                if (node->leaf_)
                    delete static_cast<leaf_node_t const *>(node);
                else
                    delete static_cast<interior_node_t const *>(node);
            }
        }

        inline std::ptrdiff_t size (node_t const * node)
        {
            assert(node);
            if (node->leaf_)
                return static_cast<leaf_node_t const *>(node)->size_;
            else
                return static_cast<interior_node_t const *>(node)->keys_.back();
        }

        inline children_t const & children (node_ptr const & node)
        { return node.as_interior()->children_; }

        inline children_t & children (mutable_node_ptr & node)
        { return node.as_interior()->children_; }

        inline keys_t const & keys (node_ptr const & node)
        { return node.as_interior()->keys_; }

        inline keys_t & keys (mutable_node_ptr & node)
        { return node.as_interior()->keys_; }

        inline int num_children (node_ptr const & node)
        { return static_cast<int>(children(node).size()); }

        inline int num_children (mutable_node_ptr & node)
        { return static_cast<int>(children(node).size()); }

        inline int num_keys (node_ptr const & node)
        { return static_cast<int>(keys(node).size()); }

        inline int num_keys (mutable_node_ptr & node)
        { return static_cast<int>(keys(node).size()); }

        inline std::ptrdiff_t find_child (interior_node_t const * node, std::ptrdiff_t n)
        {
            int i = 0;
            auto const sizes = static_cast<int>(node->keys_.size());
            while (i < sizes && node->keys_[i] <= n) {
                ++i;
            }
            assert(i < sizes);
            return i;
        }

        struct found_leaf
        {
            leaf_node_t * leaf_;
            std::ptrdiff_t offset_;
            alignas(64) boost::container::static_vector<interior_node_t *, 24> path_;
        };

        inline void find_leaf (
            node_ptr node,
            std::ptrdiff_t n,
            found_leaf & retval
        ) {
            assert(node);
            assert(n <= size(node.get()));
            if (node->leaf_) {
                auto mut_node = node.write();
                retval.leaf_ = mut_node.as_leaf();
                retval.offset_ = n;
                return;
            }
            auto const i = find_child(node.as_interior(), n);
            node_ptr next_node = children(node)[i];
            {
                auto mut_node = node.write();
                retval.path_.push_back(mut_node.as_interior());
            }
            auto const offset = node.as_interior()->offset(i);
            find_leaf(next_node, n - offset, retval);
        }

        struct found_char
        {
            found_leaf leaf_;
            char c_;
        };

        inline void find_char (node_ptr node, std::ptrdiff_t n, found_char & retval)
        {
            assert(node);
            find_leaf(node, n, retval.leaf_);

            leaf_node_t * leaf = retval.leaf_.leaf_;
            char c = '\0';
            switch (leaf->which_) {
            case node_t::which::t:
                c = *(leaf->as_text().cbegin() + n);
                break;
            case node_t::which::tv:
                c = *(leaf->as_text_view().begin() + n);
                break;
            case node_t::which::rtv:
                c = *(leaf->as_repeated_text_view().begin() + n);
                break;
            case node_t::which::ref:
                c = *(leaf->as_reference().ref_.begin() + n);
                break;
            default: assert(!"unhandled rope node case"); break;
            }
            retval.c_ = c;
        }

        inline reference::reference (node_ptr text_node, text_view ref) noexcept :
            text_ (text_node),
            ref_ (ref)
        {
            assert(text_node);
            assert(text_node->leaf_);
            assert(text_node.as_leaf()->which_ == node_t::which::t);
        }

        inline node_ptr make_node (text const & t)
        {
            leaf_node_t * leaf = nullptr;
            node_ptr retval(leaf = new leaf_node_t);
            leaf->size_ = t.size();
            leaf->which_ = node_t::which::t;
            auto at = placement_address<text>(leaf->buf_, sizeof(leaf->buf_));
            assert(at);
            leaf->buf_ptr_ = new (at) text(t);
            return retval;
        }

        inline node_ptr make_node (text && t)
        {
            leaf_node_t * leaf = nullptr;
            node_ptr retval(leaf = new leaf_node_t);
            leaf->size_ = t.size();
            leaf->which_ = node_t::which::t;
            auto at = placement_address<text>(leaf->buf_, sizeof(leaf->buf_));
            assert(at);
            leaf->buf_ptr_ = new (at) text(std::move(t));
            return retval;
        }

        inline node_ptr make_node (text_view tv)
        {
            leaf_node_t * leaf = nullptr;
            node_ptr retval(leaf = new leaf_node_t);
            leaf->size_ = tv.size();
            leaf->which_ = node_t::which::tv;
            auto at = placement_address<text_view>(leaf->buf_, sizeof(leaf->buf_));
            assert(at);
            leaf->buf_ptr_ = new (at) text_view(tv);
            return retval;
        }

        inline node_ptr make_node (repeated_text_view rtv)
        {
            leaf_node_t * leaf = nullptr;
            node_ptr retval(leaf = new leaf_node_t);
            leaf->size_ = rtv.size();
            leaf->which_ = node_t::which::rtv;
            auto at = placement_address<repeated_text_view>(leaf->buf_, sizeof(leaf->buf_));
            assert(at);
            leaf->buf_ptr_ = new (at) repeated_text_view(rtv);
            return retval;
        }

        inline node_ptr make_ref (leaf_node_t const * t, std::ptrdiff_t lo, std::ptrdiff_t hi)
        {
            assert(t->which_ == node_t::which::t);
            text_view const tv = t->as_text()(lo, hi);

            leaf_node_t * leaf = nullptr;
            node_ptr retval(leaf = new leaf_node_t);
            leaf->size_ = tv.size();
            leaf->which_ = node_t::which::ref;
            auto at = placement_address<reference>(leaf->buf_, sizeof(leaf->buf_));
            assert(at);
            leaf->buf_ptr_ = new (at) reference(t, tv);
            return retval;
        }

        inline node_ptr make_ref (reference const & t, std::ptrdiff_t lo, std::ptrdiff_t hi)
        {
            auto const offset = t.ref_.begin() - t.text_.as_leaf()->as_text().begin();
            node_ptr retval = make_ref(t.text_.as_leaf(), lo + offset, hi + offset);
            return retval;
        }

        struct const_rope_iterator;
        struct const_reverse_rope_iterator;

        template <typename Container>
        auto reverse (Container const & c)
        {
            return boost::iterator_range<typename Container::const_reverse_iterator>(
                c.rbegin(), c.rend()
            );
        }

        inline void insert_child (interior_node_t * node, int at, node_ptr child)
        {
            auto const child_size = size(child.get());
            node->children_.insert(node->children_.begin() + at, child);
            node->keys_.insert(node->keys_.begin() + at, node->offset(at));
            for (int i = at, size = (int)node->keys_.size(); i < size; ++i) {
                node->keys_[i] += child_size;
            }
        }

        enum erasure_adjustments { adjust_keys, dont_adjust_keys };

        inline void erase_child (interior_node_t * node, int at, erasure_adjustments adj = adjust_keys)
        {
            auto const child_size = size(node->children_[at].get());
            node->children_.erase(node->children_.begin() + at);
            node->keys_.erase(node->keys_.begin() + at);
            if (adj == adjust_keys) {
                for (int i = at, size = (int)node->keys_.size(); i < size; ++i) {
                    node->keys_[i] -= child_size;
                }
            }
        }

        inline node_ptr slice_leaf (leaf_node_t * leaf, std::ptrdiff_t lo, std::ptrdiff_t hi, bool immutable)
        {
            assert(leaf);
            assert(lo < leaf->size_);
            assert(hi < leaf->size_);
            assert(lo < hi);

            bool const leaf_mutable = !immutable && leaf->refs_ == 1;

            switch (leaf->which_) {
            case node_t::which::t:
                if (leaf_mutable) {
                    text & t = leaf->as_text();
                    t.erase(t(lo, hi));
                    return leaf;
                } else {
                    return make_ref(leaf, lo, hi);
                }
            case node_t::which::tv: {
                text_view & tv = leaf->as_text_view();
                if (leaf_mutable) {
                    tv = tv(lo, hi);
                    return leaf;
                } else {
                    return make_node(tv(lo, hi));
                }
                break;
            }
            case node_t::which::rtv: {
                repeated_text_view & rtv = leaf->as_repeated_text_view();
                if (lo % rtv.view().size() == 0 && hi % rtv.view().size() == 0) {
                    auto const count = (hi - lo) / rtv.view().size();
                    if (leaf_mutable) {
                        rtv = repeated_text_view(rtv.view(), count);
                        return leaf;
                    } else {
                        return make_node(repeated_text_view(rtv.view(), count));
                    }
                } else {
                    return make_node(text(rtv.begin() + lo, rtv.begin() + hi));
                }
                break;
            }
            case node_t::which::ref: {
                reference & ref = leaf->as_reference();
                if (leaf_mutable) {
                    ref.ref_ = ref.ref_(lo, hi);
                    return leaf;
                } else {
                    return make_ref(ref, lo, hi);
                }
                break;
            }
            default: assert(!"unhandled rope node case"); break;
            }
            return nullptr; // This should never execute.
        }

        inline node_ptr btree_split_child (node_ptr parent, int i)
        {
            assert(parent.as_interior()->full());

            interior_node_t * new_node = nullptr;
            node_ptr new_node_ptr(new_node = new interior_node_t);

            assert(!children(parent)[i]->leaf_);
            node_ptr child = children(parent)[i];

            {
                interior_node_t const * int_child = child.as_interior();
                new_node->children_.resize(min_children - 1);
                std::copy(
                    int_child->children_.begin() + min_children, int_child->children_.end(),
                    new_node->children_.begin()
                );
                new_node->keys_.resize(min_children - 1);
                auto it = new_node->children_.begin();
                std::ptrdiff_t sum = 0;
                for (auto & key : new_node->keys_) {
                    sum += size(it->get());
                    key = sum;
                    ++it;
                }
            }

            child = children(parent)[i];

            {
                auto mut_parent = parent.write();
                auto mut_child = child.write();
                children(mut_parent).insert(
                    children(mut_parent).begin() + i + 1,
                    new_node
                );
                keys(mut_parent).insert(
                    keys(mut_parent).begin() + i,
                    keys(mut_child)[min_children]
                );
                keys(mut_child).resize(min_children);
            }

            return parent;
        }

        inline void btree_split_leaf (node_ptr parent, int i, std::ptrdiff_t at)
        {
            node_ptr const & child = children(parent)[i];

            auto const child_size = child.as_leaf()->size_;
            auto const offset_at_i = parent.as_interior()->offset(i);
            auto const cut = static_cast<int>(at - offset_at_i);

            node_ptr right;
            node_ptr left;

            {
                auto mut_child = children(parent)[i].write();

                right = slice_leaf(mut_child.as_leaf(), cut, child_size, true);
                left = slice_leaf(mut_child.as_leaf(), 0, cut, child.as_leaf()->which_ == node_t::which::t);

                auto mut_left = left.write();
                auto mut_right = right.write();

                if (mut_child.as_leaf()->prev_)
                    mut_child.as_leaf()->prev_->next_ = mut_left.as_leaf();
                mut_left.as_leaf()->prev_ = mut_child.as_leaf()->prev_;
                mut_left.as_leaf()->next_ = mut_right.as_leaf();
                mut_right.as_leaf()->prev_ = mut_left.as_leaf();
                mut_right.as_leaf()->next_ = mut_child.as_leaf()->next_;
                if (mut_child.as_leaf()->next_)
                    mut_child.as_leaf()->next_->prev_ = mut_right.as_leaf();
            }

            auto mut_parent = parent.write();
            children(mut_parent)[i] = left;
            children(mut_parent).insert(
                children(mut_parent).begin() + i + 1,
                right
            );
            keys(mut_parent).insert(
                keys(mut_parent).begin() + i,
                offset_at_i + cut
            );
        }

        inline node_ptr btree_insert_nonfull (
            node_ptr parent,
            std::ptrdiff_t at,
            node_ptr node
        ) {
            assert(at <= size(parent.get()));

            int i = find_child(parent.as_interior(), at);
            if (children(parent)[i]->leaf_) {
                btree_split_leaf(parent, i, at);
                if (keys(parent)[i] < at)
                    ++i;

                {
                    auto mut_child = children(parent)[i].write();
                    auto mut_node = node.write();
                    if (mut_child.as_leaf()->prev_)
                        mut_child.as_leaf()->prev_->next_ = mut_node.as_leaf();
                    mut_node.as_leaf()->prev_ = mut_child.as_leaf()->prev_;
                    mut_node.as_leaf()->next_ = mut_child.as_leaf();
                    mut_child.as_leaf()->prev_ = mut_node.as_leaf();
                }

                auto mut_parent = parent.write();
                insert_child(mut_parent.as_interior(), i, node);
            } else {
                if (children(parent)[i].as_interior()->full()) {
                    parent = btree_split_child(parent, i);
                    if (keys(parent)[i] < at)
                        ++i;
                }
                auto mut_parent = parent.write();
                auto delta = -size(children(mut_parent)[i].get());
                node_ptr new_child = btree_insert_nonfull(
                    children(mut_parent)[i],
                    at - mut_parent.as_interior()->offset(i),
                    node
                );
                delta += size(new_child.get());
                children(mut_parent)[i] = new_child;
                for (int j = i, size = num_keys(mut_parent); j < size; ++j) {
                    keys(mut_parent)[j] += delta;
                }
            }
            return parent;
        }

        inline node_ptr btree_insert (node_ptr root, std::ptrdiff_t at, node_ptr node)
        {
            assert(root);
            assert(node->leaf_);
            if (root->leaf_) {
                interior_node_t * new_root = nullptr;
                node_ptr new_root_ptr(new_root = new interior_node_t);
                new_root->children_.push_back(root);
                new_root->keys_.push_back(root.as_leaf()->size_);
                return root;
            } else if (root.as_interior()->full()) {
                interior_node_t * new_root = nullptr;
                node_ptr new_root_ptr(new_root = new interior_node_t);
                new_root->children_.push_back(root);
                new_root->keys_.push_back(keys(root).back());
                new_root_ptr = btree_split_child(new_root_ptr, 0);
                return btree_insert_nonfull(new_root_ptr, at, node);
            } else {
                return btree_insert_nonfull(root, at, node);
            }
        }

        struct leaf_slices
        {
            node_ptr slice;
            node_ptr other_slice;
        };

        inline leaf_slices erase_leaf (leaf_node_t * leaf, std::ptrdiff_t lo, std::ptrdiff_t hi)
        {
            assert(leaf);
            assert(lo < leaf->size_);
            assert(hi < leaf->size_);
            assert(lo < hi);

            bool const leaf_mutable = leaf->refs_ == 1;

            leaf_slices retval;

            if (leaf_mutable && leaf->which_ == node_t::which::t) {
                text & t = leaf->as_text();
                t.erase(t(lo, hi));
                retval.slice = leaf;
                return retval;
            }

            auto const leaf_size = leaf->size_;

            if (hi != leaf_size)
                retval.other_slice = slice_leaf(leaf, hi, leaf_size, true);
            if (lo != 0)
                retval.slice = slice_leaf(leaf, 0, lo, false);

            if (!retval.slice)
                retval.slice.swap(retval.other_slice);

            return retval;
        }

        // TODO: node_ptrs must be const refs (or converted to raw pointers)
        // to avoid spurious multiple refs.
        // TODO: This interface in particular must take interior_node_t *
        // node.  Still true with mutable_node_ptr?
        inline node_ptr btree_erase (node_ptr node, std::ptrdiff_t at, leaf_node_t * leaf)
        {
            assert(node);

            if (children(node)[0]->leaf_) {
                auto const child_index = find_child(node.as_interior(), at);

                if (num_children(node) == 2)
                    return children(node)[child_index ? 0 : 1];

                assert(children(node)[child_index].as_leaf() == leaf);

                {
                    auto mut_node = node.write();
                    erase_child(mut_node.as_interior(), child_index);
                }

                return node;
            }

            node_ptr new_child = nullptr;

            auto const child_index = find_child(node.as_interior(), at);

            node_ptr const & child = children(node)[child_index];
            if (num_children(child) == min_children) {
                node_ptr const & child_left_sib =
                    child_index == 0 ?
                    nullptr :
                    children(node)[child_index - 1];
                node_ptr const & child_right_sib =
                    child_index == num_children(node) - 1 ?
                    nullptr :
                    children(node)[child_index + 1];

                assert(child_left_sib || child_right_sib);

                if (child_left_sib &&
                    min_children + 1 <= num_children(child_left_sib)) {
                    // Right-rotate.
                    // Remove last EF element of left sibling.
                    // Prepend EF onto child.
                    // Update node.
                    // new_child = btree_erase(next_node, at - offset, leaf);
                } else if (child_right_sib &&
                           min_children + 1 <= num_children(child_right_sib)) {
                    // Left-rotate.
                    // Remove first E0 element of right sibling.
                    // Append child with E0.
                    // Update node.
                    // new_child = btree_erase(next_node, at - offset, leaf);
                } else {
                    auto const right_index = child_right_sib ? child_index + 1 : child_index;
                    auto const left_index = right_index - 1;

                    node_ptr const & left = child_right_sib ? child : child_left_sib;
                    node_ptr const & right = child_right_sib ? child_right_sib : child;

                    {
                        auto mut_left = left.write();
                        auto mut_right = right.write();

                        // TODO: Employ children() and keys() in more places.
                        children_t & left_children = children(mut_left);
                        children_t & right_children = children(mut_right);

                        left_children.insert(
                            left_children.end(),
                            right_children.begin(),
                            right_children.end()
                        );

                        keys_t & left_keys = keys(mut_left);
                        keys_t & right_keys = keys(mut_right);

                        auto const old_left_size = left_keys.back();
                        int const old_children = num_keys(mut_left);

                        left_keys.insert(
                            left_keys.end(),
                            right_keys.begin(),
                            right_keys.end()
                        );
                        for (int i = old_children, size = num_keys(mut_left); i < size; ++i) {
                            left_keys[i] += old_left_size;
                        }
                    }

                    std::ptrdiff_t const offset = node.as_interior()->offset(left_index);

                    new_child = btree_erase(left, at - offset, leaf);
                    if (num_children(node) == 2)
                        return new_child;

                    auto mut_node = node.write();
                    erase_child(mut_node.as_interior(), right_index, dont_adjust_keys);
                }
            } else {
                std::ptrdiff_t const offset = node.as_interior()->offset(child_index);
                new_child = btree_erase(children(node)[child_index], at - offset, leaf);
            }

            {
                auto mut_node = node.write();
                children(mut_node)[child_index] = new_child;
                auto const size_delta = leaf->size_;
                for (int i = child_index, size = num_keys(mut_node); i < size; ++i) {
                    keys(mut_node)[i] -= size_delta;
                }
            }

            return node;
        }

        inline node_ptr btree_erase (node_ptr root, std::ptrdiff_t lo, std::ptrdiff_t hi)
        {
            assert(root);

            if (root->leaf_) {
                leaf_slices slices;
                {
                    auto mut_root = root.write();
                    slices = erase_leaf(mut_root.as_leaf(), lo, hi);
                }
                if (!slices.other_slice) {
                    return slices.slice;
                } else {
                    interior_node_t * new_root = nullptr;
                    node_ptr new_root_ptr(new_root = new interior_node_t);
                    new_root->children_.push_back(slices.slice);
                    new_root->keys_.push_back(size(slices.slice.get()));
                    new_root->children_.push_back(slices.other_slice);
                    new_root->keys_.push_back(size(slices.other_slice.get()));
                    return new_root;
                }
            } else {
                detail::found_leaf found_lo;
                detail::find_leaf(root, lo, found_lo);
                if (found_lo.offset_ != 0) {
                    // TODO: Insert prefix that's not being erased right
                    // before found_lo.leaf_.
                    // TODO: lo += prefix.size_;
                    // TODO: hi += prefix.size_;
                }

                detail::found_leaf found_hi;
                detail::find_leaf(root, hi, found_hi);
                if (found_hi.offset_ != found_hi.leaf_->size_) {
                    // TODO: Insert suffix that's not being erased right
                    // after found_hi.leaf_.
                }

                leaf_node_t * leaf = found_lo.leaf_;
                while (leaf != found_hi.leaf_) {
                    leaf_node_t * next = leaf->next_;
                    root = btree_erase(root, lo, leaf);
                    leaf = next;
                }
            }

            return root;
        }

    }

    struct rope
    {
        using iterator = detail::const_rope_iterator;
        using const_iterator = detail::const_rope_iterator;
        using reverse_iterator = detail::const_reverse_rope_iterator;
        using const_reverse_iterator = detail::const_reverse_rope_iterator;

        using size_type = std::ptrdiff_t;

        rope () : ptr_ (nullptr) {}

        rope (text const & t) : ptr_ (detail::make_node(t)) {}
        rope (text && t) : ptr_ (detail::make_node(std::move(t))) {}
        rope (text_view tv) : ptr_ (detail::make_node(tv)) {}
        rope (repeated_text_view rtv) : ptr_ (detail::make_node(rtv)) {}

        rope & operator= (text const & t)
        {
            rope temp(t);
            swap(temp);
            return *this;
        }

        rope & operator= (text && t)
        {
            rope temp(std::move(t));
            swap(temp);
            return *this;
        }

        rope & operator= (text_view tv)
        {
            rope temp(tv);
            swap(temp);
            return *this;
        }

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

        char operator[] (size_type n) const noexcept
        {
            assert(ptr_);
            assert(n < size());
            detail::found_char found;
            find_char(ptr_, n, found);
            return found.c_;
        }

        constexpr size_type max_size () const noexcept
        { return PTRDIFF_MAX; }

#if 0
        rope substr (size_type lo, size_type hi) const
        {
            assert(ptr_);
            assert(0 <= lo && lo <= size());
            assert(0 <= hi && hi <= size());
            assert(lo <= hi);

            // TODO:

            // Take an extra ref to the root, which will force all a clone of
            // all the nodes to teh insertion point.
            node_ptr extra_ref = ptr_;

            // TODO: insert slice at leftmost substr leaf.
            // TODO: Go back through nodes exterior to (left of) path to leaf
            // and remove them.
            // TODO: Same thing on the right.

            rope retval;
            retval.ptr_ = new_root;
            return retval;
        }

        rope substr (size_type cut) const
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
#endif

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

        friend const_iterator begin (rope const & r) noexcept;
        friend const_iterator end (rope const & r) noexcept;

        friend const_reverse_iterator rbegin (rope const & r) noexcept;
        friend const_reverse_iterator rend (rope const & r) noexcept;

        friend std::ostream & operator<< (std::ostream & os, rope const & r)
        {
            if (!r.ptr_)
                return os;

            detail::found_leaf found;
            detail::find_leaf(r.ptr_, 0, found);

            detail::leaf_node_t const * leaf = found.leaf_;
            while (leaf) {
                switch (leaf->which_) {
                case detail::node_t::which::t:
                    os << leaf->as_text();
                    break;
                case detail::node_t::which::tv:
                    os << leaf->as_text_view();
                    break;
                case detail::node_t::which::rtv:
                    os << leaf->as_repeated_text_view();
                    break;
                case detail::node_t::which::ref:
                    os << leaf->as_reference().ref_;
                    break;
                default: assert(!"unhandled rope node case"); break;
                }
                leaf = leaf->next_;
            }

            return os;
        }

        void clear ()
        { ptr_ = nullptr; }

        rope & insert (size_type at, rope const & r)
        {
            if (r.empty())
                return *this;

            detail::found_leaf found;
            find_leaf(r.ptr_, 0, found);

            detail::leaf_node_t * leaf = found.leaf_;

            if (!ptr_) {
                ptr_ = leaf;
                leaf = leaf->next_;
            }

            while (leaf) {
                ptr_ = detail::btree_insert(ptr_, at, leaf);
                leaf = leaf->next_;
            }

            return *this;
        }

        rope & insert (size_type at, text const & t)
        { return insert_impl(at, t, true); }

        rope & insert (size_type at, text && t)
        { return insert_impl(at, std::move(t), false); }

        rope & insert (size_type at, text_view tv)
        { return insert_impl(at, tv, false); }

        rope & insert (size_type at, repeated_text_view rtv)
        { return insert_impl(at, rtv, false); }

        rope & erase (size_type lo, size_type hi)
        {
            assert(lo <= size());
            assert(hi <= size());
            assert(lo <= hi);

            if (lo == hi)
                return *this;

            ptr_ = btree_erase(ptr_, lo, hi);

            return *this;
        }

        void swap (rope & rhs)
        { ptr_.swap(rhs.ptr_); }

    private:
        text * mutable_insertion_leaf (size_type at, size_type size, bool insertion_would_allocate)
        {
            detail::found_leaf found;
            find_leaf(ptr_, at, found);

            for (auto node : found.path_) {
                if (1 < node->refs_)
                    return nullptr;
            }

            if (found.leaf_->which_ == detail::node_t::which::t) {
                text & t = found.leaf_->as_text();
                auto const inserted_size = t.size() + size;
                if (inserted_size <= t.capacity())
                    return &t;
                else if (insertion_would_allocate && inserted_size <= detail::text_insert_max)
                    return &t;
            }

            return nullptr;
        }

        template <typename T>
        rope & insert_impl (size_type at, T && t, bool insertion_would_allocate)
        {
            if (t.empty())
                return *this;

            if (!ptr_)
                ptr_ = detail::make_node(std::forward<T &&>(t));
            else if (text * leaf_t = mutable_insertion_leaf(at, t.size(), insertion_would_allocate))
                leaf_t->insert(leaf_t->size(), t);
            else
                ptr_ = detail::btree_insert(ptr_, at, detail::make_node(std::forward<T &&>(t)));

            return *this;
        }

        detail::node_ptr ptr_;

        friend struct detail::const_rope_iterator;
    };

    namespace detail {

        struct const_rope_iterator
        {
            using value_type = char const;
            using difference_type = std::ptrdiff_t;
            using pointer = char const *;
            using reference = char const;
            using iterator_category = std::random_access_iterator_tag;

            const_rope_iterator () noexcept :
                rope_ (nullptr),
                n_ (-1),
                leaf_ (nullptr),
                leaf_start_ (-1)
            {}

            const_rope_iterator (rope const & r, difference_type n) noexcept :
                rope_ (&r),
                n_ (n),
                leaf_ (nullptr),
                leaf_start_ (0)
            {}

            reference operator* () const noexcept
            {
                if (leaf_) {
                    return deref();
                } else {
                    found_char found;
                    find_char(rope_->ptr_, n_, found);
                    leaf_ = found.leaf_.leaf_;
                    leaf_start_ = n_ - found.leaf_.offset_;
                    return found.c_;
                }
            }

            value_type operator[] (difference_type n) const noexcept
            {
                auto it = *this;
                if (0 <= n)
                    it += n;
                else
                    it -= -n;
                return *it;
            }

            const_rope_iterator & operator++ () noexcept
            {
                ++n_;
                if (leaf_ && leaf_start_ + n_ == leaf_->size_) {
                    leaf_ = leaf_->next_;
                    leaf_start_ = n_;
                }
                return *this;
            }
            const_rope_iterator operator++ (int) noexcept
            {
                const_rope_iterator retval = *this;
                ++*this;
                return retval;
            }
            const_rope_iterator & operator+= (difference_type n) noexcept
            {
                n_ += n;
                leaf_ = nullptr;
                return *this;
            }

            const_rope_iterator & operator-- () noexcept
            {
                if (leaf_ && n_ == leaf_start_) {
                    leaf_ = leaf_->prev_;
                    leaf_start_ -= leaf_->size_;
                }
                --n_;
                return *this;
            }
            const_rope_iterator operator-- (int) noexcept
            {
                const_rope_iterator retval = *this;
                --*this;
                return retval;
            }
            const_rope_iterator & operator-= (difference_type n) noexcept
            {
                n_ -= n;
                leaf_ = nullptr;
                return *this;
            }

            friend bool operator== (const_rope_iterator lhs, const_rope_iterator rhs) noexcept
            { return lhs.rope_ == rhs.rope_ && lhs.n_ == rhs.n_; }
            friend bool operator!= (const_rope_iterator lhs, const_rope_iterator rhs) noexcept
            { return !(lhs == rhs); }
            // TODO: Document wonky behavior of the inequalities when rhs.{frst,last}_ != rhs.{first,last}_.
            friend bool operator< (const_rope_iterator lhs, const_rope_iterator rhs) noexcept
            { return lhs.rope_ == rhs.rope_ && lhs.n_ < rhs.n_; }
            friend bool operator<= (const_rope_iterator lhs, const_rope_iterator rhs) noexcept
            { return lhs == rhs || lhs < rhs; }
            friend bool operator> (const_rope_iterator lhs, const_rope_iterator rhs) noexcept
            { return rhs < lhs; }
            friend bool operator>= (const_rope_iterator lhs, const_rope_iterator rhs) noexcept
            { return lhs <= rhs; }

            friend const_rope_iterator operator+ (const_rope_iterator lhs, difference_type rhs) noexcept
            { return lhs += rhs; }
            friend const_rope_iterator operator+ (difference_type lhs, const_rope_iterator rhs) noexcept
            { return rhs += lhs; }
            friend const_rope_iterator operator- (const_rope_iterator lhs, difference_type rhs) noexcept
            { return lhs -= rhs; }
            friend const_rope_iterator operator- (difference_type lhs, const_rope_iterator rhs) noexcept
            { return rhs -= lhs; }
            friend difference_type operator- (const_rope_iterator lhs, const_rope_iterator rhs) noexcept
            {
                // TODO: Document this precondition!
                assert(lhs.rope_ == rhs.rope_);
                return rhs.n_ - lhs.n_;
            }

        private:
            char deref () const
            {
                switch (leaf_->which_) {
                case node_t::which::t: {
                    text const * t = static_cast<text *>(leaf_->buf_ptr_);
                    return *(t->begin() + (n_ - leaf_start_));
                }
                case node_t::which::tv: {
                    text_view const * tv = static_cast<text_view *>(leaf_->buf_ptr_);
                    return *(tv->begin() + (n_ - leaf_start_));
                }
                case node_t::which::rtv: {
                    repeated_text_view const * rtv = static_cast<repeated_text_view *>(leaf_->buf_ptr_);
                    return *(rtv->begin() + (n_ - leaf_start_));
                }
                case node_t::which::ref: {
                    detail::reference const * ref = static_cast<detail::reference *>(leaf_->buf_ptr_);
                    return *(ref->ref_.begin() + (n_ - leaf_start_));
                }
                default: assert(!"unhandled rope node case"); break;
                }
                return '\0'; // This should never execute.
            }

            rope const * rope_;
            difference_type n_;
            mutable leaf_node_t const * leaf_;
            mutable difference_type leaf_start_;
        };

        struct const_reverse_rope_iterator
        {
            using value_type = char const;
            using difference_type = std::ptrdiff_t;
            using pointer = char const *;
            using reference = char const;
            using iterator_category = std::random_access_iterator_tag;

            const_reverse_rope_iterator () noexcept : base_ () {}
            explicit const_reverse_rope_iterator (const_rope_iterator it) noexcept : base_ (it) {}

            const_rope_iterator base () const { return base_ + 1; }

            reference operator* () const noexcept { return *base_; }
            value_type operator[] (difference_type n) const noexcept { return base_[n]; }

            const_reverse_rope_iterator & operator++ () noexcept { --base_; return *this; }
            const_reverse_rope_iterator operator++ (int) noexcept
            {
                const_reverse_rope_iterator retval = *this;
                --base_;
                return retval;
            }
            const_reverse_rope_iterator & operator+= (difference_type n) noexcept { base_ -= n; return *this; }

            const_reverse_rope_iterator & operator-- () noexcept { ++base_; return *this; }
            const_reverse_rope_iterator operator-- (int) noexcept
            {
                const_reverse_rope_iterator retval = *this;
                ++base_;
                return retval;
            }
            const_reverse_rope_iterator & operator-= (difference_type n) noexcept { base_ += n; return *this; }

            friend bool operator== (const_reverse_rope_iterator lhs, const_reverse_rope_iterator rhs) noexcept
            { return lhs.base_ == rhs.base_; }
            friend bool operator!= (const_reverse_rope_iterator lhs, const_reverse_rope_iterator rhs) noexcept
            { return !(lhs == rhs); }
            friend bool operator< (const_reverse_rope_iterator lhs, const_reverse_rope_iterator rhs) noexcept
            { return rhs.base_ < lhs.base_; }
            friend bool operator<= (const_reverse_rope_iterator lhs, const_reverse_rope_iterator rhs) noexcept
            { return rhs.base_ <= lhs.base_; }
            friend bool operator> (const_reverse_rope_iterator lhs, const_reverse_rope_iterator rhs) noexcept
            { return rhs.base_ > lhs.base_; }
            friend bool operator>= (const_reverse_rope_iterator lhs, const_reverse_rope_iterator rhs) noexcept
            { return rhs.base_ >= lhs.base_; }

            friend const_reverse_rope_iterator operator+ (const_reverse_rope_iterator lhs, difference_type rhs) noexcept
            { return lhs += rhs; }
            friend const_reverse_rope_iterator operator+ (difference_type lhs, const_reverse_rope_iterator rhs) noexcept
            { return rhs += lhs; }
            friend const_reverse_rope_iterator operator- (const_reverse_rope_iterator lhs, difference_type rhs) noexcept
            { return lhs -= rhs; }
            friend const_reverse_rope_iterator operator- (difference_type lhs, const_reverse_rope_iterator rhs) noexcept
            { return rhs -= lhs; }
            friend difference_type operator- (const_reverse_rope_iterator lhs, const_reverse_rope_iterator rhs) noexcept
            { return lhs.base_ - rhs.base_; }

        private:
            const_rope_iterator base_;
        };

    }

    int rope::compare (rope rhs) const noexcept
    {
        // TODO: This could probably be optimized quite a bit by doing
        // something equivalent to mismatch, segment-wise.
        auto const iters = std::mismatch(begin(), end(), rhs.begin(), rhs.end());
        if (iters.first == end()) {
            if (iters.second == rhs.end())
                return 0;
            else
                return -1;
        } else if (iters.second == rhs.end()) {
            return 1;
        } else if (*iters.first == *iters.second) {
            return 0;
        } else if (*iters.first < *iters.second) {
            return -1;
        } else {
            return 1;
        }
    }

    rope::const_iterator rope::begin () const noexcept
    { return const_iterator(*this, 0); }
    rope::const_iterator rope::end () const noexcept
    { return const_iterator(*this, size()); }

    rope::const_reverse_iterator rope::rbegin () const noexcept
    { return const_reverse_iterator(const_iterator(*this, size() - 1)); }
    rope::const_reverse_iterator rope::rend () const noexcept
    { return const_reverse_iterator(const_iterator(*this, -1)); }

    rope::const_iterator begin (rope const & r) noexcept
    { return r.begin(); }
    rope::const_iterator end (rope const & r) noexcept
    { return r.end(); }

    rope::const_reverse_iterator rbegin (rope const & r) noexcept
    { return r.rbegin(); }
    rope::const_reverse_iterator rend (rope const & r) noexcept
    { return r.rend(); }

} }

#endif
