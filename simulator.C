/** -*-c++-*-
 *
 *  Copyright 2012  Olaf Delgado-Friedrichs
 *
 *  File: simulator.C
 *  Project: lambdaminer (potential ICFP 2012 contest entry)
 *  Date: 2012-07-15
 *
 *  Reads a map and a sequence of robot moves, runs the sequence on the map
 *  and prints the resulting score.
 *
 */

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using std::vector;


typedef enum
{
    ROBOT, WALL, ROCK, LAMBDA, LIFT_CLOSED, LIFT_OPEN, EARTH, SPACE
}
    FieldType;

typedef enum
{
    ONGOING, WON, LOST
}
    GameState;

typedef unsigned char Field;


class Game
{
public:
    explicit Game(std::istream& input)
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
        vector<Field> row;

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
            map_.push_back(vector<Field>(row));
        }
        height_ = map_.size();

        for (size_t y = 0; y < height(); ++y)
            for (size_t x = 0; x < width(); ++x)
                if (at(x, y) == ROBOT)
                    x_ = x, y_ = y;
    }

    size_t width() const { return width_; }
    size_t height() const { return height_; }

    Field at(size_t x, size_t y) const
    {
        if (on_map(x, y))
        {
            vector<Field> const& row = map_.at(height() - 1 - y);
            return x < row.size() ? row.at(x) : SPACE;
        }
        else
            return WALL;
    }

    bool ongoing() const
    {
        return state_ == ONGOING;
    }

    bool won() const
    {
        return state_ == WON;
    }

    bool lost() const
    {
        return state_ == LOST;
    }

    Game step(char const move) const
    {
        Game tmp = move_robot(move);
        Game next(tmp);

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
                else if (tmp.at(x, y) == LIFT_CLOSED and tmp.lambdas_left_ == 0)
                    next.set(x, y, LIFT_OPEN);
            }
        }

        return next;
    }

private:
    vector<vector<Field> > map_;
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
            vector<Field>& row = map_.at(height() - 1 - y);
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

    Game move_robot(char const move) const
    {
        Game next(*this);

        size_t xo = x_, yo = y_, xn = x_, yn = y_;
        switch (move)
        {
        case 'L': --xn; break;
        case 'R': ++xn; break;
        case 'U': ++yn; break;
        case 'D': --yn; break;
        case 'W': break;
        default:
            throw "Illegal robot move.";
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
};

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


int main(const int argc, char* argv[])
{
    using std::cerr;
    using std::cin;
    using std::cout;
    using std::endl;
    using std::string;

    string const moves = "LRUDW";

    if (argc > 1)
    {
        std::ifstream fp(argv[1]);

        if (fp.is_open())
        {
            Game game(fp);
            fp.close();
            
            cout << game << endl;

            while (not cin.eof() and game.ongoing())
            {
                char c = toupper(cin.get());
                if (moves.find(c) < moves.length())
                {
                    try
                    {
                        game = game.step(c);
                    }
                    catch (char const* s)
                    {
                        cerr << "An error occurred: " << s << endl;
                    }
                    cout << game << endl;
                }
                else if (c == 'A')
                {
                    cout << "Game was aborted" << endl;
                    return 0;
                }
            }
            if (game.won())
                cout << "Game is won" << endl;
            else if (game.lost())
                cout << "Game is lost" << endl;
        }
        else
        {
            cerr << "Unable to open file" << endl;
            return 1;
        }
    }
    else
    {
        cerr << "Expected a file name." << endl;
        return 1;
    }

    return 0;
}
