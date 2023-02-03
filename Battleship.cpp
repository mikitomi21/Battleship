#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdlib.h> 
#include <math.h>
#include <string.h>

using namespace std;

#define MAX_LENGTH_COMMAND 50
#define MAX_LENGTH_NAME_OF_CLASS_OF_SHIP 10
#define NUMBER_OF_COMMANDS 30 //to zmiany w zależności ile komend będzie
#define BOARD_SIZE_Y 21
#define BOARD_SIZE_X 10
#define PLAYER_A 0
#define PLAYER_B 1
#define NUMBER_OF_PLAYERS 2
#define NUMBER_OF_CLASSES_OF_SHIPS 4
#define CAR 0
#define BAT 1
#define CRU 2
#define DES 3
#define MAX_SHIPS 10
#define MAX_LENGTH_OF_SHIP 6 // 5 + '\0'
#define MAX_SPIES 50
#define EMPTY ' '
#define PRESENT_SHIP '+'
#define DESTOYRED_SHIP 'x'
#define REEF_CHAR '#'
#define PRINT 2
#define SET_FLEET 3
#define NEXT_PLAYER 4
#define PLACE_SHIP 5
#define SHOOT 6
#define BOARD_SIZE 7
#define REEF 8
#define INIT_POSITION 9
#define SHIP 10
#define MOVE 11
#define EXTENDED_SHIPS 12
#define SPY 13
#define STATE 14
#define SAVE 15
#define SRAND 16
#define SET_AI_PLAYER 17

struct Point {
        int y;
        int x;
};

struct Ship {
        char direction;
        Point coordinates[5];
        int lenght;
        bool is_set = false;
        int moves = 0;
        int shoots = 0; // oddane strzały oraz wysłane samoloty szpiegowskie przez CARIERY
        bool engine = true;
        bool cannon = true;
        bool radar = true;

        // -1: part of ship doesn't exist
        // 0: part of ship is destroyed
        // 1: part of ship is not destroyed
        // Liczymy od poczatku do konca statku
        int structure[5] = { -1, -1, -1, -1, -1 };
};

struct Class_of_ship {
        int number_of_ships;
        int number_of_max_moves = 3; //Wszystkie statki oprócz carrier moga poruszac sie 3 razy
        Ship ships[MAX_SHIPS]; //wartosc 1 nie postawiony, wartosc 0 postawiony
};

struct Player {
        Class_of_ship number_of_ships_in_class[NUMBER_OF_CLASSES_OF_SHIPS];
        bool shot_fired = 0;
        int available_position[4]; //  (y1, x1) (left upper),  (y2, x2) (right lower)
        Point spies_send[MAX_SPIES];
        int number_of_spies = 0;
};

void set_available_position(struct Player* player, int size_of_board[2]) {
        player[PLAYER_A].available_position[0] = 0;
        player[PLAYER_A].available_position[1] = 0;
        player[PLAYER_A].available_position[2] = (size_of_board[0] / 2) - 1;
        player[PLAYER_A].available_position[3] = size_of_board[1] - 1;

        player[PLAYER_B].available_position[0] = (size_of_board[0] / 2) + 1;
        player[PLAYER_B].available_position[1] = 0;
        player[PLAYER_B].available_position[2] = size_of_board[0] - 1;
        player[PLAYER_B].available_position[3] = size_of_board[1] - 1;
}

void free_memory(char **board, int size_of_board) {
        for (int i = 0; i < size_of_board; i++){
                free(board[i]);
        }
        free(board);
}

char** set_new_board(int size_of_board[2]) {
        char** board;
        board = (char**)malloc(size_of_board[0] * sizeof(char *));
        for (int i = 0; i < size_of_board[0]; i++)
        {
                board[i] = (char*)malloc(size_of_board[1] * sizeof(char));
                for (int j = 0; j < size_of_board[1]; j++)
                {
                        board[i][j] = EMPTY;
                }
        }
        return board;
}

int check_command(char command[MAX_LENGTH_COMMAND]) {
        char list_of_commands[NUMBER_OF_COMMANDS][MAX_LENGTH_COMMAND] = { "[playerA]", "[playerB]",
                "PRINT", "SET_FLEET", "NEXT_PLAYER", "PLACE_SHIP", "SHOOT", "BOARD_SIZE", "REEF", "INIT_POSITION", "SHIP",
                "MOVE", "EXTENDED_SHIPS", "SPY", "[state]", "SAVE", "SRAND", "SET_AI_PLAYER" };

        for (int i = 0; i < NUMBER_OF_COMMANDS; i++)
        {
                bool correct_command = true;
                int length_of_command = 0;

                for (int letter_number = 0; letter_number < MAX_LENGTH_COMMAND; letter_number++)
                {
                        if (list_of_commands[i][letter_number] != '\0') length_of_command++;
                        else break;
                }

                for (int letter_number = 0; letter_number < length_of_command; letter_number++)
                {
                        if (list_of_commands[i][letter_number] != command[letter_number]) {
                                correct_command = false;
                                break;
                        }
                }

                if (correct_command == true) {
                        return i;
                }
        }
        return 99;
}

int length_of_ship(int class_of_ship) {
        if (class_of_ship == CAR) return 5;
        else if (class_of_ship == BAT) return 4;
        else if (class_of_ship == CRU) return 3;
        else if (class_of_ship == DES) return 2;
        return 0;
}

