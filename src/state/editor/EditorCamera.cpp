// ================================================================================================
// File: EditorCamera.cpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 15, 2025
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#include <algorithm>
#include "EditorCamera.hpp"
#include "../../core/Utility.hpp"

using sf::Keyboard::isKeyPressed;
using sf::Keyboard::Key;
using sf::Mouse::isButtonPressed;
using sf::Mouse::Button;

void EditorCamera::handleInput(sf::Vector2f mouseWorldPosition)
{
	handleMovingInput();
	handlePanningInput(mouseWorldPosition);
}

void EditorCamera::update(float fixedTimeStep, float mouseWheelDelta, sf::Vector2f mouseWorldPosition)
{
	// Update the camera zoom level based on mouse wheel input
	if (mouseWheelDelta != 0.f)
	{
		zoomLevel -= mouseWheelDelta * ZOOM_SPEED * fixedTimeStep;
		zoomLevel = std::clamp(zoomLevel, ZOOM_MIN, ZOOM_MAX);
	}
	// Update the camera position when panning with the middle mouse button
	//if (isPanning)
	//{
	//	sf::Vector2f delta = anchorPoint - mouseWorldPosition;
	//	view.move(delta * fixedTimeStep * PAN_SPEED);
	//}
	//// Update the camera position when moving with the arrow keys
	//else
	//{
	//	view.move(direction * MOVE_SPEED * zoomLevel * fixedTimeStep);
	//}
}

void EditorCamera::preRenderUpdate(float fixedTimeStep, float interpolationFactor, sf::Vector2f mouseWorldPosition)
{
	if (isPanning)
	{
		sf::Vector2f delta = anchorPoint - mouseWorldPosition;
		view.move(delta * fixedTimeStep * PAN_SPEED);
	}
	// Update the camera position when moving with the arrow keys
	else
	{
		view.move(direction * MOVE_SPEED * zoomLevel * fixedTimeStep);
	}
}

void EditorCamera::resize(sf::Vector2u windowSize)
{
	// Apply camera zoom and set the window view size
	view.setSize({ windowSize.x * zoomLevel, windowSize.y * zoomLevel });
}

void EditorCamera::handleMovingInput()
{
	direction = { 0.f, 0.f };

	// Set the direction the camera shoudl move based on keys held
	if      (isKeyPressed(Key::Left))  direction.x = -1.f;
	else if (isKeyPressed(Key::Right)) direction.x =  1.f;
	if      (isKeyPressed(Key::Up))    direction.y = -1.f;
	else if (isKeyPressed(Key::Down))  direction.y =  1.f;
}

void EditorCamera::handlePanningInput(sf::Vector2f mouseWorldPosition)
{
	if (isButtonPressed(Button::Middle))
	{
		if (!isPanning)
		{
			isPanning = true;
			anchorPoint = mouseWorldPosition;
		}
	}
	else
	{
		isPanning = false;
	}
}