#include "OpeningBook.h"
#include "Board.h"

#include <fstream>
#include <iostream>
#include <strstream>
#include <ctime>

using namespace std;

OpeningBook::OpeningBook(std::string filename)
{ 
    bookContents = new Book;
    _read(filename);
    srand( (unsigned int)time(NULL) );
}

OpeningBook::~OpeningBook()
{
    delete bookContents;
}

void
OpeningBook::_read(std::string filename)
{
    ifstream	bookFile(filename.c_str(), ios::in);

    if (!bookFile)
    {
        cerr << "Can't open " << filename << endl;
        validBook = false;
        return;
    }

    int nline = 0;
    char buffer[256];
    bookFile.getline(buffer,255);
    while (!bookFile.eof())
    {
        nline++;
        string line(buffer);

        int indexOfColon = line.find(':');
        if (indexOfColon == string::npos)
        {
            cerr << "Illegal book entry at line " << nline << endl;
        }
        else
        {
            string position = line.substr(0,indexOfColon);
            string moveTexts = line.substr(indexOfColon+1);
            moveTexts += " ";
            istrstream movesStream(moveTexts.c_str(), moveTexts.size());

            string move;
            vector<u_int16> moves;
            movesStream >> move;
            while (!movesStream.eof())
            {
                u_int16	origin = 0, dest = 0;

                Move m(move);
                origin = (u_int16)m.origin(); dest = (u_int16)m.destination();
                if (origin == 0 && dest == 0);
                else
                {
                    moves.push_back((origin << 8) | dest);
                }
                movesStream >> move;
            }
            bookContents->insert(Book::value_type(position, moves));
        }
        bookFile.getline(buffer,255);
    }

    validBook = true;
}



u_int16 OpeningBook::getMove(Board *board)
{
  u_int16 move = 0;

  Book::iterator it = bookContents->find(board->getPosition());

  if (it != bookContents->end()) move = it->second[rand() % it->second.size()];
  return move;
}
