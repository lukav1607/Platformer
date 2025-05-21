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
#include "../StateManager.hpp"
#include "../../core/Utility.hpp"

EditorCamera EditorState::camera;

EditorState::EditorState(StateManager& stateManager, PlayState& playState, TileMap& map, Player& player, std::vector<Enemy>& enemies, sf::Font& font) :
	State(stateManager),
	playState(playState),
	map(map),
	player(player),
	enemies(enemies),
	font(font),
	saveTextTimer(0.f),
	loadTextTimer(0.f),
	playerModes{
		PlayerMode::MOVE_TO,
		PlayerMode::SET_SPAWN
	},
	enemyPalette{
		Enemies::ENEMY_1
	},
	palette{
		Tile::Type::Background,
		Tile::Type::Solid,
		Tile::Type::Water,
		Tile::Type::Door
	},
	selectedTileIndex(0U),
	selectedPlayerModeIndex(0U),
	selectedEnemyIndex(0U),
	isDrawingSelection(false),
	selectionAction(SelectionAction::NONE),
	gridLines(sf::PrimitiveType::Lines),
	gridColor(sf::Color(255, 255, 255, 64)),
	isGridShown(true),
	mode(Mode::TILES),
	isErasing(false),
	toolOffset(32.f, 64.f),
	undoRedoTimer(UNDO_REDO_INITIAL_DELAY),
	mouseWheelDelta(0.f),
	playerPlacementText(font, "Player", 32U),
	enemyPaletteText(font, "Enemy Palette", 32U),
	tilePaletteText(font, "Tile Palette", 32U),
	mapSavedText(font, "Map saved!", 48U),
	mapLoadedText(font, "Map loaded!", 48U)
{
	rebuildGridLines();

	playerSpawnShape.setSize(player.getSize());
	playerSpawnShape.setFillColor(sf::Color::Transparent);
	playerSpawnShape.setOutlineThickness(4.f);
	playerSpawnShape.setOutlineColor(sf::Color(player.getColor().r - 50.f, player.getColor().g + 200.f, player.getColor().b - 50.f, 255));

	playerPlacementText.setFillColor(sf::Color::White);
	playerPlacementText.setOutlineColor(sf::Color(30, 30, 30, 255));
	playerPlacementText.setOutlineThickness(2.f);
	enemyPaletteText.setFillColor(sf::Color::White);
	enemyPaletteText.setOutlineColor(sf::Color(30, 30, 30, 255));
	enemyPaletteText.setOutlineThickness(2.f);
	tilePaletteText.setFillColor(sf::Color::White);
	tilePaletteText.setOutlineColor(sf::Color(30, 30, 30, 255));
	tilePaletteText.setOutlineThickness(2.f);
	mapSavedText.setFillColor(sf::Color::White);
	mapSavedText.setOutlineColor(sf::Color(30, 30, 30, 255));
	mapSavedText.setOutlineThickness(2.f);
	mapLoadedText.setFillColor(sf::Color::White);
	mapLoadedText.setOutlineColor(sf::Color(30, 30, 30, 255));
	mapLoadedText.setOutlineThickness(2.f);
}

void EditorState::processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events)
{
	// Process relevant window events
	for (const auto& event : events)
	{
		// Apply mouse wheel scroll delta (for camera zoom)
		if (const auto* mouseWheelScrolled = event.getIf<sf::Event::MouseWheelScrolled>())
			mouseWheelDelta += mouseWheelScrolled->delta;
	}	
	
	sf::Vector2i mouseWindowPosition = sf::Mouse::getPosition(window);                   // Get mouse position in window coordinates	
	mouseWorldPosition = window.mapPixelToCoords(mouseWindowPosition, camera.getView()); // Get mouse position in world coordinates relative to the view
	sf::Vector2i tileCoords = Utility::worldToTileCoords(mouseWorldPosition);            // Convert mouse position to tile coordinates

	handleSaveLoadInput();
	handleModeSwitchInput();
	handleTogglesInput();
	handleUndoRedoInput();
	camera.handleInput(mouseWorldPosition);

	// Handle mode-based input
	switch (mode)
	{
	case Mode::TILES:
		handleSelectionInput(tileCoords);
		handleTileInput(sf::Mouse::getPosition(window), tileCoords);
		break;
	case Mode::PLAYER:
		handlePlayerPlacement(sf::Mouse::getPosition(window), tileCoords);
		break;
	default:
		break;
	}
	
	// Exit editor state
	if (Utility::isKeyReleased(sf::Keyboard::Key::F1))
		stateManager.pop();
}

