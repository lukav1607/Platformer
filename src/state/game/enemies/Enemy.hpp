// ================================================================================================
// File: Enemy.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 18, 2025
// Description: Defines the Enemy class, which represents an enemy character in the game. The Enemy
//              class is responsible for handling enemy behavior, movement, and interactions with the game world.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

// STL
#include <vector>
// SFML
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Font.hpp>
// Game
#include "../../../core/Constants.hpp"
#include "../../../core/Utility.hpp"
#include "../../../core/Position.hpp"
#include "../../../core/Serializable.hpp"
#include "../../../world/TileMap.hpp"

class Player;

namespace lv
{
    class Enemy : public Serializable {
    public:
        enum class State
        {
            Patrolling,
            Chasing,
            Returning
        };

		Enemy();
        virtual ~Enemy() = default;
        virtual std::unique_ptr<Enemy> clone() const = 0;

        virtual void update(float fixedTimeStep, const TileMap& tileMap, const Player& player) = 0;
        virtual void render(sf::RenderTarget& target, const sf::Font& font, float interpolationFactor) = 0;

		// ---- Serialization ----
        void serialize(json& j) const override;
        void deserialize(const json& j) override;
        std::string getType() const override { return "Undefined Enemy"; }

        // ---- Combat ----
        ///void takeHit(float damage, sf::Vector2f knockback);
        bool isAlive() const { return health > 0; }

        // ---- LOS ----
        virtual sf::Vector2f getEyePosition() const = 0;
        virtual bool canSeePlayer(const Player& player) const;

        // ---- Patrolling ----
        virtual bool isValidPatrolPosition(const TileMap& tileMap, sf::Vector2i tilePosition) const = 0;
        void addPatrolPosition(sf::Vector2i tilePosition);
        void removePatrolPosition(sf::Vector2i tilePosition);
        void clearPatrolPositions() { patrolPositions.clear(); }
        void renderPatrolPositions(sf::RenderTarget& target, const sf::Font& font);
        const std::vector<sf::Vector2i>& getPatrolPositions() const { return patrolPositions; }
        sf::Vector2i getCurrentPatrolTarget() const;
        size_t getNextPatrolIndex() const;
        void targetNextPatrolPosition();
        void markAsComplete() { isCompleted = true;  }

        // ---- Jumping ----
        ///virtual bool canJumpGap(float gapWidth, float gapHeight);
        ///virtual bool canClearCeilingDuringJump();

        // ---- Utilities ----
		void syncPosition() { position.sync(); }
        //  Returns the tile the Enemy is currently considered to be in for purposes of pathfinding
        sf::Vector2i getTilePosition() const { return Utility::worldToTileCoords(getNavigationPosition()); }
        // Returns the pixel position of the Enemy's bounds top-left corner
        sf::Vector2f getPixelPosition() const { return position.get(); }
        // Returns the Enemy's bounding rectangle
		sf::FloatRect getBounds() const { return { position.get(), size }; }

        // ---- Debug ----
        void toggleSelected() { isSelected = !isSelected; }
        void setSelected(bool selected) { isSelected = selected; }
        void updateDebugVisuals(const TileMap& tileMap, sf::FloatRect playerBounds);
        void renderDebugVisuals(sf::RenderTarget& target, const sf::Font& font, float interpolationFactor);

    protected:
        // ---- Combat and State ----
		State state;
        int health;
        float aggroRange;
        float followRange;

        // ---- Size and Visuals ----
        sf::RectangleShape bounds;
        sf::Vector2f size;
        sf::Color color;

        // ---- Position and Movement ----
		virtual void updateMovement(const TileMap& tileMap, const Player& player, float fixedTimeStep);
        virtual void handlePatrolling(const TileMap& tileMap, const Player& player, float fixedTimeStep);
        virtual void handleChasing(const TileMap& tileMap, const Player& player, float fixedTimeStep);
        virtual void handleReturning(const TileMap& tileMap, float fixedTimeStep);
        virtual void moveTowards(sf::Vector2f target, float fixedTimeStep) = 0;
        void resolveCollisions(float fixedTimeStep, const TileMap& tileMap);
        void setPosition(sf::Vector2i tilePosition);

        lv::Position position;
        sf::Vector2f velocity;
        float chaseSpeed;
        float patrolSpeed;

        // ---- Jumping ----
        ///virtual void jump();
        ///virtual bool isGrounded() const;

        bool isOnGround;
        float jumpForce;
        float maxJumpHeight;
        float maxJumpDistance;

        // ---- Line of Sight ----
		float timeSinceGainedLOS; // Time passed since the enemy first established LOS with the player
		float timeSinceLostLOS; // Time passed since the enemy last had LOS with the player
		const float LOS_GAINED_THRESHOLD = 0.3f; // Time threshold to consider LOS gained, in seconds
		const float LOS_LOST_THRESHOLD = 10.0f; // Time threshold to consider LOS lost, in seconds

        // ---- Patrolling ----
        std::vector<sf::Vector2i> patrolPositions;
		std::size_t currentPatrolIndex = 0;
        sf::Vector2f positionBeforeAggro;
        bool isCompleted;

        // ---- Pathfinding ----
        ///virtual bool requiresPathfinding() const = 0;
        virtual void recalculatePath(const TileMap& tileMap, sf::Vector2i target);
        virtual void followPath(float fixedTimeStep);
        // Returns the pixel position of the Enemy used for pathfinding,
        //  e.g. the center of the bounds for flying enemies,
		//  or the bottom center (feet) for walking enemies.
        // This version of the function returns the position in global
        //  coordinates, meaning it is relative to the Enemy's actual
		//  position in the world. For local coordinates, i.e. just
		//  the raw offset used, see `getNavigationPositionLocal()`.
		virtual sf::Vector2f getNavigationPosition() const;
		// Returns the local position offset used for pathfinding,
		//  e.g. the center of the bounds for flying enemies,
		//  or the bottom center (feet) for walking enemies.
        // This version of the function returns the local position,
        //  meaning it does not account for the Enemy's actual
        //  position in the world, it only returns the offset used.
        virtual sf::Vector2f getNavigationPositionLocal() const = 0;
        // Returns the actual pixel target an enemy should reach and
        //  align its navigation position with for the given target tile,
		//  e.g. the center of the tile for flying enemies,
		//  or the bottom center of a tile for walking enemies.
        virtual sf::Vector2f getPathTargetPosition(sf::Vector2i targetTile) const = 0;

		const float PATH_TOLERANCE = 5.f; // Tolerance in pixels for pathfinding to consider the enemy at the target tile
        std::vector<sf::Vector2i> path;
        std::size_t currentPathIndex = 0;
		float timeSinceLastPathUpdate; // Time since the last pathfinding update - use lv::Constants::PATHFINDING_UPDATE_INTERVAL to limit updates

		// ---- Debug ----
        void initializeDebugVisuals();

        bool isSelected;
        bool isBeingEdited;
        sf::CircleShape d_patrolTargetCircle;
        sf::CircleShape d_aggroRangeCircle;
        sf::CircleShape d_positionBeforeAggroCircle;
        sf::CircleShape d_followRangeCircle;
        sf::VertexArray d_lineOfSightLine;
    };
}