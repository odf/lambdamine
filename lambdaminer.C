/** -*-c++-*-
 *
 *  Copyright 2012  Olaf Delgado-Friedrichs
 *
 *  File: lambdaminer.C
 *  Project: lambdaminer (potential ICFP 2012 contest entry)
 *  Date: 2012-07-16
 *
 *  Reads a map and emits a sequence of robot moves.
 *
 */

#include <fstream>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <tr1/memory>

#include "Game.h"

struct Node;

typedef std::tr1::shared_ptr<Node> NodePtr;

struct Node
{
    Game game;
    char move;
    NodePtr previous;
};

std::string sequence_of_moves(NodePtr node)
{
    std::stack<char> moves;

    NodePtr current = node;
    while (current->previous != 0)
    {
        moves.push(current->move);
        current = current->previous;
    }

    std::string result;
    while (not moves.empty())
    {
        result.push_back(moves.top());
        moves.pop();
    }
    if (node->game.ongoing())
        result.push_back('A');

    return result;
}

int main(const int argc, char* argv[])
{
    using std::cerr;
    using std::cin;
    using std::cout;
    using std::endl;
    using std::queue;
    using std::set;
    using std::string;

    string const moves = "LDRUW";

    if (argc > 1)
    {
        std::ifstream fp(argv[1]);

        if (fp.is_open())
        {
            Game const start(fp);
            fp.close();

            NodePtr best;
            queue<NodePtr> q;
            Game::Map::Set seen;

            q.push(NodePtr(new Node({ start, 0, NodePtr() })));
            seen.insert(start.map());

            while (not q.empty())
            {
                NodePtr node = q.front();
                q.pop();
                Game const game = node->game;

                if (best == 0 or game.score() > best->game.score())
                {
                    if (game.score() > 0)
                    {
                        cerr << "Best score so far: " << game.score() << endl
                             << game;
                        game.cache_info();
                        cerr << endl;
                    }
                    best = node;
                }

                if (game.ongoing())
                {
                    for (size_t i = 0; i < moves.size(); ++i)
                    {
                        char const c = moves.at(i);
                        Game const next = game.step(c);
                        Game::Map const m = next.map();

                        if (seen.count(m) == 0)
                        {
                            seen.insert(m);
                            q.push(NodePtr(new Node({ next, c, node })));
                        }
                    }
                }
                else if (game.won())
                    break;
            }
            cout << sequence_of_moves(best) << endl;
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
