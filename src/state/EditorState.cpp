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
	map(map),
	palette{
		Tile::Type::BACKGROUND,
		Tile::Type::SOLID,
		Tile::Type::WATER,
		Tile::Type::DOOR
	},
	mode(Mode::TILES),
	selectedTileIndex(0U),
	mouseWheelDelta(0.f)
{
}

void EditorState::processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events)
{
	// SFML Events
	for (const auto& event : events)
	{
		// Handle mouse wheel scroll (for camera zoom)
		if (const auto* mouseWheelScrolled = event.getIf<sf::Event::MouseWheelScrolled>())
			mouseWheelDelta += mouseWheelScrolled->delta;
	}

	// Exit editor state
	if (Utility::isKeyReleased(sf::Keyboard::Key::F1))
	{
		stateManager.pop();
		return;
	}

	// Editor mode
	if (Utility::isKeyReleased(sf::Keyboard::Key::Num1))
		mode = Mode::TILES;
	else if (Utility::isKeyReleased(sf::Keyboard::Key::Num2))
		mode = Mode::OBJECTS;
	else if (Utility::isKeyReleased(sf::Keyboard::Key::Num3))
		mode = Mode::ENEMIES;
	else if (Utility::isKeyReleased(sf::Keyboard::Key::Num4))
		mode = Mode::ITEMS;
	else if (Utility::isKeyReleased(sf::Keyboard::Key::Num0))
		mode = Mode::NONE;

	camera.direction = { 0.f, 0.f }; // Reset direction to zero each frame before checking input

	// Camera movement with arrow keys
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
		camera.direction.x = -1.f;
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
		camera.direction.x = 1.f;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
		camera.direction.y = -1.f;
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
		camera.direction.y = 1.f;

	// Get mouse position in world coordinates relative to the view and convert to tile coordinates
	mouseWorldPosition = window.mapPixelToCoords(sf::Mouse::getPosition(window), camera.view);
	sf::Vector2i tileCoords = Utility::worldToTileCoords(mouseWorldPosition);

	// Camera movement with middle mouse panning
	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
	{
		if (!camera.isPanning)
		{
			camera.isPanning = true;
			camera.anchorPoint = mouseWorldPosition;
		}
	}
	else
	{
		camera.isPanning = false;
	}

	// Tile placement
	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
	{
		for (size_t i = 0; i < palette.size(); ++i)
		{
			sf::FloatRect bounds({ 10.f + i * 60.f, 10.f }, { 50.f, 50.f });
			if (bounds.contains(sf::Vector2f(sf::Mouse::getPosition(window))))
			{
				selectedTileIndex = i;
				break;
			}
		}
		if (map.isWithinBounds(tileCoords))
		{
			map.setTile(tileCoords.x, tileCoords.y, Tile{ palette.at(selectedTileIndex) });
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

	// Update the camera position when panning with the middle mouse button
	if (camera.isPanning)
	{
		sf::Vector2f delta = camera.anchorPoint - mouseWorldPosition;
		camera.view.move(delta);
	}
	// Update the camera position when moving with the arrow keys
	else
	{
		camera.view.move(camera.direction * camera.MOVE_SPEED * camera.zoomLevel * fixedTimeStep);
	}
}

void EditorState::render(sf::RenderWindow& window, float interpolationFactor)
{
	// Draw UI
	window.setView(window.getDefaultView());

	for (size_t i = 0; i < palette.size(); ++i)
	{
		sf::RectangleShape shape(sf::Vector2f(50.f, 50.f));
		shape.setPosition({ 10.f + i * 60.f, 10.f });

		switch (palette.at(i))
		{
		//case Tile::Type::EMPTY: shape.setFillColor(sf::Color::Transparent); break;
		case Tile::Type::BACKGROUND: shape.setFillColor(sf::Color::Yellow); break;
		case Tile::Type::SOLID: shape.setFillColor(sf::Color::Green); break;
		case Tile::Type::WATER: shape.setFillColor(sf::Color::Blue); break;
		case Tile::Type::DOOR: shape.setFillColor(sf::Color::Magenta); break;
		}

		if (i == selectedTileIndex)
		{
			shape.setOutlineThickness(2.f);
			shape.setOutlineColor(sf::Color::White);
		}
		window.draw(shape);
	}

	window.setView(camera.view);
}

void EditorState::applyView(sf::RenderWindow& window)
{
	// Apply camera zoom and set the window view
	camera.view.setSize({ window.getSize().x * camera.zoomLevel, window.getSize().y * camera.zoomLevel });
	window.setView(camera.view);
}