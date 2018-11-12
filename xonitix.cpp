/* Copyright 2018 Breno G. de Oliveira
 * Licensed under the MIT License
 *
 * Xonitix is a game where you gain points by
 * limiting the space of the moving dots.
 *
 * Press the left/right arrow keys to move, down to stop,
 * spacebar to block. Fill 50% or more to level up!
 *
 * To compile on OSX, Linux and BSD systems:
 *    g++ -Wall -g -std=c++11 xonitix.cpp -o xonitix
 * (-std=c++14 should also work without errors)
 *
 * To compile on Windows, use Visual Studio with C++ support
 * (the free Community Edition works just fine)
 *
 */
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#endif

const std::string VERSION = "1.1";

class Actor {
 public:
  // Initializes with random position/direction/speed:
  void Init(const int screen_size) {
    screen_size_ = screen_size;
    pos_ = rand() % screen_size_;
    direction_ = rand() % 2 == 0 ? GOING_LEFT : GOING_RIGHT;
    speed_ = (rand() % 2) + 1;
  }
  int x() { return pos_; }
  // Moves the actor according to its speed and direction:
  void Update(std::vector<int> walls) {
    int closest_wall = ClosestWall_(walls);
    pos_ += (direction_ * speed_);
    if (direction_ == GOING_RIGHT && pos_ >= closest_wall) {
      pos_ = closest_wall - 1;
      direction_ *= -1;
    } else if (direction_ == GOING_LEFT && pos_ <= closest_wall) {
      pos_ = closest_wall + 1;
      direction_ *= -1;
    }
  }

 private:
  // Returns the position of the wall closest to Actor
  // (in the direction it is moving):
  int ClosestWall_(std::vector<int> &walls) {
    int closest = direction_ == GOING_LEFT ? 0 : screen_size_;
    for (int wall : walls) {
      if (direction_ == GOING_LEFT) {
        if (wall < pos_ && wall > closest) {
          closest = wall;
        }
      } else {
        if (wall > pos_ && wall < closest) {
          closest = wall;
        }
      }
    }
    return closest;
  }

 protected:
  int pos_, direction_, speed_, screen_size_;
  static const int GOING_LEFT = -1;
  static const int GOING_RIGHT = 1;
  static const int STOPPED = 0;
};

using EnemyList = std::vector<Actor>;

class Hero : public Actor {
  bool is_firing_ = false, dead_ = false;
  std::vector<int> shots_;

 public:
  // Our Hero starts at a random position,
  // but at a steady speed and not moving at all:
  void Init(int screen_size) {
    screen_size_ = screen_size;
    pos_ = rand() % screen_size_;
    direction_ = 0;
    speed_ = 1;
  }
  void SlideLeft() { direction_ = GOING_LEFT; }
  void SlideRight() { direction_ = GOING_RIGHT; }
  std::vector<int> walls() { return shots_; }
  void Stop() { direction_ = 0; }
  void Fire() { is_firing_ = true; }
  void Die() { dead_ = true; }
  bool is_dead() { return dead_; }
  bool ShouldLevelUp() {
    return (static_cast<float>(shots_.size()) /
                static_cast<float>(screen_size_) >
            0.50);  // 50% + 1
  }
  bool Update(EnemyList enemies, int &score) {
    pos_ += (direction_ * speed_);
    if (pos_ > screen_size_) {
      direction_ = STOPPED;
      pos_ = screen_size_;
    } else if (pos_ < 0) {
      direction_ = STOPPED;
      pos_ = 0;
    }
    if (is_firing_) {
      is_firing_ = false;
      // first we find our limiting left/right walls
      int paint_left_until = 0, paint_right_until = screen_size_;
      for (int shot : shots_) {
        if (shot == pos_) {
          return false;
        }
        if (shot < pos_ && shot > paint_left_until) {
          paint_left_until = shot;
        }
        if (shot > pos_ && shot < paint_right_until) {
          paint_right_until = shot;
        }
      }
      // then we check enemies inside our walls
      for (auto &enemy : enemies) {
        int enemy_pos = enemy.x();
        // design tweak: it's too hard to hit an enemy precisely,
        // so we tested and a distance of 1 seemed the most
        // balanced in terms of experience.
        const int MARGIN = 1;
        // there is an enemy between left wall and hero
        if (enemy_pos > paint_left_until && enemy_pos < pos_ - MARGIN) {
          paint_left_until = pos_;
        }
        // there is an enemy between right wall and hero
        else if (enemy_pos < paint_right_until && enemy_pos > pos_ + MARGIN) {
          paint_right_until = pos_;
        }
        // shot directly at enemy. game over!
        else if (pos_ >= enemy_pos - MARGIN && pos_ <= enemy_pos + MARGIN) {
          return true;
        }
      }
      // finally we mark everything we hit to be painted
      for (int i = paint_left_until; i <= paint_right_until; i++) {
        shots_.push_back(i);
        score += 100;
      }
    }
    return false;
  }
};

void Update(EnemyList &enemies, Hero &hero, int &score) {
  bool collision = hero.Update(enemies, score);
  if (collision) {
    hero.Die();
    return;
  }
  for (auto &enemy : enemies) {
    enemy.Update(hero.walls());
  }
}

void Render(EnemyList &enemies, Hero hero, int score, int screen_size) {
  std::string line(screen_size + 1, ' ');
  line.append("| Level " + std::to_string(enemies.size()) + " Score " +
              std::to_string(score));
  for (auto &enemy : enemies) {
    int x = enemy.x();
    std::string enemy_sprite;
    if (line.compare(x, 1, " ") == 0) {
      enemy_sprite = ".";
    } else {
      enemy_sprite = ":";
    }
    line.replace(x, 1, enemy_sprite);
  }
  std::vector<int> walls = hero.walls();
  for (int wall : walls) {
    line.replace(wall, 1, "|");
  }
  std::string hero_sprite;
  if (line.compare(hero.x(), 1, "|") == 0) {
    hero_sprite = "L";
  } else {
    hero_sprite = "_";
  }
  line.replace(hero.x(), 1, hero_sprite);
  // hack: append extra space because previous string sometimes lingered
  std::cout << line << "    \r";
}

