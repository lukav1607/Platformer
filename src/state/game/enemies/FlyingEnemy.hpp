// ================================================================================================
// File: FlyingEnemy.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 28, 2025
// Description: Defines the FlyingEnemy class, which represents a flying enemy character in the game.
//              The FlyingEnemy class inherits from the Enemy class and adds specific behavior and
//              properties for flying enemies.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include "Enemy.hpp"

namespace lv
{
	class FlyingEnemy : public Enemy
	{
	public:
		FlyingEnemy();
		std::unique_ptr<Enemy> clone() const override;

		void update(float fixedTimeStep, const TileMap& tileMap, const Player& player) override;
		void render(sf::RenderTarget& target, const sf::Font& font, float interpolationFactor) override;

		// ---- Serialization ----
		void serialize(json& j) const override;
		void deserialize(const json& j) override;
		std::string getType() const override { return "Flying Enemy"; }

		// ---- LOS ----
		sf::Vector2f getEyePosition() const override;

		// ---- Patrolling ----
		bool isValidPatrolPosition(const TileMap& tileMap, sf::Vector2i tilePosition) const override;

	private:
		void moveTowards(sf::Vector2f target, float fixedTimeStep);

		// ---- Pathfinding ----
		// Returns the pixel position of the Enemy used for pathfinding,
		//  e.g. the center of the bounds for flying enemies,
		//  or the bottom center (feet) for walking enemies.
		///sf::Vector2f getNavigationPosition() const override;

		sf::Vector2f getNavigationPositionLocal() const override;

		// Returns the actual pixel target an enemy should reach and
		//  align its navigation position with for the given target tile,
		//  e.g. the center of the tile for flying enemies,
		//  or the bottom center of a tile for walking enemies.
		sf::Vector2f getPathTargetPosition(sf::Vector2i targetTile) const override;
	};
}