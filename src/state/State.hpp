// ================================================================================================
// File: State.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// Description: Defines the State class, which represents an abstract game state for the game engine.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <SFML/Graphics/RenderWindow.hpp>

class StateManager;

class State
{
public:
	virtual ~State() = default;

	virtual void processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events) = 0;
	virtual void update(float fixedTimeStep) = 0;
	virtual void render(sf::RenderWindow& window, float interpolationFactor) = 0;

	virtual void applyView(sf::RenderWindow& window) = 0;

	virtual bool isTransparent() const { return false; }   // render state(s) underneath
	virtual bool isTranscendent() const { return false; }  // update state(s) underneath

protected:
	State(StateManager& stateManager) : stateManager(stateManager) {}
	StateManager& stateManager;
};