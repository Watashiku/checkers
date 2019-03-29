#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>
#include <chrono>

#include <thread>


#define INFINITY 100000
#define DEFAULT_DEPTH 3
#define KING_VALUE 150
#define MAN_VALUE 100

#define TIME std::chrono::system_clock::time_point
#define NOW chrono::system_clock::now()

#define ALPHABETA
#define PROGRESSIVE_DEEPENING

//#define TESTING_BOARD

using namespace std;

class Game;
class Position;
class Board;
class Move;
class Atom;
class Square;

int minimax(Position currentPosition, int depth);
int alphabeta(Position currentPosition, int depth, int a, int b);
char oppositeColor(char c);

TIME start;


struct moveScoreType {
    string move;
    int score;
};



class Square {

private:
    
    bool init = false;
    int _index;
    
    
public:
    
    // Setters
    void set(int index) {
        init = true;
        _index = index;
        
    }
    
    void setByIndices(int i, int j) {
        init = true;
        set(4*i + j/2);
    }
    
    void setByNotation(char a, int b) {
        init = true;
        setByIndices(b - 1, a - 'A');
    }
    
    void set(string s) {
        init = true;
        setByNotation(s[0], s[1] - 48);
    }
    
    // Getters
    int getIndex() {
        if (!init) cerr << "NOT INIT" << endl;
        return _index;
    }
    
    int* getIndices() {
        if (!init) cerr << "NOT INIT" << endl;
        int* ret = new int[2];
        ret[0] = _index/4;
        ret[1] = 2*(_index%4) + (ret[0])%2;
        
        return ret;
    }
    
    string toString() {
        if (!init) cerr << "NOT INIT" << endl;
        int* indices = getIndices();
        string s = "";
        s += ('A' + indices[1]);
        s += to_string(indices[0] + 1);
        return s;
    }
    
    // Other
    bool isNeighbor(Square s) {
        if (!init) cerr << "NOT INIT" << endl;
        int* a = getIndices();
        int* b = s.getIndices();
        return abs((a[0]-b[0])*(a[1]-b[1])) == 1;
    }
    
    Square getMiddle(Square s) {
        if (!init) cerr << "NOT INIT" << endl;
        // We ASSUME they are neighbors
        Square m;
        int* i0 = getIndices();
        int* i2 = s.getIndices();
        m.setByIndices((i0[0]+i2[0])/2, (i0[1]+i2[1])/2);
        return m;
    }
    
    vector<Square> potentialNeighbors() {
        if (!init) cerr << "NOT INIT" << endl;
        int* indices = getIndices();
        vector<Square> locations(4);
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                int I = indices[0] - 1 + 2*i, J = indices[1] - 1 + 2*j;
                if (I < 8 && I > -1 && J < 8 && J > -1) {
                    locations[2*i + j].setByIndices(I, J);
                } else {
                    locations[2*i + j].set(-1);
                }
            }
        }
        return locations;
    }
    
    vector<vector<Square>> potentialJumps() {
        if (!init) cerr << "NOT INIT" << endl;
        
        int* indices = getIndices();
        vector<Square> end(4);
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                int I = indices[0] - 2 + 4*i, J = indices[1] - 2 + 4*j;
                if (I < 8 && I > -1 && J < 8 && J > -1) {
                    end[2*i + j].setByIndices(I, J);
                } else {
                    end[2*i + j].set(-1);
                }
            }
        }

        vector<Square> middle = potentialNeighbors();
        
        vector<vector<Square>> jumps(4);
        for (int i = 0; i < 4; ++i) {
            jumps[i] = {middle[i], end[i]};
        }
        
        return jumps;
    }
};





class Atom {

private:
    
    Square _fr;
    Square _mi;
    Square _to;
    bool _capture;


public:
    
    Atom(Square fr, Square to) : _fr(fr), _to(to), _capture(false) {
#ifdef TESTING_BOARD
        if(!_fr.isNeighbor(_to)) { 
            cerr << "Not neighbors " << fr.toString() << " " << to.toString() << endl;
            int* a = to.getIndices();
            int* b = fr.getIndices();
            cerr << (a[0]-b[0]) << " " << (a[1]-b[1]) << " " << ((a[0]-b[0])*(a[1]-b[1])) << " " << abs((a[0]-b[0])*(a[1]-b[1])) << " " << (abs((a[0]-b[0])*(a[1]-b[1])) == 1) << endl;
        }
#endif
    }
    
