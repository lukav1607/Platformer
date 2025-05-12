// ================================================================================================
// File: PlayState.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <memory>
#include "PlayState.hpp"
#include "StateManager.hpp"
#include "EditorState.hpp"
#include "../core/Utility.hpp"

PlayState::PlayState(StateManager& stateManager) :
	State(stateManager),
	map(30, 20)
{
	for (int x = 0; x < 30; ++x)
		map.setTile(x, 19, Tile{ Tile::Type::SOLID });
}

void PlayState::processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events)
{
	// Enter editor state
	if (Utility::isKeyReleased(sf::Keyboard::Key::F1))
		stateManager.push(std::make_unique<EditorState>(stateManager, map));
}

void PlayState::update(float fixedTimeStep)
{
}

void PlayState::render(sf::RenderWindow& window, float interpolationFactor)
{
	window.draw(map);
}

void PlayState::applyView(sf::RenderWindow& window)
{
	// Temporary
	sf::View view = window.getDefaultView();
	view.setSize(sf::Vector2f(window.getSize().x, window.getSize().y));
	window.setView(view);
}