void config_ships(struct Player* player) {
        for (int number_of_player = 0; number_of_player < NUMBER_OF_PLAYERS; number_of_player++)
        {
                for (int class_of_ship = 0, temp = 1; class_of_ship < NUMBER_OF_CLASSES_OF_SHIPS; class_of_ship++, temp++) //defaultowa ilosc statkow dla klasy
                {
                        player[number_of_player].number_of_ships_in_class[class_of_ship].number_of_ships = temp;
                        for (int i = 0; i < temp; i++)
                        {
                                player[number_of_player].number_of_ships_in_class[class_of_ship].ships[i].is_set = false;
                                player[number_of_player].number_of_ships_in_class[class_of_ship].ships[i].lenght = length_of_ship(class_of_ship);
                        }
                }
                player[number_of_player].number_of_ships_in_class[CAR].number_of_max_moves = 2;
        }
}

void clean_moves_of_ships(struct Player* player, int current_player) {
        for (int i = 0; i < NUMBER_OF_CLASSES_OF_SHIPS; i++)
        {
                for (int j = 0; j < MAX_SHIPS; j++)
                {
                        player[current_player].number_of_ships_in_class[i].ships[j].moves = 0;
                        player[current_player].number_of_ships_in_class[i].ships[j].shoots = 0;
                }
        }
}

void player_move(struct Player* player, int &number_of_command, int &current_player, int excepted_player, bool &move_player, char command[MAX_LENGTH_COMMAND]) {
        if (number_of_command >= 2 && current_player == excepted_player) {
                cout << "INVALID OPERATION \"" << command << " \": THE OTHER PLAYER EXPECTED";
                exit(0);
        }
        else if (!move_player) {
                number_of_command = 1;
                move_player= 1;
                current_player = excepted_player;
                clean_moves_of_ships(player, current_player);
        }
        else {
                number_of_command++;
                move_player = 0;
        }
}

int count_undestroyed_parts_of_ships(struct Player* player, int number_of_player) {
        int amount = 0;

        for (int i = 0; i < NUMBER_OF_CLASSES_OF_SHIPS; i++)
        {
                for (int j = 0; j < player[number_of_player].number_of_ships_in_class[i].number_of_ships; j++)
                {
                        for (int k = 0; k < player[number_of_player].number_of_ships_in_class[i].ships[j].lenght; k++)
                        {
                                if (player[number_of_player].number_of_ships_in_class[i].ships[j].structure[k] == 1) amount++;
                        }
                }
        }
        return amount;
}

void parts_remaining(struct Player* player, char **board) {
        cout << "PARTS REMAINING:: A : " << count_undestroyed_parts_of_ships(player, PLAYER_A);
        cout << " B : " << count_undestroyed_parts_of_ships(player, PLAYER_B) << endl;;
}

void show_board(char **board, int size_of_board[2]) {
        for (int i = 0; i < size_of_board[0]; i++)
        {
                for (int j = 0; j < size_of_board[1]; j++)
                {
                        cout << board[i][j];
                }
                cout << endl;
        }
}

bool extra_part_of_ship(struct Player* player, int y, int x) {
        for (int number_of_player = 0; number_of_player < 2; number_of_player++)
        {
                for (int i = 0; i < NUMBER_OF_CLASSES_OF_SHIPS; i++)
                {
                        for (int j = 0; j < player[number_of_player].number_of_ships_in_class[i].number_of_ships; j++)
                        {
                                int len_of_ship = player[number_of_player].number_of_ships_in_class[i].ships[j].lenght;
                                if (player[number_of_player].number_of_ships_in_class[i].ships[j].coordinates[0].y == y &&
                                        player[number_of_player].number_of_ships_in_class[i].ships[j].coordinates[0].x == x) {
                                        cout << '@';
                                        return true;
                                }
                                else if (player[number_of_player].number_of_ships_in_class[i].ships[j].coordinates[len_of_ship - 1].y == y &&
                                        player[number_of_player].number_of_ships_in_class[i].ships[j].coordinates[len_of_ship - 1].x == x) {
                                        cout << '%';
                                        return true;
                                }
                                else if (player[number_of_player].number_of_ships_in_class[i].ships[j].coordinates[1].y == y &&
                                        player[number_of_player].number_of_ships_in_class[i].ships[j].coordinates[1].x == x) {
                                        cout << '!';
                                        return true;
                                }
                        }
                }
        }
        return false;
}

//do poprawy na pewno
void show_board_extended(struct Player* player, char **board, int size_of_board[2]) { 
        //Zalozenie, że plansza nie moze miec wiekszych rozmiarów niż 100x100
        int y_len = size_of_board[0] % 100 / 10;

        for (int i = 0; i < y_len; i++)
        {
                cout << ' ';
        }
        if (size_of_board[1] > 10) {
                for (int i = 0; i < size_of_board[1]; i++)
                {
                        cout << i / 10;
                }
                cout << endl;
                for (int i = 0; i < y_len; i++)
                {
                        cout << ' ';
                }
        }
        
        for (int i = 0; i < size_of_board[1]; i++)
        {
                cout << i % 10;
        }
        cout << endl;

        for (int i = 0; i < size_of_board[0]; i++)
        {
                if (size_of_board[0] > 10) cout << i / 10 << i % 10;
                else cout << i % 10;

                for (int j = 0; j < size_of_board[1]; j++)
                {
                        if (board[i][j] == DESTOYRED_SHIP) cout << DESTOYRED_SHIP;
                        else if (!extra_part_of_ship(player, i, j)) cout << board[i][j];
                }
                cout << endl;
        }
}

int convert_char_to_number_of_player(char letter) {
        if (letter == 'A') return PLAYER_A;
        else if (letter == 'B') return PLAYER_B;
        return 99; //blad
}

char convert_number_of_player_to_char(int current_player) {
        if (current_player == PLAYER_A) return 'A';
        else if (current_player == PLAYER_B) return 'B';
        return 99; //blad
}

