// ================================================================================================
// File: PlayState.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <vector>
#include <memory>
#include "PlayState.hpp"
#include "../StateManager.hpp"
#include "../editor/EditorState.hpp"
#include "../../core/Utility.hpp"
#include "../../core/Game.hpp"

PlayState::PlayState(StateManager& stateManager, sf::RenderWindow& window, sf::Font& font) :
	State(stateManager),
	world(player),
	camera(window),
	font(font)
{
	world.getCurrentArea().load("assets/maps/test_map.json");
}

void PlayState::processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events)
{
	// Enter editor state
	if (Utility::isKeyReleased(sf::Keyboard::Key::F1))
	{
		player.equalizePositions();
		for (auto& enemy : world.getCurrentArea().enemies)
			enemy->equalizePositions();
		stateManager.push(std::make_unique<EditorState>(stateManager, *this, world, player, world.getCurrentArea().enemies, font));
	}
	//else if (Utility::isKeyReleased(sf::Keyboard::Key::F3))
	//	map.setIsGridShown(Game::isDebugModeOn());

	player.processInput(window, events);
}

void PlayState::update(float fixedTimeStep)
{
	player.update(fixedTimeStep, world.getCurrentArea().map);

	//world.getCurrentArea().enemies.erase(std::remove_if(world.getCurrentArea().enemies.begin(), world.getCurrentArea().enemies.end(),
	//	[](const Enemy& enemy) { return !enemy.isAlive() || enemy.getPatrolPositions().empty(); }));

	world.getCurrentArea().enemies.erase(
		std::remove_if(world.getCurrentArea().enemies.begin(), world.getCurrentArea().enemies.end(),
			[](const std::unique_ptr<Enemy>& enemy) {
				return !enemy->isAlive() || enemy->getPatrolPositions().empty();
			}),
		world.getCurrentArea().enemies.end()
	);

	for (auto& enemy : world.getCurrentArea().enemies)
		enemy->update(fixedTimeStep, world.getCurrentArea().map, player.getLogicPositionCenter());


	camera.update(fixedTimeStep, player);
}

void PlayState::render(sf::RenderWindow& window, float interpolationFactor)
{
	camera.applyInterpolatedPosition(interpolationFactor);

	world.getCurrentArea().map.drawTransparentOnly = false;
	window.draw(world.getCurrentArea().map);

	player.render(window, interpolationFactor);
	for (auto& enemy : world.getCurrentArea().enemies)
		enemy->render(window, font, interpolationFactor);

	world.getCurrentArea().map.drawTransparentOnly = true;
	window.draw(world.getCurrentArea().map);

	world.getCurrentArea().map.setIsGridShown(Game::isDebugModeOn());
	world.getCurrentArea().map.renderGrid(window);
}

void PlayState::applyView(sf::RenderWindow& window)
{
	window.setView(camera.getView());
}
