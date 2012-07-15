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

#include <iostream>
#include <string>
#include <vector>

using std::vector;


typedef enum
{
    ROBOT, WALL, ROCK, LAMBDA, LIFT_CLOSED, LIFT_OPEN, EARTH, SPACE
}
    FieldType;

typedef unsigned char Field;


class Map
{
public:
    explicit Map(std::istream& input)
        : data_(0),
          width_(0),
          height_(0)
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
                case '\\': row.push_back(LAMBDA);      break;
                case 'L':  row.push_back(LIFT_CLOSED); break;
                case '.':  row.push_back(EARTH);       break;
                case ' ':  row.push_back(SPACE);       break;
                default:
                    throw "Illegal input character.";
                }
            }
            width_ = std::max(width_, row.size());
            data_.push_back(vector<Field>(row));
        }
        height_ = data_.size();
    }

    size_t width() const { return width_; }
    size_t height() const { return height_; }

    Field at(size_t x, size_t y) const
    {
        if (y >= 0 and y < height())
        {
            vector<Field> const& row = data_.at(height() - 1 - y);
            if (x >= 0 and x < row.size())
                return row.at(x);
        }
        return SPACE;
    }

private:
    vector<vector<Field> > data_;
    size_t width_;
    size_t height_;
};

std::ostream& operator<<(std::ostream& output, Map const& m)
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
    Map m(std::cin);
    std::cout << m;
}
