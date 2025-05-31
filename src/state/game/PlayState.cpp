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
#include "../../core/Debug.hpp"

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
			enemy->syncPosition();
		stateManager.push(std::make_unique<EditorState>(stateManager, *this, world, player, world.getCurrentArea().enemies, font));
	}

	sf::Vector2i mouseWindowPosition = sf::Mouse::getPosition(window);                                // Get mouse position in window coordinates	
	sf::Vector2f mouseWorldPosition = window.mapPixelToCoords(mouseWindowPosition, camera.getView()); // Get mouse position in world coordinates relative to the view

	lv::Debug::processInput(events, mouseWorldPosition, world);

	//for (const auto& event : events)
	//{
	//	if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>())
	//	{
	//		if (mouseReleased->button == sf::Mouse::Button::Left)
	//		{
	//			if (Game::getInstance().isDebugModeOn())
	//			{
	//				// Enemy selection toggle
	//				for (auto& enemy : world.getCurrentArea().enemies)
	//					if (enemy->getBounds().contains(mouseWorldPosition))
	//						enemy->toggleSelected();
	//			}
	//		}
	//		else if (mouseReleased->button == sf::Mouse::Button::Right)
	//		{
	//			if (Game::getInstance().isDebugModeOn())
	//			{
	//				// Enemy selection clear
	//				for (auto& enemy : world.getCurrentArea().enemies)
	//					enemy->setSelected(false);
	//			}
	//		}
	//	}
	//}

	player.processInput(window, events);
}

void PlayState::update(float fixedTimeStep)
{
	player.update(fixedTimeStep, world.getCurrentArea().map);

	//world.getCurrentArea().enemies.erase(std::remove_if(world.getCurrentArea().enemies.begin(), world.getCurrentArea().enemies.end(),
	//	[](const Enemy& enemy) { return !enemy.isAlive() || enemy.getPatrolPositions().empty(); }));

	world.getCurrentArea().enemies.erase(
		std::remove_if(world.getCurrentArea().enemies.begin(), world.getCurrentArea().enemies.end(),
			[](const std::unique_ptr<lv::Enemy>& enemy) {
				return !enemy->isAlive() || enemy->getPatrolPositions().empty();
			}),
		world.getCurrentArea().enemies.end()
	);

	for (auto& enemy : world.getCurrentArea().enemies)
		enemy->update(fixedTimeStep, world.getCurrentArea().map, player);


	camera.update(fixedTimeStep, player);
}

void PlayState::render(sf::RenderWindow& window, float interpolationFactor)
{
	camera.applyInterpolatedPosition(interpolationFactor);

	world.getCurrentArea().map.drawTransparentOnly = false;
	window.draw(world.getCurrentArea().map);

	world.getCurrentArea().map.drawTransparentOnly = true;
	window.draw(world.getCurrentArea().map);

	world.getCurrentArea().map.setIsGridShown(Game::getInstance().isDebugModeOn());
	world.getCurrentArea().map.renderGrid(window);

	player.render(window, interpolationFactor);
	for (auto& enemy : world.getCurrentArea().enemies)
		enemy->render(window, font, interpolationFactor);
}

void PlayState::applyView(sf::RenderWindow& window)
{
	window.setView(camera.getView());
}