void set_fleet(struct Player* player) {
        char current_player_name;
        cin >> current_player_name;
        int current_player = convert_char_to_number_of_player(current_player_name);

        int number_of_ships;
        for (int ship = 0; ship < NUMBER_OF_CLASSES_OF_SHIPS; ship++)
        {
                cin >> number_of_ships;
                player[current_player].number_of_ships_in_class[ship].number_of_ships = number_of_ships;
                for (int number_of_not_set_fleet = 0; number_of_not_set_fleet < number_of_ships; number_of_not_set_fleet++)
                {
                        player[current_player].number_of_ships_in_class[ship].ships[number_of_not_set_fleet].is_set = false;
                        player[current_player].number_of_ships_in_class[ship].ships[number_of_not_set_fleet].lenght = length_of_ship(ship);
                }
        }
}

int next_player(int current_player) {
        if (current_player == PLAYER_A) return PLAYER_B;
        return PLAYER_A;
}

int check_class_of_ship(char class_of_ship[3]) {
        char classes_of_ship[NUMBER_OF_CLASSES_OF_SHIPS][MAX_LENGTH_NAME_OF_CLASS_OF_SHIP] = { "CAR", "BAT", "CRU", "DES" };

        for (int number_of_class_of_ships = 0; number_of_class_of_ships < MAX_LENGTH_NAME_OF_CLASS_OF_SHIP; number_of_class_of_ships++)
        {
                bool class_finded = true;
                for (int letter = 0; letter < sizeof(class_of_ship) / sizeof(class_of_ship[0]) - 1; letter++)
                {
                        if (classes_of_ship[number_of_class_of_ships][letter] != class_of_ship[letter]) {
                                class_finded = false;
                                break;
                        }
                }
                if (class_finded) {
                        return number_of_class_of_ships;
                }
        }
        return 99; //blad
}

void add_state_of_segments(struct Player* player, int current_player, int class_of_ship_checked, int number_of_ship, int check_command, char segments[MAX_LENGTH_OF_SHIP]) {
        if (check_command == SHIP) {
                for (int part = 0; part < player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].lenght; part++)
                {
                        if (segments[part] == '0') player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].structure[part] = 0;
                        else if (segments[part] == '1') player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].structure[part] = 1;
                }
        }
        else if (check_command == PLACE_SHIP) {
                for (int part = 0; part < player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].lenght; part++)
                {
                        player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].structure[part] = 1;
                }
        }
}

void ship_placed_on_reef(char **board, int x, int y, char dir, int len, char command[MAX_LENGTH_COMMAND], char class_of_ship[3], int number_of_ship) {
        int x_dir = 0, y_dir = 0;
        if (dir == 'N') x_dir = 1;
        else if (dir == 'S') x_dir = -1;
        else if (dir == 'W') y_dir = 1;
        else if (dir == 'E') y_dir = -1;
        for (int i = 0; i < len; i++)
        {
                if (board[y + i * x_dir][x + i * y_dir] == REEF_CHAR) {
                        cout <<"INVALID OPERATION \"" << command << " " << y << " " << x << " " << dir << " " << number_of_ship << " " << class_of_ship <<"\": PLACING SHIP ON REEF";
                        exit(0);
                }
        }
}

void another_ship_too_close(char **board, int current_x, int current_y, int start_y, int start_x,
        int end_y, int end_x, char dir, char command[MAX_LENGTH_COMMAND], char class_of_ship[3], int number_of_ship, int position, int current_player, char segments[MAX_LENGTH_OF_SHIP]) {
        int amount_part_of_ships_around = 0;
        if (current_y - 1 >= start_y && board[current_y - 1][current_x] == PRESENT_SHIP) amount_part_of_ships_around++;
        if (current_y + 1 <= end_y && board[current_y + 1][current_x] == PRESENT_SHIP) amount_part_of_ships_around++;
        if (current_x - 1 >= start_x && board[current_y][current_x - 1] == PRESENT_SHIP) amount_part_of_ships_around++;
        if (current_x + 1 <= end_x && board[current_y][current_x + 1] == PRESENT_SHIP) amount_part_of_ships_around++;

        if (check_command(command) == PLACE_SHIP && (position == 0 && amount_part_of_ships_around > 0) || amount_part_of_ships_around > 1) {
                cout << "INVALID OPERATION \"" << command << " " << current_y << " " << current_x << " " << dir << " " <<
                        number_of_ship << " " << class_of_ship << "\": PLACING SHIP TOO CLOSE TO OTHER SHIP";
                exit(0);
        }
        else if (check_command(command) == SHIP && (position == 0 && amount_part_of_ships_around > 0) || amount_part_of_ships_around > 1) {
                cout << "INVALID OPERATION \"" << command << " " << convert_number_of_player_to_char(current_player) << " " << current_y << " " << current_x << " " <<
                        dir << " " << number_of_ship << " " << class_of_ship << " " << segments <<  "\": PLACING SHIP TOO CLOSE TO OTHER SHIP";
                exit(0);
        }
}

