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
        Entry se, sw, ne, nw;
    };

    struct Hash
    {
        std::size_t operator()(Node const& n) const
        {
            size_t h = 0;
            h = h * 101 + (*(size_t*)&n.se);
            h = h * 101 + (*(size_t*)&n.sw);
            h = h * 101 + (*(size_t*)&n.ne);
            h = h * 101 + (*(size_t*)&n.nw);
            return h;
        }
    };

    typedef std::unordered_set<Node, Hash> Bucket;

public:
    explicit QuadCache(std::vector<std::vector<ValueType> > const& data,
                       ValueType const filler)
    {
        filler_ = filler;
        height_ = data.size();
        width_ = 0;
        for (size_t i = 0; i < height_; ++i)
            width_ = std::max(width_, data.at(i).size());

        depth_ = 1;
        extend_ = 2;
        while (extend_ < height_ or extend_ < width_)
        {
            ++depth_;
            extend_ <<= 1;
        }

        buckets_ = std::vector<Bucket>(depth_);
        original_ = build(data, 0, extend_, 0, 0);
    }

    ValueType at(size_t const x, size_t const y) const
    {
        return find(original_, 0, extend_, x, y);
    }

private:
    ValueType filler_;
    size_t width_;
    size_t height_;
    size_t depth_;
    size_t extend_;
    std::vector<Bucket> buckets_;
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

    Node* build(std::vector<std::vector<ValueType> > const& data,
                size_t const level, size_t const extend,
                size_t const x0, size_t const y0)
    {
        Entry se, sw, ne, nw;
        if (extend == 2)
        {
            se.val = get(data, x0  , y0  );
            sw.val = get(data, x0+1, y0  );
            ne.val = get(data, x0  , y0+1);
            nw.val = get(data, x0+1, y0+1);
        }
        else
        {
            size_t const e = extend >> 1;
            se.node = build(data, level+1, e, x0  , y0  );
            sw.node = build(data, level+1, e, x0+e, y0  );
            ne.node = build(data, level+1, e, x0  , y0+e);
            nw.node = build(data, level+1, e, x0+e, y0+e);
        }

        return new Node({ se, sw, ne, nw });
    }

    ValueType find(Node const* const node,
                   size_t const level, size_t const extend,
                   size_t const x, size_t const y) const
    {
        size_t const e = extend >> 1;
        Entry entry;

        if (y < e)
            entry = (x < e) ? node->se : node->sw;
        else
            entry = (x < e) ? node->ne : node->nw;

        if (extend == 2)
            return entry.val;
        else
            return find(entry.node, level+1, e, x % e, y % e);
    }
};

#endif
