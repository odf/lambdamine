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

#include "Game.h"

int main(const int argc, char* argv[])
{
    using std::cerr;
    using std::cin;
    using std::cout;
    using std::endl;
    using std::string;

    string const moves = "LRUDWA";

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
                    cout << "Moves:   " << game.moves() << endl;
                    cout << "Lambdas: " << game.lambdas_collected() << endl;
                    cout << "Score:   " << game.score() << endl;
                }
            }
            if (game.won())
                cout << "Game was won" << endl;
            else if (game.lost())
                cout << "Game was lost" << endl;
            else if (game.aborted())
                cout << "Game was aborted" << endl;
            else
                cout << "Game was interrupted" << endl;
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