bool add_points(struct Player* player, int current_player, char **board, int x, int y, int start_y, int start_x, int end_y, int end_x, char dir,
        int len, char command[MAX_LENGTH_COMMAND], char class_of_ship[3], int number_of_ship, char segments[MAX_LENGTH_OF_SHIP]) {
        int class_of_ship_checked = check_class_of_ship(class_of_ship);
        add_state_of_segments(player, current_player, class_of_ship_checked, number_of_ship, check_command(command), segments);

        if (dir == 'S' && (y - len + 1) >= start_y) {
                for (int position = 0; position < len; position++)
                {
                        if (player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].structure[position] == 1) board[y - position][x] = PRESENT_SHIP;
                        else if (player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].structure[position] == 0) board[y - position][x] = DESTOYRED_SHIP;
                        another_ship_too_close(board, x, y, start_y, start_x, end_y, end_x, dir, command, class_of_ship, number_of_ship, position, current_player, segments);
                        player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].coordinates[position].y = y - position;
                        player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].coordinates[position].x = x;
                }
                return 1;
        }
        else if (dir == 'N' && (y + len - 1) <= end_y) {
                for (int position = 0; position < len; position++)
                {
                        if (player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].structure[position] == 1) board[y + position][x] = PRESENT_SHIP;
                        else if (player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].structure[position] == 0) board[y + position][x] = DESTOYRED_SHIP;
                        another_ship_too_close(board, x, y, start_y, start_x, end_y, end_x, dir, command, class_of_ship, number_of_ship, position, current_player, segments);
                        player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].coordinates[position].y = y + position;
                        player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].coordinates[position].x = x;
                }
                return 1;
        }
        else if (dir == 'E' && (x - len + 1) >= start_x) {
                for (int position = 0; position < len; position++)
                {
                        if (player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].structure[position] == 1) board[y][x - position] = PRESENT_SHIP;
                        else if (player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].structure[position] == 0) board[y][x - position] = DESTOYRED_SHIP;
                        another_ship_too_close(board, x, y, start_y, start_x, end_y, end_x, dir, command, class_of_ship, number_of_ship, position, current_player, segments);
                        player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].coordinates[position].y = y;
                        player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].coordinates[position].x = x - position;
                }
                return 1;
        }
        else if (dir == 'W' && (x + len - 1) <= end_x) {
                for (int position = 0; position < len; position++)
                {
                        if (player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].structure[position] == 1) board[y][x + position] = PRESENT_SHIP;
                        else if (player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].structure[position] == 0) board[y][x + position] = DESTOYRED_SHIP;
                        another_ship_too_close(board, x, y, start_y, start_x, end_y, end_x, dir, command, class_of_ship, number_of_ship, position, current_player, segments);
                        player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].coordinates[position].y = y;
                        player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].coordinates[position].x = x + position;
                }
                return 1;
        }
        return 0;
}

bool all_ships_set_of_class(struct Player* player, int current_player, int class_of_ship_checked) {
        for (int ship = 0; ship < player[current_player].number_of_ships_in_class[class_of_ship_checked].number_of_ships; ship++)
        {
                if (player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[ship].is_set == 0) return 0;
        }
        return 1;
}

bool all_ships_set(struct Player* player) {
        for (int i = 0; i < NUMBER_OF_PLAYERS; i++)
        {
                for (int j = 0; j < NUMBER_OF_CLASSES_OF_SHIPS; j++)
                {
                        if (!all_ships_set_of_class(player, i, j)) return 0;
                }
        }
        return 1;
}

void place_ship(struct Player* player, int current_player, char **board, char command[MAX_LENGTH_COMMAND]) {
        char char_of_player;
        char segments[MAX_LENGTH_OF_SHIP];

        if (check_command(command) == SHIP) {
                cin >> char_of_player;
                current_player = convert_char_to_number_of_player(char_of_player);
        }

        int y, x, number_of_ship;
        char direction, class_of_ship[4];
        cin >> y; cin >> x;
        cin >> direction; cin >> number_of_ship;
        cin >> class_of_ship;
        class_of_ship[3] = '\0';

        if (check_command(command) == SHIP) {
                cin >> segments;
        }

        int start_y, start_x, end_y, end_x;
        start_y = player[current_player].available_position[0];
        start_x = player[current_player].available_position[1];
        end_y = player[current_player].available_position[2];
        end_x = player[current_player].available_position[3];
        int class_of_ship_checked = check_class_of_ship(class_of_ship);
        int len_of_ship = length_of_ship(class_of_ship_checked);

        if (player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].is_set == true) {
                cout << "INVALID OPERATION \"" << command << " " << y << " " << x << " " << direction << " " << number_of_ship << " " << class_of_ship << "\": SHIP ALREADY PRESENT";
                exit(0);
        }
        if (all_ships_set_of_class(player, current_player, class_of_ship_checked)) {
                cout << "INVALID OPERATION \"" << command << " " << y << " " << x << " " << direction << " " << number_of_ship << " " << class_of_ship << "\": ALL SHIPS OF THE CLASS ALREADY SET";
                exit(0);
        }
        if (y >= start_y && y <= end_y) ship_placed_on_reef(board, x, y, direction, len_of_ship, command, class_of_ship, number_of_ship);
        if ((y >= start_y && y <= end_y) && add_points(player, current_player, board,
                x, y, start_y, start_x, end_y, end_x, direction, len_of_ship, command, class_of_ship, number_of_ship, segments)) {
                player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].is_set = true;
                player[current_player].number_of_ships_in_class[class_of_ship_checked].ships[number_of_ship].direction = direction;
        }
        else {
                cout << "INVALID OPERATION \"" << command << " " << y << " " << x << " " << direction << " " << number_of_ship << " " << class_of_ship << "\": NOT IN STARTING POSITION";
                exit(0);
        }
}

bool change_structure_of_ship(struct Player* player, int current_player, int y, int x) {
        current_player = next_player(current_player);
        for (int i = 0; i < NUMBER_OF_CLASSES_OF_SHIPS; i++)
        {
                for (int j = 0; j < MAX_SHIPS; j++)
                {
                        for (int k = 0; k < player[current_player].number_of_ships_in_class[i].ships[j].lenght; k++)
                        {
                                if (player[current_player].number_of_ships_in_class[i].ships[j].coordinates[k].x == x
                                        && player[current_player].number_of_ships_in_class[i].ships[j].coordinates[k].y == y) {
                                        player[current_player].number_of_ships_in_class[i].ships[j].structure[k] = 0;

                                        if (k == 0) player[current_player].number_of_ships_in_class[i].ships[j].radar = false;
                                        if (k == 1) player[current_player].number_of_ships_in_class[i].ships[j].cannon = false;
                                        if (k == player[current_player].number_of_ships_in_class[i].ships[j].lenght - 1)
                                                player[current_player].number_of_ships_in_class[i].ships[j].engine = false;
                                        return 1;
                                }
                        }
                }
        }
        return 0;
}

