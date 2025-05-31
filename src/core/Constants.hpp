// ================================================================================================
// File: Constants.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 28, 2025
// Description: Centralized game constants for physics, gameplay, timing, and UI.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

namespace lv {
    namespace Constants
    {
        // --- Physics ---
        constexpr float GRAVITY = 980.f; // Pixels per second^2, approx Earth gravity scaled for pixels
		constexpr float JUMP_FORCE = 500.f; // Force applied when jumping
        constexpr float TERMINAL_VELOCITY = 1500.f; // Max falling speed
        constexpr float FRICTION_GROUND = 1500.f; // Friction when on ground (pixels/sec^2)
        constexpr float FRICTION_AIR = 300.f; // Friction when in air (for air control)

        // --- Timing ---
        constexpr float FIXED_TIMESTEP = 1.f / 60.f; // Fixed update timestep for physics
        constexpr float MAX_FRAME_TIME = 0.25f; // Max dt to avoid spiral of death

        constexpr float PATHFINDING_UPDATE_INTERVAL = 0.33f; // Time in seconds between pathfinding updates
    }
}