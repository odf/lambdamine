/** -*-c++-*-
 *
 *  Copyright 2012  Olaf Delgado-Friedrichs
 *
 *  File: Game.h
 *  Project: lambdaminer (potential ICFP 2012 contest entry)
 *  Date: 2012-07-15
 *
 *  Declaration of the Game class.
 *
 */

#ifndef LAMBDAMINER_GAME_H
#define LAMBDAMINER_GAME_H 1

#include <iostream>
#include <string>
#include <vector>

using std::vector;

typedef enum
{
    ROBOT, WALL, ROCK, LAMBDA, LIFT_CLOSED, LIFT_OPEN, EARTH, SPACE
}
    FieldType;


class Game
{
    typedef enum { ONGOING, WON, LOST, ABORTED } GameState;
    typedef unsigned char Field;
    typedef vector<Field> Row;

public:
    typedef vector<Row> Map;

    explicit Game(std::istream& input);

    size_t width() const { return width_; }
    size_t height() const { return height_; }

    Field at(size_t x, size_t y) const
    {
        if (on_map(x, y))
        {
            Row const& row = map_.at(height() - 1 - y);
            return x < row.size() ? row.at(x) : SPACE;
        }
        else
            return WALL;
    }

    bool ongoing() const { return state_ == ONGOING; }

    bool won() const { return state_ == WON; }

    bool lost() const { return state_ == LOST; }

    bool aborted() const { return state_ == ABORTED; }

    int lambdas_collected() const { return lambdas_collected_; }

    int lambdas_left() const { return lambdas_left_; }

    int moves() const { return moves_; }

    int score() const
    {
        switch (state_)
        {
        case ONGOING:
        case LOST:    return 25 * lambdas_collected() - moves();
        case ABORTED: return 50 * lambdas_collected() - moves();
        case WON:     return 75 * lambdas_collected() - moves();
        default:
            throw "Illegal game state";
        }
    }

    Game step(char const move) const;

    Map map() const { return map_; }

private:
    Map map_;
    size_t width_, height_;
    size_t x_, y_;
    int moves_, lambdas_left_, lambdas_collected_;
    GameState state_;

    bool on_map(size_t const x, size_t const y) const
    {
        return x >= 0 and x < width() and y >= 0 and y < height();
    }

    bool empty(size_t const x, size_t const y) const
    {
        return at(x, y) == SPACE;
    }

    bool is_free(size_t const x, size_t const y) const
    {
        switch (at(x, y))
        {
        case SPACE:
        case EARTH:
        case LAMBDA:
        case LIFT_OPEN:
            return true;
        default:
            return false;
        }
    }

    void set(size_t const x, size_t const y, Field const value)
    {
        if (on_map(x, y))
        {
            Row& row = map_.at(height() - 1 - y);
            while (x >= row.size())
                row.push_back(SPACE);
            row.at(x) = value;
        }
        else
            throw "Illegal coordinates for set() call.";
    }

    void rock_fall(size_t const xo, size_t const yo,
                   size_t const xn, size_t const yn)
    {
        set(xo, yo, SPACE);
        set(xn, yn, ROCK);
        if (at(xn, yn-1) == ROBOT)
            state_ = LOST;
    }

    Game move_robot(char const move) const;
};

std::ostream& operator<<(std::ostream& output, Game const& m);

#endif