void EditorState::update(float fixedTimeStep)
{
	camera.update(fixedTimeStep, mouseWheelDelta, mouseWorldPosition);
	handleUndoRedoUpdate(fixedTimeStep);	

	saveTextTimer = std::max(0.f, saveTextTimer - fixedTimeStep);
	loadTextTimer = std::max(0.f, loadTextTimer - fixedTimeStep);
	mouseWheelDelta = 0.f;
}

void EditorState::render(sf::RenderWindow& window, float interpolationFactor)
{
	sf::Vector2i tileCoords = Utility::worldToTileCoords(mouseWorldPosition);
	camera.applyInterpolatedPosition(interpolationFactor);

	// Draw in world
	window.setView(camera.getView());
	playState.render(window, interpolationFactor); // <- Handle rendering the PlayState in the background here to
	renderSelectionRect(window);                   //    make sure it's in sync with the EditorState camera.
	handleTilePreviewRendering(window, tileCoords);
	window.draw(playerSpawnShape);
	renderPlayerPreview(window, tileCoords);
	renderGrid(window);	

	// Draw as overlay/UI
	window.setView(uiView);
	renderTilePalette(window);
	renderPlayerModes(window);
	renderEnemyPalette(window);
	renderSaveLoadText(window);

	// Reset the view to the default Editor view (editor camera)
	window.setView(camera.getView());
}

void EditorState::applyView(sf::RenderWindow& window)
{
	// Resize UI view to match the window size
	uiView.setSize({ static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y) });
	uiView.setCenter({ static_cast<float>(window.getSize().x) / 2.f, static_cast<float>(window.getSize().y) / 2.f });

	camera.resize(window.getSize());
}

void EditorState::handleSaveLoadInput()
{
	bool ctrlPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl);
	bool sReleased = Utility::isKeyReleased(sf::Keyboard::Key::S);
	bool lReleased = Utility::isKeyReleased(sf::Keyboard::Key::L);

	if (ctrlPressed && sReleased)
	{
		if (map.saveToJson("assets/maps/test_map.json"))
		{
			std::cout << "Map saved successfully!" << std::endl;
			saveTextTimer = SAVE_LOAD_TEXT_LIFETIME;
		}
	}
	if (ctrlPressed && lReleased)
	{
		if (map.loadFromJson("assets/maps/test_map.json"))
		{
			rebuildGridLines();
			std::cout << "Map loaded successfully!" << std::endl;
			loadTextTimer = SAVE_LOAD_TEXT_LIFETIME;
		}
	}
}

void EditorState::renderSaveLoadText(sf::RenderWindow& window)
{
	if (saveTextTimer > 0.f)
	{
		mapSavedText.setPosition({ uiView.getCenter().x - mapSavedText.getGlobalBounds().size.x / 2.f,
			uiView.getCenter().y - mapSavedText.getGlobalBounds().size.y / 2.f });
		window.draw(mapSavedText);
	}
	if (loadTextTimer > 0.f)
	{
		mapLoadedText.setPosition({ uiView.getCenter().x - mapLoadedText.getGlobalBounds().size.x / 2.f,
			uiView.getCenter().y - mapLoadedText.getGlobalBounds().size.y / 2.f });
		window.draw(mapLoadedText);
	}
}

