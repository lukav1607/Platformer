// ================================================================================================
// File: EditorState.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <iostream>
#include <algorithm>
#include "EditorState.hpp"
#include "StateManager.hpp"
#include "../core/Utility.hpp"

EditorState::Camera EditorState::camera;

EditorState::EditorState(StateManager& stateManager, TileMap& map) :
	State(stateManager),
	map(map)
{
}

void EditorState::processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events)
{
	// Exit editor state
	if (Utility::isKeyReleased(sf::Keyboard::Key::F1))
	{
		stateManager.pop();
		return;
	}

	// SFML Events
	for (const auto& event : events)
	{
		// Handle mouse wheel scroll (for camera zoom)
		if (const auto* mouseWheelScrolled = event.getIf<sf::Event::MouseWheelScrolled>())
			mouseWheelDelta += mouseWheelScrolled->delta;
	}

	camera.direction = { 0.f, 0.f }; // Reset direction to zero each frame before checking input

	// Camera movement
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
		camera.direction.x = -1.f;
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
		camera.direction.x = 1.f;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
		camera.direction.y = -1.f;
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
		camera.direction.y = 1.f;

	// Get mouse position in world coordinates relative to the view and convert to tile coordinates
	sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window), camera.view);
	sf::Vector2i tileCoords = Utility::worldToTileCoords(worldPos);

	// Tile placement
	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
	{
		if (map.isWithinBounds(tileCoords))
		{
			map.setTile(tileCoords.x, tileCoords.y, Tile{ Tile::Type::SOLID });
		}
	}
	else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
	{
		if (map.isWithinBounds(tileCoords))
		{
			map.setTile(tileCoords.x, tileCoords.y, Tile{ Tile::Type::EMPTY });
		}
	}
}

void EditorState::update(float fixedTimeStep)
{
	// Camera zooming
	if (mouseWheelDelta != 0.f)
	{
		camera.zoomLevel -= mouseWheelDelta * camera.ZOOM_SPEED * fixedTimeStep;
		camera.zoomLevel = std::clamp(camera.zoomLevel, camera.ZOOM_MIN, camera.ZOOM_MAX);
		mouseWheelDelta = 0.f;
	}

	// Update the camera position
	camera.view.move(camera.direction * camera.MOVE_SPEED * fixedTimeStep);
}

void EditorState::render(sf::RenderWindow& window, float interpolationFactor)
{
}

void EditorState::applyView(sf::RenderWindow& window)
{
	// Apply camera zoom and set the window view
	camera.view.setSize({ window.getSize().x * camera.zoomLevel, window.getSize().y * camera.zoomLevel });
	window.setView(camera.view);
}