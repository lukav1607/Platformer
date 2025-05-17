// ================================================================================================
// File: StateManager.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 11, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <iostream>
#include "StateManager.hpp"

void StateManager::push(std::unique_ptr<State> state)
{
	if (state)
		states.push_back(std::move(state));
	else
		std::cerr << "Error: Attempted to push a null state!" << std::endl;
}

void StateManager::pop()
{
	if (!states.empty())
		states.pop_back();
	else
		std::cerr << "Error: No states to pop!" << std::endl;
}

void StateManager::clear()
{
	states.clear();
}

void StateManager::change(std::unique_ptr<State> state)
{
	states.clear();

	if (state)
		states.push_back(std::move(state));
	else
		std::cerr << "Error: Attempted to change to a null state!" << std::endl;
}

void StateManager::processInput(const sf::RenderWindow& window, const std::vector<sf::Event>& events)
{
	if (!states.empty())
		states.back()->processInput(window, events);
	else
		std::cerr << "Error: No states to process input!" << std::endl;
}

void StateManager::update(float fixedTimeStep)
{
	if (!states.empty())
	{
		for (auto it = states.rbegin(); it != states.rend(); ++it)
		{			
			(*it)->update(fixedTimeStep);
			if (!(*it)->isTranscendent())
				break; // Stop updating if the state is not transcendent
		}
	}
	else
		std::cerr << "Error: No states to update!" << std::endl;
}

void StateManager::render(sf::RenderWindow& window, float interpolationFactor, float fixedTimeStep)
{
	// Make sure there is at least one state to render
	if (states.empty())
		std::cerr << "Error: No states to render!" << std::endl;

	unsigned stateCounter = 0;

	// Loop through states in reverse order to find the first non-transparent state.
	// Since we made sure at least one state exists, we can safely start from the last one
	// and assume it definitely needs to be rendered. Therefore we increase stateRenderCounter by one.
	// After that we check if the state we just added to the counter is transparent. If it is, and if
	// there are states before it, we move to the previous state index and add it to the counter as well.
	// This loops until we find a non-transparent state or reach the beginning of the vector.
	// This way we find the number (from the back/top) of all transparent states on top of the first
	// non-transparent state, plus that state itself.
	for (int i = int(states.size()) - 1; i >= 0; --i)
	{
		++stateCounter;
		if (!states.at(i)->isTransparent())
			break;
	}

	// Render states from first/bottom to last/top. The index of the first state to be rendered is
	// the size of the vector minus the number of transparent states on top of the first non-transparent
	// state plus that state. The last state to be rendered is then the last state in the vector.
	for (int i = int(states.size()) - stateCounter; i < int(states.size()); ++i)
	{
		auto& s = *states.at(i);
		s.render(window, interpolationFactor, fixedTimeStep);
	}
}