void EditorState::handlePlayerPlacement(sf::Vector2i mouseWindowPosition, sf::Vector2i tileCoords)
{
	if (mode != Mode::PLAYER)
		return;

	bool isLeftClickReleased = Utility::isButtonReleased(sf::Mouse::Button::Left);

	if (isLeftClickReleased)
	{
		// Tile palette click detection / selection
		for (size_t i = 0; i < playerModes.size(); ++i)
		{
			sf::FloatRect bounds({ toolOffset.x + 10.f + i * 60.f, toolOffset.y + 10.f }, { 50.f, 50.f });
			if (bounds.contains(sf::Vector2f(mouseWindowPosition)))
			{
				selectedPlayerModeIndex = i;
				return;
			}
		}

		if (!map.isWithinBounds(tileCoords) || !map.isWithinBounds(tileCoords + sf::Vector2i(0.f, -1.f)))
			return;
		if (map.getTile(tileCoords).type == Tile::Type::Solid || map.getTile(tileCoords + sf::Vector2i(0.f, -1.f)).type == Tile::Type::Solid)
			return;

		if (selectedPlayerModeIndex == static_cast<unsigned>(PlayerMode::MOVE_TO))
		{
			player.setPosition(tileCoords);
			//batch.push_back(std::make_unique<TileAction>(tileCoords, oldType, newType));
		}
		if (selectedPlayerModeIndex == static_cast<unsigned>(PlayerMode::SET_SPAWN))
		{
			player.spawnPosition = tileCoords;
			playerSpawnShape.setPosition({
				tileCoords.x * TileMap::TILE_SIZE + (TileMap::TILE_SIZE / 2.f - player.getSize().x / 2.f),
				tileCoords.y * TileMap::TILE_SIZE + (TileMap::TILE_SIZE - player.getSize().y) });
			//batch.push_back(std::make_unique<TileAction>(tileCoords, oldType, newType));
		}
	}	
	//if (!batch.empty() && (isLeftClickReleased || isRightClickReleased))
	//{
	//	// Push the batch of actions to the undo stack and clear the redo stack
	//	undoStack.push(std::make_unique<BatchAction>(std::move(batch)));
	//	redoStack = std::stack<std::unique_ptr<Action>>();
	//}
}

void EditorState::renderPlayerPreview(sf::RenderWindow& window, sf::Vector2i tileCoords)
{
	if (mode != Mode::PLAYER)
		return;

	if (!map.isWithinBounds(tileCoords) || !map.isWithinBounds(tileCoords + sf::Vector2i(0.f, -1.f)))
		return;

	if (map.getTile(tileCoords).type == Tile::Type::Solid || map.getTile(tileCoords + sf::Vector2i(0.f, -1.f)).type == Tile::Type::Solid)
		return;

	if (selectedPlayerModeIndex == static_cast<unsigned>(PlayerMode::SET_SPAWN))
	{
		sf::RectangleShape playerPreview(player.getSize());
		playerPreview.setPosition({
			tileCoords.x * TileMap::TILE_SIZE + (TileMap::TILE_SIZE / 2.f - player.getSize().x / 2.f),
			tileCoords.y * TileMap::TILE_SIZE + (TileMap::TILE_SIZE - player.getSize().y) });
		playerPreview.setFillColor(sf::Color::Transparent);
		playerPreview.setOutlineThickness(4.f);
		playerPreview.setOutlineColor(sf::Color(player.getColor().r - 50.f, player.getColor().g + 200.f, player.getColor().b - 50.f, 128));
		window.draw(playerPreview);
	}
	else if (selectedPlayerModeIndex == static_cast<unsigned>(PlayerMode::MOVE_TO))
	{
		sf::RectangleShape playerPreview(player.getSize());
		playerPreview.setPosition({
			tileCoords.x * TileMap::TILE_SIZE + (TileMap::TILE_SIZE / 2.f - player.getSize().x / 2.f),
			tileCoords.y * TileMap::TILE_SIZE + (TileMap::TILE_SIZE - player.getSize().y) });
		playerPreview.setFillColor(sf::Color(player.getColor().r, player.getColor().g, player.getColor().b, 128));
		window.draw(playerPreview);
	}
}

void EditorState::renderPlayerModes(sf::RenderWindow& window)
{
	if (mode != Mode::PLAYER)
		return;

	for (size_t i = 0; i < playerModes.size(); ++i)
	{
		sf::RectangleShape shape(sf::Vector2f(50.f, 50.f));
		shape.setPosition({ toolOffset.x + i * 60.f, toolOffset.y });
		shape.setFillColor(selectedPlayerModeIndex == 0U ? player.getColor() : player.getColor() + sf::Color(10, 10, 10));

		if (i == selectedPlayerModeIndex)
		{
			shape.setOutlineThickness(2.f);
			shape.setOutlineColor(sf::Color::White);
		}
		window.draw(shape);
	}

	playerPlacementText.setOrigin({ playerPlacementText.getGlobalBounds().size.x / 2.f, playerPlacementText.getGlobalBounds().size.y / 2.f });
	playerPlacementText.setPosition({ toolOffset.x + 55.f/*(palette.size() * 60.f) / 2.f - 10.f*/, playerPlacementText.getGlobalBounds().size.y });
	window.draw(playerPlacementText);
}

