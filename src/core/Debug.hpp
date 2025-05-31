// ================================================================================================
// File: Debug.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 31, 2025
// Description: A utility header for debugging functionalities in the game.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include "Game.hpp"
#include "../world/World.hpp"

namespace lv
{
	namespace Debug
	{
		inline bool isDebugModeOn()
		{
			return Game::getInstance().isDebugModeOn();
		}
		inline void toggleDebugMode()
		{
			Game::getInstance().toggleDebugMode();
		}
		inline void setDebugMode(bool enabled)
		{
			Game::getInstance().setDebugMode(enabled);
		}

		void processInput(const std::vector<sf::Event>& events, sf::Vector2f mouseWorldPosition, World& world);		
	}
}