#ifdef _WIN32
void ProcessInput(Hero &hero) {
  if (GetAsyncKeyState(VK_LEFT)) {
    hero.SlideLeft();
  } else if (GetAsyncKeyState(VK_RIGHT)) {
    hero.SlideRight();
  } else if (GetAsyncKeyState(VK_DOWN)) {
    hero.Stop();
  } else if (GetAsyncKeyState(VK_SPACE)) {
    hero.Fire();
  } else if (GetAsyncKeyState(0x51)) {  // 'q'
    hero.Die();
  }
}

void HideCursor() {
  HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_CURSOR_INFO info;
  info.dwSize = 100;
  info.bVisible = FALSE;
  SetConsoleCursorInfo(consoleHandle, &info);
}
#else
// this function is heavily inspired Morgan McGuire's article:
// http://www.flipcode.com/archives/_kbhit_for_Linux.shtml
int DetectKeyPress() {
  static bool init = false;
  const int STDIN = 0;

  // initialization disables line buffering
  // to get single key presses:
  if (!init) {
    termios term{};
    tcgetattr(STDIN, &term);
    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN, TCSANOW, &term);
    setbuf(stdin, nullptr);
    init = true;
  }

  timeval timeout{};
  fd_set rdset;

  FD_ZERO(&rdset);
  FD_SET(STDIN, &rdset);
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  return select(STDIN + 1, &rdset, nullptr, nullptr, &timeout);
}

void ProcessInput(Hero &hero) {
  if (DetectKeyPress() != 0) {
    int k = std::cin.get();
    if (k == 27) {
      k = std::cin.get();
      if (k == 91) {
        k = std::cin.get();
        if (k == 67) {  // right key
          hero.SlideRight();
        } else if (k == 68) {  // left key
          hero.SlideLeft();
        } else if (k == 66) {  // down key
          hero.Stop();
        }
      }
    } else if (k == 32) {  // spacebar
      hero.Fire();
    } else if (k == 113) {  // 'q'
      hero.Die();
    }
  }
}

void HideCursor() { std::cout << "\e[?25l"; }

#endif

void ShowUsage() {
  std::cout
      << "xonitix " << VERSION
      << "\n\n"
         "Xonitix is a one-line game where you gain points by "
         "limiting the space of moving dots.\n\n"
         "Press the left/right arrow keys to move, down arrow to "
         "stop, and spacebar to block, 'q' to quit.\n"
         "Fill 50% of the line to level up!\n\n"
         "  -l 80      line size\n"
         "  -q         quiet mode (don't show header)\n"
         "  -s         stealth/boss mode ('q' removes game from terminal)\n"
         "  -h         shows this help\n";
}

int GetScreenSize() {
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  return csbi.srWindow.Right; // csbi.dwSize.X
#else
  struct winsize w {};
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w.ws_col;
#endif
}

int main(int argc, char **argv) {
  bool stealth = false;
  int score = 0;
  int screen_size = GetScreenSize() - 30;

  if (screen_size < 50) {
    std::cerr << "Your terminal window is too small to play! "
                 "Try increasing its width to at least 50 columns."
              << std::endl;
  }
  if (argc < 2) {
    std::cout << "xonitix " << VERSION
              << ": arrows move, space fires, q quits. try -h for usage\n";
  } else {
    for (int i = 1; i < argc; i++) {
      std::string argument(argv[i]);
      if (argument == "-h") {
        ShowUsage();
        exit(0);
      } else if (argument == "-s") {
        stealth = true;
      } else if (argument == "-l") {
        if (i + 1 < argc) {
          screen_size = std::stoi(argv[++i]);
        }
      }
    }
  }

  std::clock_t t_previous = std::clock();
  double lag = 0.0;
#ifdef _WIN32
  const int DELTA_PER_UPDATE = 30;
#else
  const int DELTA_PER_UPDATE = 10000;
#endif

  bool game_over = false;
  int enemy_count = 0;
  srand(t_previous);
  HideCursor();

  // this is our level loop. Every time the user reaches over 50%
  // of the screen, we move to the next level, which is resetting
  // the screen and adding one more enemy.
  while (!game_over) {
    enemy_count++;
    EnemyList enemies(enemy_count);

    for (int i = 0; i < enemy_count; i++) {
      Actor enemy {};
      enemy.Init(screen_size);
      enemies[i] = enemy;
    }

    Hero hero;
    hero.Init(screen_size);

    while (true) {
      std::clock_t t_current = std::clock();
      std::clock_t elapsed = t_current - t_previous;
      t_previous = t_current;
      lag += elapsed;

      ProcessInput(hero);
      while (lag >= DELTA_PER_UPDATE) {
        Update(enemies, hero, score);
        lag -= DELTA_PER_UPDATE;
      }
      Render(enemies, hero, score, screen_size);

      if (hero.is_dead()) {
        game_over = true;
        break;
      }
      if (hero.ShouldLevelUp()) {
        fflush(stdout);  // let the player see victory before level up!
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        break;
      }
    }
  }
  if (stealth) {
    std::string clear(screen_size + 20, ' ');
    std::cout << clear + "\r\n";
  } else {
    std::cout << "\n[G A M E  O V E R ]\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }
  exit(0);
}