void EditorState::handleEnemyPlacement(sf::Vector2i mouseWindowPosition, sf::Vector2i tileCoords)
{
	if (mode != Mode::ENEMIES)
		return;

	bool isLeftClickReleased = Utility::isButtonReleased(sf::Mouse::Button::Left);

	if (isLeftClickReleased)
	{
		// Tile palette click detection / selection
		for (size_t i = 0; i < enemyPalette.size(); ++i)
		{
			sf::FloatRect bounds({ toolOffset.x + 10.f + i * 60.f, toolOffset.y + 10.f }, { 50.f, 50.f });
			if (bounds.contains(sf::Vector2f(mouseWindowPosition)))
			{
				selectedEnemyIndex = i;
				return;
			}
		}

		if (!map.isWithinBounds(tileCoords)/* || !map.isWithinBounds(tileCoords + sf::Vector2i(0.f, -1.f))*/)
			return;
		if (map.getTile(tileCoords).type == Tile::Type::Solid/* || map.getTile(tileCoords + sf::Vector2i(0.f, -1.f)).type == Tile::Type::Solid*/)
			return;

		if (selectedEnemyIndex == static_cast<unsigned>(Enemies::ENEMY_1))
		{

			//batch.push_back(std::make_unique<TileAction>(tileCoords, oldType, newType));
		}
		//if (selectedPlayerModeIndex == static_cast<unsigned>(PlayerMode::SET_SPAWN))
		//{
		//	player.spawnPosition = tileCoords;
		//	playerSpawnShape.setPosition({
		//		tileCoords.x * TileMap::TILE_SIZE + (TileMap::TILE_SIZE / 2.f - player.getSize().x / 2.f),
		//		tileCoords.y * TileMap::TILE_SIZE + (TileMap::TILE_SIZE - player.getSize().y) });
		//	//batch.push_back(std::make_unique<TileAction>(tileCoords, oldType, newType));
		//}
	}
}

void EditorState::renderEnemyPreview(sf::RenderWindow& window, sf::Vector2i tileCoords)
{
	if (mode != Mode::ENEMIES)
		return;

	if (!map.isWithinBounds(tileCoords) ||
		!map.isWithinBounds(tileCoords + sf::Vector2i(0.f, -1.f)))
		return;

	/*if (map.getTile(tileCoords).type != Tile::Type::Solid &&
		map.getTile(tileCoords + sf::Vector2i(0.f, -1.f)).type != Tile::Type::Solid)
	{
		sf::RectangleShape playerPreview(player.getSize());
		playerPreview.setPosition({
			tileCoords.x * TileMap::TILE_SIZE + (TileMap::TILE_SIZE / 2.f - player.getSize().x / 2.f),
			tileCoords.y * TileMap::TILE_SIZE + (TileMap::TILE_SIZE - player.getSize().y) });
		playerPreview.setFillColor(sf::Color(player.getColor().r, player.getColor().g, player.getColor().b, 128));
		window.draw(playerPreview);
	}*/
}

void EditorState::renderEnemyPalette(sf::RenderWindow& window)
{
	if (mode != Mode::ENEMIES)
		return;

	enemyPaletteText.setOrigin({ enemyPaletteText.getGlobalBounds().size.x / 2.f, enemyPaletteText.getGlobalBounds().size.y / 2.f });
	enemyPaletteText.setPosition({ toolOffset.x + 110.f/*(enemies.size() * 60.f) / 2.f - 10.f*/, enemyPaletteText.getGlobalBounds().size.y });
	window.draw(enemyPaletteText);
}

