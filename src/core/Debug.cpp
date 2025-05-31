// ================================================================================================
// File: Debug.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 31, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include "Debug.hpp"

void lv::Debug::processInput(const std::vector<sf::Event>& events, sf::Vector2f mouseWorldPosition, World& world)
{
	for (const auto& event : events)
	{
		if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>())
		{
			if (mouseReleased->button == sf::Mouse::Button::Left)
			{
				if (Game::getInstance().isDebugModeOn())
				{
					// Enemy selection toggle
					for (auto& enemy : world.getCurrentArea().enemies)
						if (enemy->getBounds().contains(mouseWorldPosition))
							enemy->toggleSelected();
				}
			}
			else if (mouseReleased->button == sf::Mouse::Button::Right)
			{
				if (Game::getInstance().isDebugModeOn())
				{
					// Enemy selection clear
					for (auto& enemy : world.getCurrentArea().enemies)
						enemy->setSelected(false);
				}
			}
		}
		if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>())
		{
			if (keyReleased->code == sf::Keyboard::Key::F3)
				toggleDebugMode();
		}
	}
}