void shoot_on_board(struct Player* player, int current_player, char **board, int y, int x) {
        if (board[y][x] == PRESENT_SHIP) {
                board[y][x] = DESTOYRED_SHIP;
        }
        if (change_structure_of_ship(player, current_player, y, x));
        player[current_player].shot_fired = 1;
}

void shoot_on_board_extended(struct Player* player, int current_player, char **board, int y, int x, char class_of_ship[4], int number_of_ship) {
        if (board[y][x] == PRESENT_SHIP) {
                board[y][x] = DESTOYRED_SHIP;
        }
        if (change_structure_of_ship(player, current_player, y, x));
        player[current_player].number_of_ships_in_class[check_class_of_ship(class_of_ship)].ships[number_of_ship].shoots += 1;
}

bool check_victory(struct Player* player, int current_player, char **board, int size_of_board[2]) {
        int min_y, max_y;
        min_y = player[current_player].available_position[0];
        max_y = player[current_player].available_position[2];

        for (int i = min_y; i <= max_y; i++)
        {
                for (int j = 0; j < size_of_board[1]; j++)
                {
                        if (board[i][j] == PRESENT_SHIP) return 0;
                }
        }
        return 1;
}

bool is_cannon_not_destroyed(struct Player* player, int current_player, int y, int x, char class_of_ship[4], int number_of_ship) {
        if (player[current_player].number_of_ships_in_class[check_class_of_ship(class_of_ship)].ships[number_of_ship].cannon == true) return true;
        else {
                cout << "INVALID OPERATION \"SHOOT " << number_of_ship << " " << class_of_ship << " " << y << " " << x << "\": SHIP CANNOT SHOOT";
                exit(0);
        }
}

float lenght_of_shoot(struct Player* player, int current_player, int y, int x, int class_of_ship, int number_of_ship) {
        float y_cannon = player[current_player].number_of_ships_in_class[class_of_ship].ships[number_of_ship].coordinates[0].y;
        float x_cannon = player[current_player].number_of_ships_in_class[class_of_ship].ships[number_of_ship].coordinates[0].x;
        return sqrt(pow((y - y_cannon), 2) + pow((x - x_cannon), 2));
}

bool no_ship_sees_this_position(struct Player* player, int current_player, int y, int x) {
        for (int i = 0; i < NUMBER_OF_CLASSES_OF_SHIPS; i++)
        {
                for (int j = 0; j < MAX_SHIPS; j++)
                {
                        if (player[current_player].number_of_ships_in_class[i].ships[j].is_set) {
                                if (lenght_of_shoot(player, current_player, y, x, i, j) <= player[current_player].number_of_ships_in_class[i].ships[j].lenght) return false;
                        }
                }
        }
        return true;
}

bool is_on_this_position_spy(struct Player* player, int current_player, int size_of_board[2], int y, int x) {
        for (int i = -1; i < 2; i++)
        {
                for (int j = -1; j < 2; j++)
                {
                        if ((y+i) >= 0 && (y + i) < size_of_board[0] && (x + j) >= 0 && (x + j) < size_of_board[1]) {
                                for (int spy = 0; spy < player[current_player].number_of_spies; spy++)
                                {
                                        if (player[current_player].spies_send[spy].y == (y + i) && player[current_player].spies_send[spy].x == (x + j)) return true;
                                }
                        }
                }
        }
        return false;
}

void show_board_with_radars(struct Player* player, int current_player, char **board, int size_of_board[2]) {
        for (int i = 0; i < size_of_board[0]; i++)
        {
                for (int j = 0; j < size_of_board[1]; j++)
                {
                        if(is_on_this_position_spy(player, current_player, size_of_board, i, j)) cout << board[i][j];
                        else if (no_ship_sees_this_position(player, current_player, i, j)) cout << '?';
                        else cout << board[i][j];
                }
                cout << endl;
        }
}

bool shoot_not_in_range(struct Player* player, int current_player, int y, int x, char class_of_ship[4], int number_of_ship) {
        if (int(lenght_of_shoot(player, current_player, y, x, check_class_of_ship(class_of_ship), number_of_ship)) <=
                player->number_of_ships_in_class[check_class_of_ship(class_of_ship)].ships[number_of_ship].lenght) return true;
        else {
                cout << "INVALID OPERATION \"SHOOT " << number_of_ship << " " << class_of_ship << " " << y << " " << x << "\": SHOOTING TOO FAR";
                exit(0);
        }
}