void EditorState::rebuildGridLines()
{
	gridLines.clear();

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

void EditorState::renderGrid(sf::RenderWindow& window)
{
	if (!isGridShown)
		return;
	
	sf::RectangleShape border(sf::Vector2f(map.getSize().x * TileMap::TILE_SIZE, map.getSize().y * TileMap::TILE_SIZE));
	border.setFillColor(sf::Color::Transparent);
	border.setOutlineThickness(2.f);
	border.setOutlineColor(sf::Color(255, 255, 255, 128));
	window.draw(border);
	window.draw(gridLines);
}

void EditorState::handleTileInput(sf::Vector2i mouseWindowPosition, sf::Vector2i tileCoords)
{
	if (isDrawingSelection)
		return;

	static std::vector<std::unique_ptr<Action>> batch;

	bool isLeftClickHeld = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
	bool isRightClickHeld = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
	bool isLeftClickReleased = Utility::isButtonReleased(sf::Mouse::Button::Left);
	bool isRightClickReleased = Utility::isButtonReleased(sf::Mouse::Button::Right);
	bool hasPaletteBeenClicked = false;

	if (isLeftClickHeld)
	{
		// Tile palette click detection / selection
		for (size_t i = 0; i < palette.size(); ++i)
		{
			sf::FloatRect bounds({ toolOffset.x + 10.f + i * 60.f, toolOffset.y + 10.f }, { 50.f, 50.f });
			if (bounds.contains(sf::Vector2f(mouseWindowPosition)))
			{
				selectedTileIndex = i;
				hasPaletteBeenClicked = true;
				break;
			}
		}
	}
	// Tile placement
	if (!hasPaletteBeenClicked &&
		map.isWithinBounds(tileCoords))
	{
		Tile::Type oldType = map.getTile(tileCoords).type;
		Tile::Type newType = isLeftClickHeld && !isErasing ? palette.at(selectedTileIndex) : Tile::Type::EMPTY;

		if (oldType != newType)
		{
			if (isLeftClickHeld && !isErasing)
				map.setTile(tileCoords.x, tileCoords.y, Tile{ palette.at(selectedTileIndex) });

			else if (isLeftClickHeld && isErasing)
				map.setTile(tileCoords.x, tileCoords.y, Tile{ Tile::Type::EMPTY });

			if (isLeftClickHeld || isRightClickHeld)
				batch.push_back(std::make_unique<TileAction>(tileCoords, oldType, newType));
		}
	}
	if (!batch.empty() && (isLeftClickReleased || isRightClickReleased))
	{
		// Push the batch of actions to the undo stack and clear the redo stack
		undoStack.push(std::make_unique<BatchAction>(std::move(batch)));
		redoStack = std::stack<std::unique_ptr<Action>>();
	}
}

void EditorState::handleSelectionInput(sf::Vector2i tileCoords)
{
	if (mode == Mode::PLAYER ||	mode == Mode::ENEMIES || mode == Mode::ITEMS)
		return;

	bool isLeftClickHeld = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
	bool isRightClickHeld = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
	bool isCtrlHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl);
	bool isShiftHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift);

	if (!isDrawingSelection)
	{
		if (isShiftHeld && isLeftClickHeld)
		{
			isDrawingSelection = true;
			selectionStart = tileCoords;
			selectionAction = SelectionAction::PLACE;
			if (isErasing)
			{
				if (isCtrlHeld)
					selectionAction = SelectionAction::ERASE_ALL;
				else
					selectionAction = SelectionAction::ERASE_SPECIFIC;
			}
		}
	}

	if (isDrawingSelection)
	{
		selectionEnd = tileCoords;

		if (!(isLeftClickHeld || isRightClickHeld))
		{
			applySelectionAction();
			isDrawingSelection = false;
			selectionAction = SelectionAction::NONE;
		}
	}
}

void EditorState::applySelectionAction()
{
	sf::Vector2i topLeft = { std::min(selectionStart.x, selectionEnd.x), std::min(selectionStart.y, selectionEnd.y) };
	sf::Vector2i bottomRight = { std::max(selectionStart.x, selectionEnd.x), std::max(selectionStart.y, selectionEnd.y) };

	std::vector<std::unique_ptr<Action>> batch;

	Tile::Type newType = palette.at(selectedTileIndex);

	for (int y = topLeft.y; y <= bottomRight.y; ++y)
	{
		for (int x = topLeft.x; x <= bottomRight.x; ++x)
		{
			if (!map.isWithinBounds({ x, y }))
				continue;

			Tile::Type oldType = map.getTile({ x, y }).type;

			if (selectionAction == SelectionAction::PLACE)
			{
				switch (mode)
				{
				case Mode::TILES:
					if (oldType == newType)
						continue;
					batch.push_back(std::make_unique<TileAction>(sf::Vector2i(x, y), oldType, newType));
					map.setTile(x, y, Tile{ newType }, false);
					break;
				default:
					break;
				}
			}
			else if (selectionAction == SelectionAction::ERASE_SPECIFIC)
			{
				switch (mode)
				{
				case Mode::TILES:
					batch.push_back(std::make_unique<TileAction>(sf::Vector2i(x, y), oldType, Tile::Type::EMPTY));
					map.setTile(x, y, Tile{ Tile::Type::EMPTY }, false);
					break;
				default:
					break;
				}
			}
			else if (selectionAction == SelectionAction::ERASE_ALL)
			{
				batch.push_back(std::make_unique<TileAction>(sf::Vector2i(x, y), oldType, Tile::Type::EMPTY));
				map.setTile(x, y, Tile{ Tile::Type::EMPTY }, false);
			}
		}
	}
	map.rebuildVisuals();

	if (!batch.empty())
	{
		// Push the batch of actions to the undo stack and clear the redo stack
		undoStack.push(std::make_unique<BatchAction>(std::move(batch)));
		redoStack = std::stack<std::unique_ptr<Action>>();
	}
}

