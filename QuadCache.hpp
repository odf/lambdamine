/** -*-c++-*-
 *
 *  Copyright 2012  Olaf Delgado-Friedrichs
 *
 *  File: QuadCache.hpp
 *  Project: lambdaminer (potential ICFP 2012 contest entry)
 *  Date: 2012-07-16
 *
 *  A quad tree that caches square contents on every level.
 *
 */

#ifndef LAMBDAMINER_QUADCACHE_HPP
#define LAMBDAMINER_QUADCACHE_HPP 1

#include <iostream>
#include <vector>
#include <unordered_set>

using std::size_t;

template<typename ValueType>
class QuadCache
{
    struct Node;

    union Entry
    {
        ValueType val;
        Node* node;
    };

    struct Node
    {
        size_t extent;
        Entry se, sw, ne, nw;
    };

    struct Hash
    {
        std::size_t operator()(Node* n) const
        {
            size_t h = 0;

            if (n->extent == 2)
            {
                h = h * 101 + n->se.val;
                h = h * 101 + n->sw.val;
                h = h * 101 + n->ne.val;
                h = h * 101 + n->nw.val;
            }
            else
            {
                h = h * 101 + (size_t)n->se.node;
                h = h * 101 + (size_t)n->sw.node;
                h = h * 101 + (size_t)n->ne.node;
                h = h * 101 + (size_t)n->nw.node;
            }
            return h;
        }
    };

    struct Equal
    {
        bool operator()(Node* n, Node* m) const
        {
            if (n->extent != m->extent)
                return false;

            if (n->extent == 2)
                return (n->se.val == m->se.val and
                        n->sw.val == m->sw.val and
                        n->ne.val == m->ne.val and
                        n->nw.val == m->nw.val);
            else
                return (n->se.node == m->se.node and
                        n->sw.node == m->sw.node and
                        n->ne.node == m->ne.node and
                        n->nw.node == m->nw.node);
        }
    };

public:
    class Map
    {
        struct MHash
        {
            std::size_t operator()(Map const m) const
            {
                return Hash()(m.root_);
            }
        };

        struct MEqual
        {
            std::size_t operator()(Map const m, Map const o) const
            {
                return Equal()(m.root_, o.root_);
            }
        };

    public:
        typedef std::unordered_set<Map, MHash, MEqual> Set;
        
        Map()
        {
        }

        ValueType at(size_t const x, size_t const y) const
        {
            return parent_->find(root_, 0, parent_->extent_, x, y);
        }

        Map set(size_t const x, size_t const y, ValueType val) const
        {
            return Map(parent_,
                       parent_->set(root_, 0, parent_->extent_, x, y, val));
        }

    private:
        friend class QuadCache;

        QuadCache* parent_;
        Node* root_;

        Map(QuadCache* const parent, Node* const root)
            : parent_(parent),
              root_(root)
        {
        }
    };

    typedef std::unordered_set<Node*, Hash, Equal> Bucket;

    QuadCache()
    {
    }

    explicit QuadCache(std::vector<std::vector<ValueType> > const& data,
                       ValueType const filler)
    {
        filler_ = filler;
        height_ = data.size();
        width_ = 0;
        for (size_t i = 0; i < height_; ++i)
            width_ = std::max(width_, data.at(i).size());

        depth_ = 1;
        extent_ = 2;
        while (extent_ < height_ or extent_ < width_)
        {
            ++depth_;
            extent_ <<= 1;
        }

        buckets_ = new std::vector<Bucket>(depth_);
        original_ = build(data, 0, extent_, 0, 0);
    }

    Map original()
    {
        return Map(this, original_);
    }

    void info() const
    {
        for (size_t i = 0; i < depth_; ++i)
            std::cerr << buckets_->at(i).size() << " squares at level " << i
                      << std::endl;
    }

private:
    ValueType filler_;
    size_t width_;
    size_t height_;
    size_t depth_;
    size_t extent_;
    std::vector<Bucket>* buckets_;
    Node* original_;

    ValueType get(std::vector<std::vector<ValueType> > const& data,
                  size_t const x, size_t const y) const
    {
        if (y >= height_)
            return filler_;
        else
        {
            std::vector<ValueType> const& row = data.at(y);
            return x >= row.size() ? filler_ : row.at(x);
        }
    }

    Node* canonical(Node* node, size_t const level)
    {
        std::pair<typename Bucket::iterator, bool> result =
            buckets_->at(level).insert(node);

        if (result.second)
            return node;
        else
            return *result.first;
    }

    Node* build(std::vector<std::vector<ValueType> > const& data,
                size_t const level, size_t const extent,
                size_t const x0, size_t const y0)
    {
        Entry se, sw, ne, nw;
        if (extent == 2)
        {
            se.val = get(data, x0  , y0  );
            sw.val = get(data, x0+1, y0  );
            ne.val = get(data, x0  , y0+1);
            nw.val = get(data, x0+1, y0+1);
        }
        else
        {
            size_t const e = extent >> 1;
            se.node = build(data, level+1, e, x0  , y0  );
            sw.node = build(data, level+1, e, x0+e, y0  );
            ne.node = build(data, level+1, e, x0  , y0+e);
            nw.node = build(data, level+1, e, x0+e, y0+e);
        }

        return canonical(new Node({ extent, se, sw, ne, nw }), level);
    }

    ValueType find(Node const* const node,
                   size_t const level, size_t const extent,
                   size_t const x, size_t const y) const
    {
        size_t const e = extent >> 1;
        Entry entry;

        if (y < e)
            entry = (x < e) ? node->se : node->sw;
        else
            entry = (x < e) ? node->ne : node->nw;

        if (extent == 2)
            return entry.val;
        else
            return find(entry.node, level+1, e, x % e, y % e);
    }

    Node* set(Node const* const node,
              size_t const level, size_t const extent,
              size_t const x, size_t const y, ValueType const val)
    {
        size_t const e = extent >> 1;
        Node* result = new Node(*node);
        Entry entry;

        if (y < e)
            entry = (x < e) ? node->se : node->sw;
        else
            entry = (x < e) ? node->ne : node->nw;

        if (extent == 2)
            entry.val = val;
        else
            entry.node = set(entry.node, level+1, e, x % e, y % e, val);

        if (y < e)
        {
            if (x < e)
                result->se = entry;
            else
                result->sw = entry;
        }
        else
        {
            if (x < e)
                result->ne = entry;
            else
                result->nw = entry;
        }

        return canonical(result, level);
    }
};

#endif