    Atom(Square fr, Square mi, Square to) : _fr(fr), _mi(mi), _to(to), _capture(true) {
#ifdef TESTING_BOARD
        if(_fr.isNeighbor(_to)) { 
            cerr << "Neighbors " << fr.toString() << " " << to.toString() << endl;
        }
#endif
    }
    
    Atom(string s) {
        _fr.set(s.substr(0, 2));
        _to.set(s.substr(2, 2));
        _capture = !_fr.isNeighbor(_to);
        if (_capture) {
            _mi = _fr.getMiddle(_to);
        }
    }
    
    Square getFr() {
        return _fr;
    }
    
    Square getMi() {
        return _mi;
    }
    
    Square getTo() {
        return _to;
    }
    
    bool isCapture() {
        return _capture;
    }
    
};





class Move {

private:
    
    vector<Atom> _atoms;
    
    
public:

    Move(Atom atom) {
        _atoms.push_back(atom);
    }

    Move(string moveString) {
        int iterations = moveString.size()/2 - 1;
        int done = 0;
        while (done < iterations) {
            _atoms.push_back(Atom(moveString.substr(done * 2, 4)));
            ++done;
        }
    }
    
    vector<Atom> getAtoms() {
        return _atoms;
    }
    
    string toString() {
        string s = _atoms[0].getFr().toString();
        for (auto && atom : _atoms) {
            s += atom.getTo().toString();
        }
        return s;
    }
    
    void addStart(Atom atom) {
        _atoms.insert(_atoms.begin(), atom);
    }
    
    int chainSize() {
        return _atoms.size();
    }
};





class Board {
    
private:

    static const int tabSize = 4 * 8;
    char _board[tabSize];
    //int _evaluation;
    
    int _rMan;
    int _rKing;
    int _bMan;
    int _bKing;
    
    
public:
    
    Board() {
        for (int i = 0; i < tabSize; ++i) {
            _board[i] = '.';
        }
        //_evaluation = 0;
        _rMan = 0; _bMan = 0; _rKing = 0; _bKing = 0;
    }
    
    Board(Board& b) {
        for (int i = 0; i < tabSize; ++i) {
            _board[i] = b._board[i];
        }
        //_evaluation = b._evaluation;
        _rMan = b._rMan; _rKing = b._rKing; _bMan = b._bMan; _bKing = b._bKing;
    }
    
    char get(int i, int j) {
        if (i < 0 || j < 0 || i > 9 || j > 7) {
            return '#';
        }
        return _board[4*i + j/2];
    }
    
    char get(int index) {
        if (index < 0 || index >= tabSize) {
            return '#';
        }
        return _board[index];
    }
    
    char get(Square s) {
        return get(s.getIndex());
    }
    
    void set(int i, int j, char value) {
#ifdef TESTING_BOARD
        if ((i+j) % 2 != 0 || i < 0 || j < 0 || i > 9 || j > 7) {
            cerr << "mistake on the indexes : " << i << " " << j << endl;
        }
#endif
        _board[4*i + j/2] = value;
    }
    
    void play(Move move) {
        for (auto && atom : move.getAtoms()) {
            play(atom);
        }
    }
    
    void play(Atom atom) {
        
        char piece = get(atom.getFr());
        
        // detect crowning
        int* toIndices = atom.getTo().getIndices();
        if (piece == 'r' && toIndices[0] == 0) {
            piece = 'R';
            //_evaluation += KING_VALUE - 1;
            _rMan--; _rKing++;
        }
        
        else if (piece == 'b' && toIndices[0] == 7) {
            piece = 'B';
            //_evaluation -= KING_VALUE - 1;
            _bMan--; _bKing++;
        }
        
        
        _board[atom.getFr().getIndex()] = '.';
        _board[atom.getTo().getIndex()] = piece;
        
        
        // detect capture
        if (atom.isCapture()) {
            switch (get(atom.getMi())) {
                case 'r': 
                //--_evaluation;
                _rMan--;
                break;
                
                case 'b':
                //++_evaluation;
                _bMan--;
                break;
                
                case 'R':
                //_evaluation -= KING_VALUE;
                _rKing--;
                break;
                
                case 'B':
                //_evaluation += KING_VALUE;
                _bKing--;
                break;
                
                default:
                cerr << "No piece found : " << get(atom.getMi()) << " " << atom.getFr().toString() << atom.getMi().toString() << atom.getTo().toString() << endl;
            }
            
            _board[atom.getMi().getIndex()] = '.';
        }
    }
    