void EditorState::renderSelectionRect(sf::RenderWindow& window) const
{
	if (!isDrawingSelection)
		return;

	sf::Vector2f selectionStartPos = Utility::tileToWorldCoords(selectionStart);
	sf::Vector2f selectionEndPos = Utility::tileToWorldCoords(selectionEnd);

	sf::Vector2f topLeft = { std::min(selectionStartPos.x, selectionEndPos.x), std::min(selectionStartPos.y, selectionEndPos.y) };
	sf::Vector2f bottomRight = { std::max(selectionStartPos.x, selectionEndPos.x), std::max(selectionStartPos.y, selectionEndPos.y) };
	sf::Vector2f size = { bottomRight.x - topLeft.x + TileMap::TILE_SIZE, bottomRight.y - topLeft.y + TileMap::TILE_SIZE};

	sf::RectangleShape outline(size);
	outline.setPosition(topLeft);
	outline.setFillColor(sf::Color::Transparent);
	outline.setOutlineThickness(2.f);

	if (!isErasing)
		outline.setOutlineColor(sf::Color::White);
	else
	{
		outline.setOutlineColor(sf::Color::Red);
		sf::VertexArray crossedLines(sf::PrimitiveType::Lines, 4);
		crossedLines[0].position = { topLeft.x, topLeft.y };
		crossedLines[1].position = { bottomRight.x + TileMap::TILE_SIZE, bottomRight.y + TileMap::TILE_SIZE};
		crossedLines[2].position = { bottomRight.x + TileMap::TILE_SIZE, topLeft.y };
		crossedLines[3].position = { topLeft.x, bottomRight.y + TileMap::TILE_SIZE};
		for (int i = 0; i < crossedLines.getVertexCount(); ++i)
			crossedLines[i].color = sf::Color::Red;
		window.draw(crossedLines);
	}
	window.draw(outline);
}

void EditorState::handleTilePreviewRendering(sf::RenderWindow& window, sf::Vector2i tileCoords)
{
	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) ||
		selectionAction == SelectionAction::ERASE_SPECIFIC ||
		selectionAction == SelectionAction::ERASE_ALL)
		return;

	if (isDrawingSelection)
	{
		sf::Vector2i topLeft = { std::min(selectionStart.x, selectionEnd.x), std::min(selectionStart.y, selectionEnd.y) };
		sf::Vector2i bottomRight = { std::max(selectionStart.x, selectionEnd.x), std::max(selectionStart.y, selectionEnd.y) };

		for (int y = topLeft.y; y <= bottomRight.y; ++y)
			for (int x = topLeft.x; x <= bottomRight.x; ++x)
				renderTilePreview(window, { x, y });
	}
	else
	{
		renderTilePreview(window, tileCoords);
	}
}

void EditorState::renderTilePreview(sf::RenderWindow& window, sf::Vector2i tileCoords)
{
	if (mode != Mode::TILES)
		return;

	if (map.isWithinBounds(tileCoords) && (map.getTile(tileCoords).type != palette.at(selectedTileIndex) || isErasing))
	{
		sf::RectangleShape preview(sf::Vector2f(TileMap::TILE_SIZE, TileMap::TILE_SIZE));
		preview.setPosition({ static_cast<float>(tileCoords.x * TileMap::TILE_SIZE), static_cast<float>(tileCoords.y * TileMap::TILE_SIZE) });
		preview.setFillColor([&]
			{
				if (isErasing)
					return sf::Color(255, 0, 0, 30);
				sf::Color color = map.getTileColor(palette.at(selectedTileIndex));
				color.a = 128;
				return color;
			}());
		preview.setOutlineColor([&]
			{
				if (isErasing)
					return sf::Color::Red;
				return sf::Color::Transparent;
			}());
		preview.setOutlineThickness(2.f);
		if (isErasing)
		{
			sf::VertexArray crossedLines(sf::PrimitiveType::Lines, 4);
			crossedLines[0].position = { tileCoords.x * TileMap::TILE_SIZE, tileCoords.y * TileMap::TILE_SIZE };
			crossedLines[1].position = { tileCoords.x * TileMap::TILE_SIZE + TileMap::TILE_SIZE, tileCoords.y * TileMap::TILE_SIZE + TileMap::TILE_SIZE };
			crossedLines[2].position = { tileCoords.x * TileMap::TILE_SIZE + TileMap::TILE_SIZE, tileCoords.y * TileMap::TILE_SIZE };
			crossedLines[3].position = { tileCoords.x * TileMap::TILE_SIZE, tileCoords.y * TileMap::TILE_SIZE + TileMap::TILE_SIZE };
			for (int i = 0; i < crossedLines.getVertexCount(); ++i)
				crossedLines[i].color = sf::Color::Red;
			window.draw(crossedLines);
		}
		window.draw(preview);
	}
}