void shoot(struct Player* player, int current_player, char **board, char command[MAX_LENGTH_COMMAND], int size_of_board[2], bool extended) {
        char class_of_ship[4];
        int number_of_ship;
        if (extended) {
                cin >> number_of_ship;
                cin >> class_of_ship;
                class_of_ship[3] = '\0';
        }
        int y, x;
        cin >> y; cin >> x;

        if (extended) {
                is_cannon_not_destroyed(player, current_player, y, x, class_of_ship, number_of_ship);
                if (check_class_of_ship(class_of_ship) != CAR) shoot_not_in_range(player, current_player, y, x, class_of_ship, number_of_ship);

        }
        if (!all_ships_set(player)) {
                cout << "INVALID OPERATION \"" << command << " " << y << " " << x <<  "\": NOT ALL SHIPS PLACED";
                exit(0);
        }
        else if (y >= size_of_board[0] || y < 0 || x >= size_of_board[1] || x < 0) {
                cout << "INVALID OPERATION \"" << command << " " << y << " " << x << "\": FIELD DOES NOT EXIST";
                exit(0);
        }
        else if (extended && player[current_player].number_of_ships_in_class[check_class_of_ship(class_of_ship)].ships[number_of_ship].shoots >=
                player[current_player].number_of_ships_in_class[check_class_of_ship(class_of_ship)].ships[number_of_ship].lenght) {
                cout << "INVALID OPERATION \"" << command << " " << number_of_ship << " " << class_of_ship << " " << y << " " << x << "\": TOO MANY SHOOTS";
                exit(0);
        }
        else if (extended) {
                shoot_on_board_extended(player, current_player, board, y, x, class_of_ship, number_of_ship);
        }
        else if (!extended) {
                shoot_on_board(player, current_player, board, y, x);
        }

        int secound_player = next_player(current_player);
        if (check_victory(player, secound_player, board, size_of_board)) {
                cout << char(65 + current_player) << " won"; //65:A, 66:B
                exit(0);
        }
}

void add_reef(char **board, int size_of_board[2], Point *reef_send, int &number_of_reefs) {
        int y, x;
        cin >> y; cin >> x;
        if (y >= 0 && y < size_of_board[0] && x >= 0 && x < size_of_board[1]) {
                board[y][x] = REEF_CHAR;
                reef_send[number_of_reefs].y = y;
                reef_send[number_of_reefs].x = x;
                number_of_reefs++;
        }
        else {
                cout << "INVALID OPERATION \"REEF " << y << " " << x << "\": REEF IS NOT PLACED ON BOARD";
                exit(0);
        }
}

void init_position(struct Player* player) {
        char name_of_player;
        int y1, x1, y2, x2;

        cin >> name_of_player;
        cin >> y1; cin >> x1; cin >> y2; cin >> x2;
        int number_of_player = convert_char_to_number_of_player(name_of_player);

        player[number_of_player].available_position[0] = y1;
        player[number_of_player].available_position[1] = x1;
        player[number_of_player].available_position[2] = y2;
        player[number_of_player].available_position[3] = x2;
}

void ship_error(int number_of_error, int number_of_ship, char class_of_ship[4], char dir) {
        if (number_of_error >= 100 && number_of_error <= 104) {
                if (number_of_error == 100) {
                        cout << "INVALID OPERATION \"MOVE " << number_of_ship << " " << class_of_ship << " " << dir << "\": SHIP CANNOT MOVE";
                }
                else if (number_of_error == 101) {
                        cout << "INVALID OPERATION \"MOVE " << number_of_ship << " " << class_of_ship << " " << dir << "\": SHIP MOVED ALREADY";
                }
                else if (number_of_error == 102) {
                        cout << "INVALID OPERATION \"MOVE " << number_of_ship << " " << class_of_ship << " " << dir << "\": PLACING SHIP ON REEF";
                }
                else if (number_of_error == 103) {
                        cout << "INVALID OPERATION \"MOVE " << number_of_ship << " " << class_of_ship << " " << dir << "\": SHIP WENT FROM BOARD";
                }
                else if (number_of_error == 104) {
                        cout << "INVALID OPERATION \"MOVE " << number_of_ship << " " << class_of_ship << " " << dir << "\": PLACING SHIP TOO CLOSE TO OTHER SHIP";
                }
                exit(0);
        }
}

void turn_left(struct Player* player, int current_player, int class_of_ship, int number_of_ship) {
        char dir = player[current_player].number_of_ships_in_class[class_of_ship].ships[number_of_ship].direction;
        if (dir == 'N') player[current_player].number_of_ships_in_class[class_of_ship].ships[number_of_ship].direction = 'W';
        else if (dir == 'W') player[current_player].number_of_ships_in_class[class_of_ship].ships[number_of_ship].direction = 'S';
        else if (dir == 'S') player[current_player].number_of_ships_in_class[class_of_ship].ships[number_of_ship].direction = 'E';
        else if (dir == 'E') player[current_player].number_of_ships_in_class[class_of_ship].ships[number_of_ship].direction = 'N';
}

void turn_right(struct Player* player, int current_player, int class_of_ship, int number_of_ship) {
        char dir = player[current_player].number_of_ships_in_class[class_of_ship].ships[number_of_ship].direction;
        if (dir == 'N') player[current_player].number_of_ships_in_class[class_of_ship].ships[number_of_ship].direction = 'E';
        else if (dir == 'E') player[current_player].number_of_ships_in_class[class_of_ship].ships[number_of_ship].direction = 'S';
        else if (dir == 'S') player[current_player].number_of_ships_in_class[class_of_ship].ships[number_of_ship].direction = 'W';
        else if (dir == 'W') player[current_player].number_of_ships_in_class[class_of_ship].ships[number_of_ship].direction = 'N';
}

void correct_move(struct Player* player, int current_player, char **board, int y_current, int x_current, int y_temp, int x_temp,
        char class_of_ship[4], int number_of_ship, char dir, int size_of_board[2]) {
        int checked_class_of_ship = check_class_of_ship(class_of_ship);

        if (y_current < 0 || y_current >= size_of_board[0] || x_current < 0 || x_current >= size_of_board[1]) ship_error(103, number_of_ship, class_of_ship, dir);
        else if (player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].engine == false) ship_error(100, number_of_ship, class_of_ship, dir);
        else if (board[y_current][x_current] == REEF_CHAR) ship_error(102, number_of_ship, class_of_ship, dir);
        else if (player[current_player].number_of_ships_in_class[checked_class_of_ship].number_of_max_moves <=
                player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].moves) ship_error(101, number_of_ship, class_of_ship, dir);
        else if (board[y_current][x_current] == PRESENT_SHIP) ship_error(104, number_of_ship, class_of_ship, dir);
        else {
                board[y_current][x_current] = PRESENT_SHIP;
                board[y_temp][x_temp] = EMPTY;
        }
}

