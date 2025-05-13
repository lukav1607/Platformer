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
		Tile::Type::EMPTY,
		Tile::Type::BACKGROUND,
		Tile::Type::SOLID,
		Tile::Type::WATER,
		Tile::Type::DOOR
	},
	selectedTileIndex(0U),
	gridLines(sf::PrimitiveType::Lines),
	gridColor(sf::Color(255, 255, 255, 128)),
	isGridShown(true),
	mode(Mode::TILES),
	mouseWheelDelta(0.f)
{
	rebuildGridLines();
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
	if (Utility::isKeyReleased(sf::Keyboard::Key::Num2))
		mode = Mode::OBJECTS;
	if (Utility::isKeyReleased(sf::Keyboard::Key::Num3))
		mode = Mode::ENEMIES;
	if (Utility::isKeyReleased(sf::Keyboard::Key::Num4))
		mode = Mode::ITEMS;
	if (Utility::isKeyReleased(sf::Keyboard::Key::Num0))
		mode = Mode::NONE;
	if (Utility::isKeyReleased(sf::Keyboard::Key::G))
		isGridShown = !isGridShown;

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
		bool hasPaletteBeenClicked = false;

		for (size_t i = 0; i < palette.size(); ++i)
		{
			sf::FloatRect bounds({ 10.f + i * 60.f, 10.f }, { 50.f, 50.f });
			if (bounds.contains(sf::Vector2f(sf::Mouse::getPosition(window))))
			{
				selectedTileIndex = i;
				hasPaletteBeenClicked = true;
				break;
			}
		}
		if (map.isWithinBounds(tileCoords) && !hasPaletteBeenClicked)
		{
			map.setTile(tileCoords.x, tileCoords.y, Tile{ palette.at(selectedTileIndex) });
		}
	}
	/*else if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
	{
		if (map.isWithinBounds(tileCoords))
		{
			map.setTile(tileCoords.x, tileCoords.y, Tile{ Tile::Type::EMPTY });
		}
	}*/
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
		camera.view.move(delta * fixedTimeStep * 20.f);
	}
	// Update the camera position when moving with the arrow keys
	else
	{
		camera.view.move(camera.direction * camera.MOVE_SPEED * camera.zoomLevel * fixedTimeStep);
	}
}

void EditorState::render(sf::RenderWindow& window, float interpolationFactor)
{
	sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window), camera.view);
	sf::Vector2i tileCoords = Utility::worldToTileCoords(mouseWorldPos);

	//=====================================================================//
	//                         DRAW IN WORLD                               //
	//=====================================================================//
	window.setView(camera.view);

	// Draw the grid lines and tilemap border
	if (isGridShown)
	{
		sf::RectangleShape border(sf::Vector2f(map.getSize().x * TileMap::TILE_SIZE, map.getSize().y * TileMap::TILE_SIZE));
		border.setFillColor(sf::Color::Transparent);
		border.setOutlineThickness(2.f);
		border.setOutlineColor(sf::Color::White);
		window.draw(border);
		window.draw(gridLines);
	}

	// Draw the tile placement/erase preview
	if (map.isWithinBounds(tileCoords) && map.getTile(tileCoords).type != palette.at(selectedTileIndex))
	{
		sf::RectangleShape preview(sf::Vector2f(TileMap::TILE_SIZE, TileMap::TILE_SIZE));
		preview.setPosition({ static_cast<float>(tileCoords.x * TileMap::TILE_SIZE), static_cast<float>(tileCoords.y * TileMap::TILE_SIZE) });
		preview.setFillColor([&]
			{
				switch (palette.at(selectedTileIndex))
				{
				case Tile::Type::EMPTY: return sf::Color::Transparent;
				case Tile::Type::BACKGROUND: return sf::Color(255, 255, 0, 100);
				case Tile::Type::SOLID: return sf::Color(0, 255, 0, 100);
				case Tile::Type::WATER: return sf::Color(0, 0, 255, 100);
				case Tile::Type::DOOR: return sf::Color(255, 0, 255, 100);
				default: return sf::Color::Transparent;
				}
			}());
		preview.setOutlineColor([&]
			{
				if (palette.at(selectedTileIndex) == Tile::Type::EMPTY)
					return sf::Color::Red;
				return sf::Color::Transparent;
			}());
		preview.setOutlineThickness(2.f);
		window.draw(preview);
	}

	//=====================================================================//
	//                           DRAW IN UI                                //
	//=====================================================================//
	window.setView(uiView);

	// Draw the tile palette
	for (size_t i = 0; i < palette.size(); ++i)
	{
		sf::RectangleShape shape(sf::Vector2f(50.f, 50.f));
		shape.setPosition({ 10.f + i * 60.f, 10.f });

		switch (palette.at(i))
		{
		case Tile::Type::EMPTY: shape.setFillColor(sf::Color::Transparent); shape.setOutlineColor(sf::Color::Red); shape.setOutlineThickness(2.f); break;
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
	// Resize UI view to match the window size
	uiView.setSize({ static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y) });
	uiView.setCenter({ static_cast<float>(window.getSize().x) / 2.f, static_cast<float>(window.getSize().y) / 2.f });

	// Apply camera zoom and set the window view size
	camera.view.setSize({ window.getSize().x * camera.zoomLevel, window.getSize().y * camera.zoomLevel });
	///window.setView(camera.view);
}

void EditorState::rebuildGridLines()
{
	//	Vertical lines:
	for (int x = 0; x <= map.getSize().x; ++x)
	{
		float xpos = static_cast<float>(x * TileMap::TILE_SIZE);
		gridLines.append(sf::Vertex{ { sf::Vector2f(xpos, 0.f) }, gridColor });
		gridLines.append(sf::Vertex{ { sf::Vector2f(xpos, map.getSize().y * TileMap::TILE_SIZE) }, gridColor });
	}
	//	Horizontal lines:
	for (int y = 0; y <= map.getSize().y; ++y)
	{
		float ypos = static_cast<float>(y * TileMap::TILE_SIZE);
		gridLines.append(sf::Vertex{ { sf::Vector2f(0.f, ypos) }, gridColor });
		gridLines.append(sf::Vertex{ { sf::Vector2f(map.getSize().x * TileMap::TILE_SIZE, ypos)}, gridColor });
	}
}