void EditorState::renderTilePalette(sf::RenderWindow& window)
{
	if (mode != Mode::TILES)
		return;

	for (size_t i = 0; i < palette.size(); ++i)
	{
		sf::RectangleShape shape(sf::Vector2f(50.f, 50.f));
		shape.setPosition({ toolOffset.x + i * 60.f, toolOffset.y });
		shape.setFillColor(map.getTileColor(palette.at(i)));

		if (i == selectedTileIndex)
		{
			shape.setOutlineThickness(2.f);
			shape.setOutlineColor(sf::Color::White);
		}
		window.draw(shape);
	}
	tilePaletteText.setOrigin({ tilePaletteText.getGlobalBounds().size.x / 2.f, tilePaletteText.getGlobalBounds().size.y / 2.f });
	tilePaletteText.setPosition({ toolOffset.x + (palette.size() * 60.f) / 2.f - 10.f, tilePaletteText.getGlobalBounds().size.y});
	window.draw(tilePaletteText);
}

void EditorState::handleTogglesInput()
{
	if (Utility::isKeyReleased(sf::Keyboard::Key::G))
		isGridShown = !isGridShown;
}

void EditorState::handleModeSwitchInput()
{
	if (Utility::isKeyReleased(sf::Keyboard::Key::M))
	{
		mode = static_cast<Mode>(static_cast<int>(mode) + 1);
		if (mode == Mode::COUNT)
			mode = Mode::TILES;
	}

	if (Utility::isKeyReleased(sf::Keyboard::Key::E))
		isErasing = !isErasing;
}

void EditorState::handleUndoRedoInput()
{
	static bool isUndoOrRedoPressed = false;
	bool wasUndoOrRedoPressedLastFrame;

	bool ctrlPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl);
	bool zPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z);
	bool yPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Y);
	bool zReleased = Utility::isKeyReleased(sf::Keyboard::Key::Z);
	bool yReleased = Utility::isKeyReleased(sf::Keyboard::Key::Y);

	wasUndoOrRedoPressedLastFrame = isUndoOrRedoPressed;
	isUndoOrRedoPressed = false;

	if (ctrlPressed && (zPressed || yPressed))
	{
		isUndoOrRedoPressed = true;

		if (!wasUndoOrRedoPressedLastFrame)
			undoRedoTimer = UNDO_REDO_INITIAL_DELAY;

		if (undoRedoTimer <= 0.f)
		{
			undoRedoTimer = UNDO_REDO_INTERVAL;

			if (zPressed)
				undo();
			else if (yPressed)
				redo();
		}
	}
	else if (ctrlPressed && zReleased)
	{
		undo();
	}
	else if (ctrlPressed && yReleased)
	{
		redo();
	}
	else
	{
		undoRedoTimer = 0.f;
	}
}

void EditorState::handleUndoRedoUpdate(float fixedTimeStep)
{
	if (undoRedoTimer > 0.f)
		undoRedoTimer -= fixedTimeStep;
	else
		undoRedoTimer = 0.f;
}

void EditorState::undo()
{
	if (undoStack.empty())
		return;

	auto action = std::move(undoStack.top());
	undoStack.pop();
	action->undo(map);
	redoStack.push(std::move(action));	
}

void EditorState::redo()
{
	if (redoStack.empty())
		return;

	auto action = std::move(redoStack.top());
	redoStack.pop();
	action->redo(map);
	undoStack.push(std::move(action));
}