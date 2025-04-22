#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <windows.h>
#include <vector>
#include <algorithm>
#include <sstream>
#include <limits>
#include <conio.h>
#include <thread>
#include <chrono>
using namespace std;

class Player;

class GameItem {
public:
    string name;
    int position;
    GameItem(string n, int pos) : name(n), position(pos) {}
    virtual void interact(Player& player) = 0;
    virtual ~GameItem() = default;
};

class Coin : public GameItem {
public:
    int value;
    Coin(string n, int pos, int val) : GameItem(n, pos), value(val) {}
    void interact(Player& player) override;
};

class Hurdle : public GameItem {
public:
    int waitTurns;
    string requiredHelper;
    Hurdle(string n, int pos, int wt, string rh) : GameItem(n, pos), waitTurns(wt), requiredHelper(rh) {}
    void interact(Player& player) override;
};

class Helper : public GameItem {
public:
    Helper(string n, int pos) : GameItem(n, pos) {}
    void interact(Player& player) override;
};

class Player {
public:
    string name;
    int position;
    int points, goldCoins, silverCoins;
    int swordUses;
    bool hasShield, hasWater, hasKey;
    int waitTurns;

    Player(string n)
        : name(n), position(0), points(0), goldCoins(10), silverCoins(20), swordUses(2),
        hasShield(false), hasWater(false), hasKey(false), waitTurns(0) {
    }

    void displayStatus(int boardSize);
};

class Board {
public:
    int size;
    GameItem** cells;
    Board(int s);
    ~Board();
    void generateItems();
    void display(const Player& p1, const Player& p2);
};

class AdventureQuest {
public:
    Board* board;
    Player p1, p2;
    bool p1Reached = false, p2Reached = false;

    AdventureQuest(int boardSize);
    ~AdventureQuest();
    void start();
    bool isGameOver();
    int goalPos();
    void playerTurn(Player& player, int a);
    void showResults();
    void saveGame();
    void loadGame();
};

// Interactions
void Coin::interact(Player& player) {
    if (name == "Gold") {
        player.goldCoins++;
        player.points += 10;
        cout << " Picked up a Gold Coin (+10 points)\n";
    }
    else {
        player.silverCoins++;
        player.points += 5;
        cout << " Picked up a Silver Coin (+5 points)\n";
    }
}

void Hurdle::interact(Player& player) {
    cout << " Encountered a " << name;
    if (requiredHelper == "None") {
        cout << ". Wait for " << waitTurns << " turns.\n";
        player.waitTurns = waitTurns;
        return;
    }

    bool usedHelper = false;
    if ((requiredHelper == "Sword" && player.swordUses > 0) ||
        (requiredHelper == "Shield" && player.hasShield) ||
        (requiredHelper == "Water" && player.hasWater) ||
        (requiredHelper == "Key" && player.hasKey)) {
        cout << ". Used " << requiredHelper << "!\n";
        if (requiredHelper == "Sword") player.swordUses--;
        else if (requiredHelper == "Shield") player.hasShield = false;
        else if (requiredHelper == "Water") player.hasWater = false;
        else if (requiredHelper == "Key") player.hasKey = false;
        usedHelper = true;
    }

    if (!usedHelper) {
        if (name == "Snake") {
            cout << ". Bitten by a Snake! Moving back 3 steps.\n";
            player.position = max(0, player.position - 3);
        }
        else {
            cout << ". No " << requiredHelper << ", wait for " << waitTurns << " turns.\n";
            player.waitTurns = waitTurns;
        }
    }
}

void Helper::interact(Player& player) {
    cout << " Found a " << name << ". ";
    if (name == "Sword") player.swordUses++;
    else if (name == "Shield") player.hasShield = true;
    else if (name == "Water") player.hasWater = true;
    else if (name == "Key") player.hasKey = true;
    cout << "Added to inventory.\n";
}

void Player::displayStatus(int boardSize) {
    cout << "\n" << name << " STATUS\n";
    cout << " Position     : " << position << "/" << (boardSize * boardSize - 1) << endl;
    cout << " Points       : " << points << endl;
    cout << " Gold Coins   : " << goldCoins << endl;
    cout << " Silver Coins : " << silverCoins << endl;
    cout << " Sword Uses   : " << swordUses << endl;
    cout << " Shield       : " << (hasShield ? "Yes" : "No") << endl;
    cout << " Water        : " << (hasWater ? "Yes" : "No") << endl;
    cout << " Key          : " << (hasKey ? "Yes" : "No") << endl;
    cout << " Wait Turns   : " << waitTurns << endl;
}

// Board
Board::Board(int s) : size(s) {
    cells = new GameItem * [s * s];
    for (int i = 0; i < s * s; ++i) cells[i] = nullptr;
}

