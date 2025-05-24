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
#include <SFML/Graphics/Text.hpp>
#include "../State.hpp"
#include "../game/PlayState.hpp"
#include "../../world/TileMap.hpp"
#include "EditorCamera.hpp"

struct Action;

class EditorState : public State
{
public:
	EditorState(StateManager& stateManager, PlayState& playState, TileMap& map, Player& player, std::vector<Enemy>& enemies, sf::Font& font);

	void processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events) override;
	void update(float fixedTimeStep) override;
	void render(sf::RenderWindow& window, float interpolationFactor) override;

	void applyView(sf::RenderWindow& window) override;

	bool isTranscendent() const override { return false; } // Do not update state(s) underneath
	bool isTransparent() const override { return false; } // Do not render state(s) underneath - NOTE: Editor is a special case where it still
														  // renders the PlayState from it's render function despite this flag.

private:
	sf::Font& font;
	TileMap& map;
	PlayState& playState;
	Player& player;
	std::vector<Enemy>& enemies;
	static EditorCamera camera;

	void handleSaveLoadInput();
	void renderSaveLoadText(sf::RenderWindow& window);
	const float SAVE_LOAD_TEXT_LIFETIME = 1.5f;
	float saveTextTimer;
	float loadTextTimer;
	sf::Text mapSavedText;
	sf::Text mapLoadedText;

	void handlePlayerPlacement(sf::Vector2i mouseWindowPosition, sf::Vector2i tileCoords);
	void renderPlayerPreview(sf::RenderWindow& window, sf::Vector2i tileCoords);
	void renderPlayerModes(sf::RenderWindow& window);
	enum class PlayerMode
	{
		SET_SPAWN,
		MOVE_TO
	};
	std::vector<PlayerMode> playerModes;
	unsigned selectedPlayerModeIndex;
	sf::Text playerPlacementText;
	sf::RectangleShape playerSpawnShape;

	void handleEnemyPlacement(sf::Vector2i mouseWindowPosition, sf::Vector2i tileCoords);
	void renderEnemyPreview(sf::RenderWindow& window, sf::Vector2i tileCoords);
	void renderEnemyPalette(sf::RenderWindow& window);
	std::vector<Enemy::Type> enemyPalette;
	unsigned selectedEnemyIndex;
	sf::Text enemyPaletteText;
	Enemy currentEnemy;
	//std::vector<sf::Vector2i> currentEnemyPatrolPositions;

	void rebuildGridLines();
	void renderGrid(sf::RenderWindow& window);
	sf::VertexArray gridLines;
	sf::Color gridColor;
	bool isGridShown;

	void handleTileInput(sf::Vector2i mouseWindowPosition, sf::Vector2i tileCoords);

	void handleSelectionInput(sf::Vector2i tileCoords);
	void applySelectionAction();
	void renderSelectionRect(sf::RenderWindow& window) const;
	void handleTilePreviewRendering(sf::RenderWindow& window, sf::Vector2i tileCoords);
	void renderTilePreview(sf::RenderWindow& window, sf::Vector2i tileCoords);
	void renderTilePalette(sf::RenderWindow& window);
	sf::Text tilePaletteText;
	std::vector<Tile::Type> palette;
	unsigned selectedTileIndex;
	bool isDrawingSelection;
	sf::Vector2i selectionStart;
	sf::Vector2i selectionEnd;
	enum class SelectionAction
	{
		NONE,
		PLACE,
		ERASE_SPECIFIC,
		ERASE_ALL
	};
	SelectionAction selectionAction;

	void handleTogglesInput();

	void handleModeSwitchInput();
	enum class Mode
	{
		//NONE,
		TILES,
		OBJECTS,
		PLAYER,
		ENEMIES,
		ITEMS,
		COUNT
	};
	Mode mode;
	bool isErasing;

	void handleUndoRedoInput();
	void handleUndoRedoUpdate(float fixedTimeStep);
	void undo();
	void redo();
	const float UNDO_REDO_INITIAL_DELAY = 0.2f;
	const float UNDO_REDO_INTERVAL = 0.033f;
	float undoRedoTimer;
	std::stack<std::unique_ptr<Action>> undoStack;
	std::stack<std::unique_ptr<Action>> redoStack;

	//std::vector<std::unique_ptr<Action>> batch;

	sf::Vector2f toolOffset;

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
	inline void undo(TileMap& map) override { map.setTile(coords.x, coords.y, Tile{ oldType }); }
	inline void redo(TileMap& map) override { map.setTile(coords.x, coords.y, Tile{ newType }); }
};

struct BatchAction : public Action
{
	std::vector<std::unique_ptr<Action>> actions;

	BatchAction(std::vector<std::unique_ptr<Action>>&& actions)
		: actions(std::move(actions))
	{}
	inline void undo(TileMap& map) override
	{
		for (auto it = actions.rbegin(); it != actions.rend(); ++it)
			(*it)->undo(map);
	}
	inline void redo(TileMap& map) override
	{
		for (auto& action : actions)
			action->redo(map);
	}
};