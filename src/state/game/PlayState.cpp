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
#include "../StateManager.hpp"
#include "../editor/EditorState.hpp"
#include "../../core/Utility.hpp"

PlayState::PlayState(StateManager& stateManager, sf::RenderWindow& window) :
	State(stateManager),
	map(32, 8),
	camera(window)
{
	// Load map from JSON file
	map.loadFromJson("assets/maps/test_map.json");
}

void PlayState::processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events)
{
	// Enter editor state
	if (Utility::isKeyReleased(sf::Keyboard::Key::F1))
		stateManager.push(std::make_unique<EditorState>(stateManager, *this, map));

	player.processInput(window, events);
}

void PlayState::update(float fixedTimeStep)
{
	player.update(fixedTimeStep, map);
	//camera.update(fixedTimeStep, player);
}

void PlayState::render(sf::RenderWindow& window, float interpolationFactor, float fixedTimeStep)
{
	camera.preRenderUpdate(fixedTimeStep, interpolationFactor, player);
	window.draw(map);
	player.render(window, interpolationFactor);
}

void PlayState::applyView(sf::RenderWindow& window)
{
	window.setView(camera.getView());
}