    int redEvaluation() {
        
        int materialLeft = _rMan+_bMan+_rKing+_bKing;
        float value = MAN_VALUE*(_rMan-_bMan) + KING_VALUE*(_rKing - _bKing);
        
        if (materialLeft < 15) {
            vector<Square> rKings = selectPieces('R');
            for (auto && king : rKings) {
                int* indices = king.getIndices();
                value -= max(abs(2*indices[0]-7), abs(2*indices[1]-7));
            }
            vector<Square> bKings = selectPieces('B');
            for (auto && king : bKings) {
                int* indices = king.getIndices();
                value += max(abs(2*indices[0]-7), abs(2*indices[1]-7));
            }
            
        }
        
        value /= materialLeft;
        return (int) value ;
        
    }
    
    void setEvaluation() {
        //_evaluation = 0;
        _rMan = 0; _bMan = 0; _rKing = 0; _bKing = 0;
        for (int i = 0; i < tabSize; ++i) {
            switch (_board[i]) {
                case '.':
                break;
                
                case 'r': 
                _rMan++;
                //++_evaluation;
                break;
                
                case 'b':
                _bMan++;
                //--_evaluation;
                break;
                
                case 'R':
                _rKing++;
                //_evaluation += KING_VALUE;
                break;
                
                case 'B':
                _bKing++;
                //_evaluation -= KING_VALUE;
                break;
                
                default:
                cerr << "Piece not recognized : " << _board[i] << endl;
            }
        }
    }
    
    vector<Square> selectPieces(char color) {
        vector<Square> pieceLocations;
        for (int i = 0; i < tabSize; ++i) {
            if (tolower(_board[i]) == color) {
                Square s;
                s.set(i);
                pieceLocations.push_back(s);
            }
        }
        return pieceLocations;
    }
};




class Position {

private:

    
    
public:
    char _turn;
    Board _board;

    Position(Board b, char t) : _board(b), _turn(t) {}

    void play(Move move) {
        _board.play(move);
        _turn = oppositeColor(_turn);
    }

    void play(Atom atom) {
        _board.play(atom);
    }
    
    int evaluate() {
        int score = _board.redEvaluation();
        if (_turn == 'b') {
            score *= -1;
        }
        return score;
    }
    
    vector<Move> generateMoves() {
        
        vector<Move> moves;
        bool canCapture = false;
        vector<Square> myPieces = _board.selectPieces(_turn);
        
        for (auto && piece : myPieces) {
            
            bool mem = canCapture;
            
            vector<Move> pieceMoves = squareMoves(piece, canCapture);
            
            if (canCapture ^ mem) {
                // First time we find captures. Previous moves are not valid
                moves = pieceMoves;
            } else {
                moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
            }
        }
        
        return moves;
    }
    
    
    
    vector<Move> squareMoves(Square square, bool mustCapture) {
        
        vector<Move> moves;
        vector<Atom> captures = findCaptures(square);
        
        if (captures.size() != 0) {
            mustCapture = true;
            for (auto && capture : captures) {
                Position p(*this);
                p.play(capture);
                vector<Move> chainMoves = p.squareMoves(capture.getTo(), mustCapture);
                if (chainMoves.size() == 0) {
                    moves.push_back(Move(capture));
                } else {
                    for (auto && chain : chainMoves) {
                        chain.addStart(capture);
                        moves.push_back(chain);
                    }
                }
            }
        }
        
        if (moves.empty() && !mustCapture) {
            char value = _board.get(square);
            vector<Square> potential = square.potentialNeighbors();
            for (auto && location : potential) {
                if (value == 'r' && location.getIndex() > square.getIndex()) continue;
                if (value == 'b' && location.getIndex() < square.getIndex()) continue;
                if (_board.get(location) != '.') continue;
                moves.push_back(Move(Atom(square, location)));
            }
        }
        
        return moves;
    }
    
