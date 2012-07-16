/** -*-c++-*-
 *
 *  Copyright 2012  Olaf Delgado-Friedrichs
 *
 *  File: Game.C
 *  Project: lambdaminer (potential ICFP 2012 contest entry)
 *  Date: 2012-07-16
 *
 *  Implementation of the Game class.
 *
 */

#include <stack>

#include "QuadCache.hpp"
#include "Game.h"

Game::Game(std::istream& input)
    : map_(0),
      width_(0),
      height_(0),
      x_(0),
      y_(0),
      moves_(0),
      lambdas_left_(0),
      lambdas_collected_(0),
      state_(ONGOING)
{
    std::string line;
    std::vector<Field> row;
    std::stack<Row> data;

    while (not input.eof())
    {
        std::getline(input, line);
        if (line.size() == 0)
            break;

        row.clear();

        for (size_t i = 0; i < line.size(); ++i)
        {
            switch (line.at(i))
            {
            case 'R':  row.push_back(ROBOT);       break;
            case '#':  row.push_back(WALL);        break;
            case '*':  row.push_back(ROCK);        break;
            case '\\':
                row.push_back(LAMBDA);
                ++lambdas_left_;
                break;
            case 'L':  row.push_back(LIFT_CLOSED); break;
            case '.':  row.push_back(EARTH);       break;
            case ' ':  row.push_back(SPACE);       break;
            default:
                throw "Illegal input character.";
            }
        }
        width_ = std::max(width_, row.size());
        data.push(row);
    }

    while (not data.empty())
    {
        map_.push_back(data.top());
        data.pop();
    }

    height_ = map_.size();

    for (size_t y = 0; y < height(); ++y)
        for (size_t x = 0; x < width(); ++x)
            if (at(x, y) == ROBOT)
                x_ = x, y_ = y;

    QuadCache<Field> qc(map_, SPACE);
    qc.info();
}

Game Game::step(char const move) const
{
    if (not ongoing())
        return *this;

    Game tmp = move_robot(move);
    Game next(tmp);

    if (not aborted())
    {
        for (size_t y = 0; y < height(); ++y)
        {
            for (size_t x = 0; x < width(); ++x)
            {
                if (tmp.at(x, y) == ROCK)
                {
                    if (tmp.empty(x, y-1))
                        next.rock_fall(x, y, x, y-1);
                    else if (tmp.at(x, y-1) == ROCK)
                    {
                        if (tmp.empty(x+1, y) and tmp.empty(x+1, y-1))
                            next.rock_fall(x, y, x+1, y-1);
                        else if (tmp.empty(x-1, y) and tmp.empty(x-1, y-1))
                            next.rock_fall(x, y, x-1, y-1);
                    }
                    else if (tmp.at(x, y-1) == LAMBDA)
                        if (tmp.empty(x+1, y) and tmp.empty(x+1, y-1))
                            next.rock_fall(x, y, x+1, y-1);
                }
                else if (tmp.at(x, y) == LIFT_CLOSED and 
                         tmp.lambdas_left_ == 0)
                    next.set(x, y, LIFT_OPEN);
            }
        }
    }

    return next;
}

Game Game::move_robot(char const move) const
{
    Game next(*this);

    size_t xo = x_, yo = y_, xn = x_, yn = y_;
    switch (move)
    {
    case 'L': --xn; break;
    case 'R': ++xn; break;
    case 'U': ++yn; break;
    case 'D': --yn; break;
    case 'W':
    case 'A': break;
    default:
        throw "Illegal robot move.";
    }
        
    if (move == 'A')
    {
        next.state_ = ABORTED;
        return next;
    }

    ++next.moves_;

    bool good;

    if (is_free(xn, yn))
        good = true;
    else if (at(xn, yn) == ROCK and yn == yo)
    {
        if (xn == xo + 1 and at(xn + 1, yn) == SPACE)
        {
            good = true;
            next.set(xn + 1, yn, ROCK);
        }
        else if (xn == xo - 1 and at(xn - 1, yn) == SPACE)
        {
            good = true;
            next.set(xn - 1, yn, ROCK);
        }
        else
            good = false;
    }
    else
        good = false;

    if (good)
    {
        if (at(xn, yn) == LAMBDA)
        {
            ++next.lambdas_collected_;
            --next.lambdas_left_;
        } else if (at(xn, yn) == LIFT_OPEN)
            next.state_ = WON;

        next.set(xn, yn, ROBOT);
        next.set(xo, yo, SPACE);
        next.x_ = xn;
        next.y_ = yn;
    }

    return next;
}


std::ostream& operator<<(std::ostream& output, Game const& m)
{
    for (size_t y = 0; y < m.height(); ++y)
    {
        for (size_t x = 0; x < m.width(); ++x)
        {
            char c;
            switch (m.at(x, m.height() - 1 - y))
            {
            case ROBOT:       c = 'R';  break;
            case WALL:        c = '#';  break;
            case ROCK:        c = '*';  break;
            case LAMBDA:      c = '\\'; break;
            case LIFT_CLOSED: c = 'L';  break;
            case LIFT_OPEN:   c = 'O';  break;
            case EARTH:       c = '.';  break;
            case SPACE:       c = ' ';  break;
            default:
                throw "Illegal map entry.";
            }
            output << c;
        }
        output << std::endl;
    }

    return output;
}
