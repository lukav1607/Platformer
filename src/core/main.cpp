// ================================================================================================
// File: main.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// Description: Starting point for the project.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

//#include <memory>
#include "Game.hpp"

int main()
{
	//std::unique_ptr<Game> game = std::make_unique<Game>(Game::getInstance());
    //return game->run();
	return Game::getInstance().run();
}