    vector<Atom> findCaptures(Square s) {
        vector<Atom> captures;
        vector<vector<Square>> jumps = s.potentialJumps();
        
        char ennemy = oppositeColor(_turn);
        char value = _board.get(s);
        
        for (auto && jump : jumps) {
            
            char v1 = _board.get(jump[0]), v2 = _board.get(jump[1]);
            if (v2 != '.' || tolower(v1) != ennemy) continue;
            
            if (value == 'r' && jump[1].getIndex() > s.getIndex()) continue;
            if (value == 'b' && jump[1].getIndex() < s.getIndex()) continue;
            
            captures.push_back(Atom(s, jump[0], jump[1]));
            
        }
        return captures;
    }
};





class Game {
    
private:

    minstd_rand0 rng;
    char _turn;
    Board _board;
    
    
public:
    
    Game() {
        // retrieve color
        string myColor;
        
        getline(cin, myColor);
        if (myColor == "b") _turn = 'b';
        else if (myColor == "r") _turn = 'r';
        else if (myColor == "w") { cerr << "w as a color color -> r" << endl; _turn = 'r'; }
        else { cerr << "Wrong input color : " << myColor << endl; throw "Wrong input color"; }
        
        cerr << "My color is : " << _turn << endl;
            
        
        // init randomness
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        rng.seed(seed);
        
        minstd_rand0 rng = default_random_engine {};
    }
    
    
    void testBoard() {
        
        
        _board.set(0, 0, 'b');
        _board.set(0, 2, 'b');
        _board.set(1, 3, 'b');
        _board.set(2, 0, 'b');
        _board.set(2, 2, 'b');
        _board.set(2, 4, 'b');
        _board.set(3, 5, 'b');
        
        _board.set(4, 0, 'r');
        _board.set(4, 2, 'r');
        _board.set(5, 1, 'r');
        _board.set(5, 3, 'r');
        _board.set(5, 7, 'r');
        _board.set(6, 6, 'r');
        _board.set(7, 3, 'r');
        
        _board.setEvaluation();
        start = NOW;
        Position pos(_board, 'b');
        vector<Move> m = pos.generateMoves();
        /*/
        cerr << "Playing " << m[3].toString() << endl;
        _board.play(m[3]);
        pos = Position(_board, 'r');
        m = pos.generateMoves();
        /*/
        vector<string> s;
        for (auto && mo : m) {
            s.push_back(mo.toString());
            pos.play(mo);
            //cerr << mo.toString() << "    \t-->\t" << pos.evaluate() << endl;
            pos = Position(_board, 'b');
        }
        moveScoreType moveScore = progressiveDeepening(s);
        //bestAtDepth(s, 2);
    }
    
    
    void updateBoard() {
        for (int i = 7; i > -1; --i) {
            string inputLine; // board line
            getline(cin, inputLine);
            for (int j = 0; j < 8; ++j) {
                if ((i+j)%2 == 0) {
                    _board.set(i, j, inputLine[j]);
                }
            }
        }
        _board.setEvaluation();
    }


