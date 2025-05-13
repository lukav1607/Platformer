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

#include <stack>
#include <SFML/Graphics/View.hpp>
#include "State.hpp"
#include "PlayState.hpp"
#include "../world/TileMap.hpp"

struct Action;
struct TileAction;

class EditorState : public State
{
public:
	EditorState(StateManager& stateManager, PlayState& playState, TileMap& map);

	void processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events) override;
	void update(float fixedTimeStep) override;
	void render(sf::RenderWindow& window, float interpolationFactor) override;

	void applyView(sf::RenderWindow& window) override;

	bool isTranscendent() const override { return false; } // Do not update state(s) underneath
	bool isTransparent() const override { return false; } // Do not render state(s) underneath - NOTE: Editor is a special case where it still
														  // renders the PlayState from it's render function despite this flag.

private:
	TileMap& map; // Reference to the tile map being edited
	PlayState& playState; // Reference to the PlayState for rendering

	void rebuildGridLines();
	void renderGrid(sf::RenderWindow& window);
	sf::VertexArray gridLines;
	sf::Color gridColor;
	bool isGridShown;

	void handleTileInput(sf::Vector2i mouseWindowPosition, sf::Vector2i tileCoords);
	void renderTileHoverPreview(sf::RenderWindow& window, sf::Vector2i tileCoords);
	void renderTilePalette(sf::RenderWindow& window);
	std::vector<Tile::Type> palette;
	unsigned selectedTileIndex;

	void handleTogglesInput();

	void handleModeSwitchInput();
	enum class Mode
	{
		NONE,
		TILES,
		OBJECTS,
		ENEMIES,
		ITEMS
	};
	Mode mode;

	void handleUndoRedoInput();
	void handleUndoRedoUpdate(float fixedTimeStep);
	void undo();
	void redo();
	const float UNDO_REDO_INITIAL_DELAY = 0.2f;
	const float UNDO_REDO_INTERVAL = 0.033f;
	float undoRedoTimer;
	std::stack<std::unique_ptr<Action>> undoStack;
	std::stack<std::unique_ptr<Action>> redoStack;

	void handleCameraMoveInput();
	void handleCameraPanInput(sf::Vector2f mouseWorldPosition);
	void handleCameraUpdate(float fixedTimeStep);
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

	sf::View uiView;

	float mouseWheelDelta;
	sf::Vector2f mouseWorldPosition;
};

struct Action
{
	virtual ~Action() = default;
	virtual void undo(TileMap& map) = 0;
	virtual void redo(TileMap& map) = 0;
};

struct TileAction : public Action
{
	sf::Vector2i coords;
	Tile::Type oldType;
	Tile::Type newType;

	TileAction(sf::Vector2i coords, Tile::Type oldType, Tile::Type newType)
		: coords(coords), oldType(oldType), newType(newType)
	{}
	void undo(TileMap& map) override { map.setTile(coords.x, coords.y, Tile{ oldType }); }
	void redo(TileMap& map) override { map.setTile(coords.x, coords.y, Tile{ newType }); }
};