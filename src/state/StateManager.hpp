// ================================================================================================
// File: StateManager.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// Description: Defines the StateManager class, which manages game states and handles state transitions,
//              as well as processing input, updating, and rendering the current state and any states
//              below it that require it. For example, transparent states also render the state
//              immediately below them. Likewise, transcendent states update the state immediately
//              below them. This behaviour can be chained so that multiple states can be rendered or
//              updated at the same time.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <memory>
#include <vector>
#include "State.hpp"

class StateManager
{
public:
	void push(std::unique_ptr<State> state);
	void pop();
	void clear();
	void change(std::unique_ptr<State> state);

	inline State* top() const {	return states.empty() ? nullptr : states.back().get(); }

	void processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events);
	void update(float fixedTimeStep);
	void render(sf::RenderWindow& window, float interpolationFactor, float fixedTimeStep);

private:
	std::vector<std::unique_ptr<State>> states;
};