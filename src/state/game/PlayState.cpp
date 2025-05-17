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
	map(48, 32),
	camera(window)
{
	map.loadFromJson("assets/maps/test_map.json");
}

void PlayState::processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events)
{
	// Enter editor state
	if (Utility::isKeyReleased(sf::Keyboard::Key::F1))
		stateManager.push(std::make_unique<EditorState>(stateManager, *this, map, player));

	player.processInput(window, events);
}

void PlayState::update(float fixedTimeStep)
{
	player.update(fixedTimeStep, map);
}

void PlayState::render(sf::RenderWindow& window, float interpolationFactor, float fixedTimeStep)
{
	camera.updateVerticalLook(fixedTimeStep, player.isLookingUp(), player.isLookingDown());
	camera.preRenderUpdate(fixedTimeStep, interpolationFactor, player);
	map.drawTransparentOnly = false;
	window.draw(map);
	player.render(window, interpolationFactor);
	map.drawTransparentOnly = true;
	window.draw(map);
}

void PlayState::applyView(sf::RenderWindow& window)
{
	window.setView(camera.getView());
}