void move_forward(struct Player* player, int current_player, char **board, char class_of_ship[4], int number_of_ship, int part, char dir, int size_of_board[2]) {
        int checked_class_of_ship = check_class_of_ship(class_of_ship);
        int y_move = 0, x_move = 0;
        if (player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].direction == 'N') y_move = 1;
        else if (player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].direction == 'S') y_move = -1;
        else if (player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].direction == 'W') x_move = 1;
        else if (player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].direction == 'E') x_move = -1;
        int y_temp = player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].y;
        int x_temp = player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].x;

        player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].y -= y_move;
        player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].x -= x_move;

        int y_current = player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].y;
        int x_current = player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].x;

        correct_move(player, current_player, board, y_current, x_current, y_temp, x_temp, class_of_ship, number_of_ship, dir, size_of_board);
}

void move_side(struct Player* player, int current_player, char **board, char class_of_ship[4], int number_of_ship, int part, int len, char dir, int size_of_board[2]) {
        int checked_class_of_ship = check_class_of_ship(class_of_ship);
        char ship_dir = player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].direction;
        int y_temp = player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].y;
        int x_temp = player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].x;

        int side = 1;
        if (dir == 'L') side = 1;
        else if (dir == 'R') side = -1;

        if (ship_dir == 'N') {
                player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].y -= (part + 1);
                player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].x += side * (part - len + 1);
        }
        else if (ship_dir == 'W') {
                player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].y += side * (len - 1 - part);
                player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].x -= (part + 1);
        }
        else if (ship_dir == 'S') {
                player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].y += (part + 1);
                player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].x -= side * (part - len + 1);
        }
        else if (ship_dir == 'E') {
                player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].y -= side * (len - 1 - part);
                player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].x += (part + 1);
        }
        int y_current = player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].y;
        int x_current = player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].coordinates[part].x;

        correct_move(player, current_player, board, y_current, x_current, y_temp, x_temp, class_of_ship, number_of_ship, dir, size_of_board);
}

void move_ship(struct Player* player, int current_player, char **board, char class_of_ship[4], int number_of_ship, char direction, int size_of_board[2]) {
        int checked_class_of_ship = check_class_of_ship(class_of_ship);
        int len = player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].lenght;
        if (direction == 'F') {
                for (int i = 0; i < len; i++)
                {
                        move_forward(player, current_player, board, class_of_ship, number_of_ship, i, direction, size_of_board);
                }
        }
        else if (direction == 'L') {
                for (int i = 0; i < len; i++)
                {
                        move_side(player, current_player, board, class_of_ship, number_of_ship, i, len, direction, size_of_board);
                }
                turn_left(player, current_player, checked_class_of_ship, number_of_ship);
        }
        else if (direction == 'R') {
                for (int i = 0; i < len; i++)
                {
                        move_side(player, current_player, board, class_of_ship, number_of_ship, i, len, direction, size_of_board);
                }
                turn_right(player, current_player, checked_class_of_ship, number_of_ship);
        }
        player[current_player].number_of_ships_in_class[checked_class_of_ship].ships[number_of_ship].moves += 1;
}

void move(struct Player* player, int current_player, char **board, int size_of_board[2]) {
        int number_of_ship;
        char class_of_ship[4], direction;
        cin >> number_of_ship;
        cin >> class_of_ship;
        cin >> direction;
        class_of_ship[3] = '\0';

        move_ship(player, current_player, board, class_of_ship, number_of_ship, direction, size_of_board);
}

void spy(struct Player* player, int current_player) {
        int number_of_carrier, y, x;
        cin >> number_of_carrier;
        cin >> y; cin >> x;

        if (player[current_player].number_of_ships_in_class[CAR].ships[number_of_carrier].shoots >= length_of_ship(CAR)) {
                cout << "INVALID OPERATION \"SPY " << number_of_carrier << " " << y << " " << x << "\": ALL PLANES SENT";
                exit(0);
        }
        else if (!player[current_player].number_of_ships_in_class[CAR].ships[number_of_carrier].radar) {
                cout << "INVALID OPERATION \"SPY " << number_of_carrier << " " << y << " " << x << "\": CANNOT SEND PLANE";
                exit(0);
        }
        for (int i = 0; i < MAX_SPIES; i++)
        {
                if (!(player[current_player].spies_send[i].y >= 0)) { //sprawdzenie czy w tabeli na pozyci "i" jest pusta
                        player[current_player].spies_send[i].y = y;
                        player[current_player].spies_send[i].x = x;
                        player[current_player].number_of_spies++;
                        player[current_player].number_of_ships_in_class[CAR].ships[number_of_carrier].shoots++;
                        break;
                }
        }
}

void state_activity(bool &state) {
        if (state) state = false;
        else state = true;
}

