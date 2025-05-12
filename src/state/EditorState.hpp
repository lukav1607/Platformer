// ================================================================================================
// File: EditorState.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// Description: Defines the EditorState class, which represents the editor state of the game. The
//              editor state allows the user to edit the game world, including placing and removing
//              tiles, and saving/loading the game world. It gets overlaid on top of the PlayState.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <SFML/Graphics/View.hpp>
#include "State.hpp"
#include "../world/TileMap.hpp"

class EditorState : public State
{
public:
	EditorState(StateManager& stateManager, TileMap& map);

	void processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events) override;
	void update(float fixedTimeStep) override;
	void render(sf::RenderWindow& window, float interpolationFactor) override;

	void applyView(sf::RenderWindow& window) override;

	bool isTransparent() const override { return true; } // Render state(s) underneath
	bool isTranscendent() const override { return false; } // Do not update state(s) underneath

private:
	TileMap& map; // Reference to the tile map being edited
	std::vector<Tile::Type> palette;
	unsigned selectedTileIndex;

	enum class Mode
	{
		NONE,
		TILES,
		OBJECTS,
		ENEMIES,
		ITEMS
	};
	Mode mode;

	struct Camera
	{
		sf::View view;

		static constexpr float MOVE_SPEED = 500.f;
		sf::Vector2f direction;

		static constexpr float PAN_SPEED = 1500.f;
		sf::Vector2f anchorPoint;
		bool isPanning = false;

		static constexpr float ZOOM_SPEED = 5.f;
		static constexpr float ZOOM_MIN = 0.5f;
		static constexpr float ZOOM_MAX = 4.f;
		float zoomLevel = 1.f;
	};
	static Camera camera;

	float mouseWheelDelta;
	sf::Vector2f mouseWorldPosition;
};