Board::~Board() {
    for (int i = 0; i < size * size; ++i) delete cells[i];
    delete[] cells;
}

void Board::generateItems() {
    int minItems = max(4, size * 2);
    int maxItems = min(size * 4, (size * size) / 2);
    int totalObjects = minItems + rand() % (maxItems - minItems + 1);

    for (int i = 0; i < totalObjects; i++) {
        int pos = rand() % (size * size);
        if (cells[pos] != nullptr) continue;

        int type = rand() % 3;
        if (type == 0)
            cells[pos] = new Coin("Gold", pos, 10);
        else if (type == 1)
            cells[pos] = new Coin("Silver", pos, 5);
        else {
            string hurdleNames[5] = { "Fire", "Snake", "Ghost", "Lion", "Lock" };
            string helpers[5] = { "Water", "Sword", "Shield", "Sword", "Key" };
            int waitTimes[5] = { 2, 3, 1, 4, 5 };
            int index = rand() % 5;
            cells[pos] = new Hurdle(hurdleNames[index], pos, waitTimes[index], helpers[index]);
        }
    }
}

void Board::display(const Player& p1, const Player& p2) {
    Sleep(2000);
    system("cls");
    cout << " BOARD\n";
    for (int i = 0; i < size * size; i += size) {
        for (int j = 0; j < size; j++) cout << "+------";
        cout << "+\n";

        for (int j = 0; j < size; j++) {
            int index = i + j;
            cout << "| ";
            string content = "     ";
            if (p1.position == index) content = "P1";
            else if (p2.position == index) content = "P2";
            else if (cells[index]) content = cells[index]->name.substr(0, 5);
            cout << setw(5) << left << content;
        }
        cout << "|\n";
    }
    for (int j = 0; j < size; j++) cout << "+------";
    cout << "+\n";
}

// Game
AdventureQuest::AdventureQuest(int boardSize) : board(new Board(boardSize)), p1("PLAYER_ONE"), p2("PLAYER_TWO") {
    p1.position = boardSize - 1;
    p2.position = boardSize * (boardSize - 1);
}

AdventureQuest::~AdventureQuest() {
    delete board;
}

void AdventureQuest::start() {
    board->generateItems();
    cout << "WELCOME TO ADVENTURE QUEST!\n";
    while (!isGameOver()) {
        board->display(p1, p2);
        p1.displayStatus(board->size);
        p2.displayStatus(board->size);
        playerTurn(p1, 1);
        if (isGameOver()) break;
        playerTurn(p2, 2);
    }
    showResults();
}

bool AdventureQuest::isGameOver() {
    if (p1.position == goalPos()) p1Reached = true;
    if (p2.position == goalPos()) p2Reached = true;
    return p1Reached && p2Reached;
}

int AdventureQuest::goalPos() {
    return (board->size * board->size) / 2;
}

void AdventureQuest::playerTurn(Player& player, int a) {
    if (player.waitTurns > 0) {
        cout << player.name << " waits for " << player.waitTurns << " turn(s)\n";
        player.waitTurns--;
        return;
    }
    if (player.position == goalPos()) {
        cout << player.name << " has reached the goal and skips turn.\n";
        return;
    }

    cout << "\n" << player.name << " Turn Menu: [1] Move, [2] Buy Helper, [3] Place Hurdle: ";
    int choice;
    cin >> choice;

    if (choice == 2) {
        cout << "Which helper do you want to buy? (Sword/Shield/Water/Key): ";
        string item;
        cin >> item;
        if (player.goldCoins >= 2) {
            if (item == "Sword") player.swordUses++;
            else if (item == "Shield") player.hasShield = true;
            else if (item == "Water") player.hasWater = true;
            else if (item == "Key") player.hasKey = true;
            else {
                cout << "Invalid item.\n";
                return;
            }
            player.goldCoins -= 2;
            cout << item << " bought successfully.\n";
        }
        else {
            cout << "Not enough Gold Coins.\n";
        }
        return;
    }

    if (choice == 3) {
        int targetPos;
        cout << "Enter position to place Snake hurdle: ";
        cin >> targetPos;
        if (targetPos >= 0 && targetPos < board->size * board->size && board->cells[targetPos] == nullptr) {
            board->cells[targetPos] = new Hurdle("Snake", targetPos, 3, "Sword");
            player.goldCoins -= 2;
            player.points -= 5;
            cout << "Hurdle placed.\n";
        }
        else {
            cout << "Invalid position.\n";
        }
        return;
    }

    if (choice != 1) {
        cout << "Invalid choice.\n";
        return;
    }

    int row = player.position / board->size;
    int col = player.position % board->size;
    if (a == 1) {
        if (row % 2 == 0) {
            if (col > 0) col--;
            else if (row + 1 < board->size) { row++; }
        }
        else {
            if (col < board->size - 1) col++;
            else if (row + 1 < board->size) { row++; }
        }
    }
    else {
        if ((board->size - 1 - row) % 2 == 0) {
            if (col < board->size - 1) col++;
            else if (row > 0) row--;
        }
        else {
            if (col > 0) col--;
            else if (row > 0) row--;
        }
    }

    int newPos = row * board->size + col;
    if (newPos == (a == 1 ? p2.position : p1.position)) {
        cout << "Cell occupied by opponent. Wait!\n";
        return;
    }

    player.position = newPos;
    cout << "Moved to " << newPos << endl;
    if (board->cells[newPos]) {
        board->cells[newPos]->interact(player);
        delete board->cells[newPos];
        board->cells[newPos] = nullptr;
    }
}