void save(struct Player* player, int current_player, int size_of_board[2], Point *reef_send, int number_of_reefs, bool extended_ships) {
        cout << "[state]" << endl;
        cout << "BOARD_SIZE " << size_of_board[0] << " " << size_of_board[1] << endl;
        cout << "NEXT_PLAYER " << convert_number_of_player_to_char(next_player(current_player));
        for (int i = 0; i < NUMBER_OF_PLAYERS; i++)
        {
                cout << endl << "INIT_POSITION " << convert_number_of_player_to_char(i);
                for (int j = 0; j < 4; j++)
                {
                        cout << " " << player[i].available_position[j];
                }
                cout << endl << "SET_FLEET " << convert_number_of_player_to_char(i);
                for (int  j= 0; j < 4; j++)
                {
                        cout << " " << player[i].number_of_ships_in_class[j].number_of_ships;
                }
                for (int j = 0; j < 4; j++)
                {
                        for (int k = 0; k < player[i].number_of_ships_in_class[j].number_of_ships; k++)
                        {
                                //IDK jak to zrobic
                                char name[4];
                                
                                if (j == CAR) strcpy(name, "CAR");
                                else if (j == BAT) strcpy(name, "BAT");
                                else if (j == CRU) strcpy(name, "CRU");
                                else if (j == DES) strcpy(name, "DES");
                                
                                cout << endl << "SHIP " << convert_number_of_player_to_char(i) << " " <<
                                        player[i].number_of_ships_in_class[j].ships[k].coordinates[0].y << " " <<
                                        player[i].number_of_ships_in_class[j].ships[k].coordinates[0].x << " " <<
                                        player[i].number_of_ships_in_class[j].ships[k].direction << " " << k << " " << name << " ";
                                for (int l = 0; l < player[i].number_of_ships_in_class[j].ships[k].lenght; l++)
                                {
                                        cout << player[i].number_of_ships_in_class[j].ships[k].structure[l];
                                }
                        }
                }
        }
        for (int i = 0; i < number_of_reefs; i++)
        {
                cout << endl << "REEF " << reef_send[i].y << " " << reef_send[i].x;
        }
        if (extended_ships) cout << endl << "EXTENDED_SHIP " << extended_ships;
        cout << endl << "[state]";
}

int set_number_of_reefs(int size_of_board[2]) {
        return (size_of_board[0] * size_of_board[1] - NUMBER_OF_PLAYERS * MAX_SHIPS * length_of_ship(CAR)) / 2; //liczba reefów może zajmować maksymalnie połowa pustych pól planszy
}

int main() {
        int size_of_board[2] = { BOARD_SIZE_Y, BOARD_SIZE_X };
        char **board = set_new_board(size_of_board);

        Player a, b;
        Player player[NUMBER_OF_PLAYERS] = { a, b };
        config_ships(player);
        set_available_position(player, size_of_board);

        int max_number_of_reefs = set_number_of_reefs(size_of_board);
        Point *reef_send = (Point*)malloc(max_number_of_reefs * sizeof(Point));
        int number_of_reefs = 0;

        int current_player = PLAYER_A;
        bool move_playerA = 0;
        bool move_playerB = 0;
        int number_of_command = 1; //dafault -1

        bool is_state = false;
        bool extended_ships = false;

        char command[MAX_LENGTH_COMMAND];
        
        while (cin >> command) {
                switch (check_command(command))
                {
                        /*aby dodac case trzeba:
                        1.Dodac #define <nazwa kmendy> <kolejna wartość>
                        2.W funckji check_command dodac nazwe komendy do tablicy komend
                        3.Ewentualnie zwiekszyc stała NUMBER_OF_COMMANDS
                        */
                case PLAYER_A:
                        player_move(player, number_of_command, current_player, PLAYER_A, move_playerA, command);
                        break;
                case PLAYER_B:
                        player_move(player, number_of_command, current_player, PLAYER_B, move_playerB, command);
                        break;
                case PRINT:
                        short int mode;
                        cin >> mode; // poxniej tutaj bedzie na 0 lub 1
                        if (mode == 0) {
                                if (!move_playerA && !move_playerB) {
                                        show_board(board, size_of_board);
                                        parts_remaining(player, board);
                                }
                                else show_board_with_radars(player, current_player, board, size_of_board);
                        }
                        else if (mode == 1) {
                                show_board_extended(player, board, size_of_board);
                                parts_remaining(player, board);
                        }
                        break;
                case SET_FLEET:
                        set_fleet(player);
                        break;
                case NEXT_PLAYER:
                        current_player = next_player(current_player);
                        break;
                case PLACE_SHIP:
                        place_ship(player, current_player, board, command);
                        break;
                case SHOOT:
                        shoot(player, current_player, board, command, size_of_board, extended_ships);
                        break;
                case BOARD_SIZE:
                        free_memory(board, size_of_board[0]);
                        int y, x;
                        cin >> y; cin >> x;
                        size_of_board[0] = y;
                        size_of_board[1] = x;
                        board = set_new_board(size_of_board);
                        break;
                case REEF:
                        add_reef(board, size_of_board, reef_send, number_of_reefs);
                        break;
                case INIT_POSITION:
                        init_position(player);
                        break;
                case SHIP:
                        place_ship(player, current_player, board, command);
                        break;
                case MOVE:
                        move(player, current_player, board, size_of_board);
                        break;
                case EXTENDED_SHIPS:
                        extended_ships = true;
                        break;
                case SPY:
                        spy(player, current_player);
                        break;
                case STATE:
                        state_activity(is_state);
                        break;
                case SAVE:
                        if(is_state) save(player, current_player, size_of_board, reef_send, number_of_reefs, extended_ships);//Dodac tutaj jeszcze opcje z AI
                        break;
                case SRAND:
                        unsigned int random_number;
                        cin >> random_number;
                        srand(random_number);
                        break;
                case SET_AI_PLAYER:
                        int number_of_player;
                        cin >> number_of_player;
                        break;
                default:
                        break;
                }
        }
        free_memory(board, size_of_board[0]);
        free(reef_send);
        return 0;
}