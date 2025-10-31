#ifndef CHAOSGAME_HPP
/** @file chaosgame.hpp
 *
 */
#define CHAOSGAME_HPP

#include <cmath>
#include <vector>
#include <chrono>
#include <cstdio>

#include <SDL2/SDL.h>

#include "egg2d.h"


void InitCGame();

bool RenderCGame();

void ShutdownCGame();

#endif