void AdventureQuest::showResults() {
    cout << "\nFINAL RESULTS\n";
    p1.displayStatus(board->size);
    p2.displayStatus(board->size);

    if (p1.points == p2.points) cout << "It's a Draw!\n";
    else if (p1.points > p2.points) cout << "PLAYER_ONE Wins!\n";
    else cout << "PLAYER_TWO Wins!\n";

    char next;
    cout << "Play next level? (Y/N): ";
    cin >> next;
    if (next == 'Y' || next == 'y') {
        int newSize = board->size + 2;
        if (newSize > 11) {
            cout << "Max board size reached.\n";
            return;
        }
        delete board;
        board = new Board(newSize);
        int p1g = p1.goldCoins + 3;
        int p1s = p1.silverCoins + 5;
        int p2g = p2.goldCoins + 3;
        int p2s = p2.silverCoins + 5;
        p1 = Player("PLAYER_ONE");
        p2 = Player("PLAYER_TWO");
        p1.goldCoins = p1g; p1.silverCoins = p1s;
        p2.goldCoins = p2g; p2.silverCoins = p2s;
        p1.position = newSize - 1;
        p2.position = newSize * (newSize - 1);
        p1Reached = p2Reached = false;
        board->generateItems();
        start();
    }
}

void AdventureQuest::saveGame() {
    ofstream file("savegame.txt");
    file << board->size << endl;
    for (int i = 0; i < board->size * board->size; i++) {
        if (board->cells[i]) {
            file << board->cells[i]->name << " " << board->cells[i]->position << " ";
            if (Coin* coin = dynamic_cast<Coin*>(board->cells[i])) {
                file << "Coin " << coin->value << endl;
            }
            else if (Hurdle* hurdle = dynamic_cast<Hurdle*>(board->cells[i])) {
                file << "Hurdle " << hurdle->waitTurns << " " << hurdle->requiredHelper << endl;
            }
            else if (dynamic_cast<Helper*>(board->cells[i])) {
                file << "Helper" << endl;
            }
        }
        else {
            file << "Empty" << endl;
        }
    }

    auto writePlayer = [&](const Player& p) {
        file << p.name << " " << p.position << " " << p.points << " " << p.goldCoins << " "
            << p.silverCoins << " " << p.swordUses << " " << p.hasShield << " " << p.hasWater
            << " " << p.hasKey << " " << p.waitTurns << endl;
        };

    writePlayer(p1);
    writePlayer(p2);
    file.close();
    cout << "Game saved successfully.\n";
}

void AdventureQuest::loadGame() {
    ifstream file("savegame.txt");
    if (!file) {
        cout << "No saved game found.\n";
        return;
    }
    int size;
    file >> size;
    delete board;
    board = new Board(size);

    for (int i = 0; i < size * size; i++) {
        string type;
        file >> type;
        if (type == "Empty") {
            board->cells[i] = nullptr;
        }
        else {
            string name = type;
            int pos;
            file >> pos >> type;
            if (type == "Coin") {
                int val; file >> val;
                board->cells[i] = new Coin(name, pos, val);
            }
            else if (type == "Hurdle") {
                int wt; string req; file >> wt >> req;
                board->cells[i] = new Hurdle(name, pos, wt, req);
            }
            else if (type == "Helper") {
                board->cells[i] = new Helper(name, pos);
            }
        }
    }

    auto readPlayer = [&](Player& p) {
        file >> p.name >> p.position >> p.points >> p.goldCoins >> p.silverCoins
            >> p.swordUses >> p.hasShield >> p.hasWater >> p.hasKey >> p.waitTurns;
        };

    readPlayer(p1);
    readPlayer(p2);
    file.close();
    cout << "Game loaded successfully.\n";
}

// Entry Point
int main() {
    srand(static_cast<unsigned>(time(0)));
    AdventureQuest game(5);
    game.start();
    return 0;
}