    void printMove() {
        
        int legalMoves; // number of legal moves
        cin >> legalMoves; cin.ignore();
        
        vector<string> movesVect(legalMoves);
        
        for (int i = 0; i < legalMoves; i++) {
            cin >> movesVect[i]; cin.ignore();
        }
        
        
#ifdef PROGRESSIVE_DEEPENING   
        moveScoreType moveScore = progressiveDeepening(movesVect);
#else    
        moveScoreType moveScore = bestAtDepth(movesVect, DEFAULT_DEPTH);
#endif

        
        cout << moveScore.move << endl;
    }
    
    
    moveScoreType progressiveDeepening(vector<string> moves) {
        
        int depth = 0;
        moveScoreType bestMoveScore = {moves[0], -INFINITY};
        
        try {
            while(1) {
                bestMoveScore = bestAtDepth(moves, ++depth);
                int timeUsed = std::chrono::duration_cast<std::chrono::milliseconds>(NOW - start).count();
                cerr << bestMoveScore.move << " is best at depth " << depth << " with score " << bestMoveScore.score << " reached in " << timeUsed << endl;
                if (abs(bestMoveScore.score) == INFINITY) throw "Gameover";
            }
        } catch (...) {
            int timeUsed = std::chrono::duration_cast<std::chrono::milliseconds>(NOW - start).count();
            cerr << bestMoveScore.move << " chosen before depth " << depth << " at " << timeUsed << endl;
            return bestMoveScore;
        }
    }
    
    
    
    
    moveScoreType bestAtDepth(vector<string> movesVect, int depth) {
        
        int bestScore = -INFINITY;
        vector<string> bestMoveVect;
        
        for(auto && moveString : movesVect) {
            Move move(moveString);
            
            Position pos(_board, _turn);
            pos.play(move);
            


#ifdef ALPHABETA

        //a = max(a, -alphabeta(p, depth - 1, -b, -a));
            int score = -alphabeta(pos, depth - 1, -INFINITY, INFINITY);
#else
            int score = -minimax(pos, depth - 1);
#endif
        
#ifdef TESTING_BOARD
            cerr << moveString << " --> " << score << " " << bestScore << endl;
#endif

            if (score == bestScore) {
                bestMoveVect.push_back(move.toString());
            }
            
            else if (score > bestScore) {
                bestScore = score;
                bestMoveVect = {move.toString()};
            }
        }

#ifdef TESTING_BOARD
        cerr << "Selected moves : " << endl;
        for (auto && el : bestMoveVect) {
            cerr << el << endl;
        }
#endif

        shuffle(begin(bestMoveVect), end(bestMoveVect), rng);
        return {bestMoveVect[0], bestScore};
    }
    
};






int minimax(Position currentPosition, int depth) {
    
#ifdef PROGRESSIVE_DEEPENING
        int timeUsed = std::chrono::duration_cast<std::chrono::milliseconds>(NOW - start).count();
        if (timeUsed > 99) {
            throw "Timeout";
        }
#endif
    
    vector<Move> moves = currentPosition.generateMoves();
    
    if (moves.empty()) {
        cerr << "GAME OVER" << endl;
        return -INFINITY;
    }
    
    if (depth <= 0) {
        return currentPosition.evaluate();
    }
    
    int score = -INFINITY;
    for (auto && move : moves) {
        Position p(currentPosition);
        p.play(move);
        score = max(score, -minimax(p, depth - 1));
    }
    
    return score;
}



int alphabeta(Position currentPosition, int depth, int a, int b) {
    
#ifdef PROGRESSIVE_DEEPENING
        int timeUsed = std::chrono::duration_cast<std::chrono::milliseconds>(NOW - start).count();
        if (timeUsed > 99) {
            throw "Timeout";
        }
#endif
    
    vector<Move> moves = currentPosition.generateMoves();
    
    if (moves.empty()) {
        return -INFINITY-depth;
    }
    
    if (depth <= 0) {
        return currentPosition.evaluate();
    }
    
    for (auto && move : moves) {
        Position p(currentPosition);
        p.play(move);
        a = max(a, -alphabeta(p, depth - 1, -b, -a));
        if (a >= b) {
            //cerr << "Pruning at depth " << depth << endl;
            break;
        }
    }
    return a;
}


char oppositeColor(char c) {
    
    if (tolower(c) == 'r') {
        return 'b';
    }
    
    if (tolower(c) == 'b') {
        return 'r';
    }
    
    cerr << "Error in the input of oppositeColor (should be 'r', 'R', 'b' or 'B') : " << c << endl;
    return 'r';
}





int main()
{
    
    Game game;

#ifdef TESTING_BOARD
    
    game.testBoard();

#else

    unsigned int nthreads = std::thread::hardware_concurrency();
    
    cerr << nthreads << " threads possible" << endl;
    
    
    while(1) {
        game.updateBoard();
        start = NOW;
        game.printMove();
    }
    
#endif
}
