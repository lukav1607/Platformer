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

void StateManager::render(sf::RenderWindow& window, float interpolationFactor)
{
	if (!states.empty())
	{
		for (auto it = states.rbegin(); it != states.rend(); ++it)
		{
			(*it)->render(window, interpolationFactor);
			if (!(*it)->isTransparent())
				break; // Stop rendering if the state is not transparent
		}
	}
	else
		std::cerr << "Error: No states to render!" << std::endl;
}
