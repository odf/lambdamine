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
        size_t extend;
        Entry se, sw, ne, nw;
    };

    struct Hash
    {
        std::size_t operator()(Node* n) const
        {
            size_t h = 0;

            if (n->extend == 2)
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
            if (n->extend != m->extend)
                return false;

            if (n->extend == 2)
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

    typedef std::unordered_set<Node*, Hash, Equal> Bucket;

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

    void info() const
    {
        std::cerr << "size = " << width_ << "x" << height_ << std::endl;
        std::cerr << "extend = " << extend_ << std::endl;
        for (size_t i = 0; i < depth_; ++i)
            std::cerr << buckets_.at(i).size() << " squares at level " << i
                      << std::endl;
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

        Node* result = new Node({ extend, se, sw, ne, nw });
        std::pair<typename Bucket::iterator, bool> res =
            buckets_.at(level).insert(result);

        if (res.second)
            return result;
        else
            return *res.first;
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
