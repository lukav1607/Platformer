// ================================================================================================
// File: Time.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 30, 2025
// Description: A thin utility wrapper around Game::getInstance() to provide a simple interface for
//              accessing the current game time and delta time. This allows for easier access to
//              time-related functions without needing to directly reference the Game class.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include "Game.hpp"

namespace lv
{
    namespace Time
    {
        inline float get()
        {
            return Game::getInstance().getTime();
        }
        inline float delta()
        {
            return Game::getInstance().getDeltaTime();
        }

        // TODO: Add time scaling later
        // inline float scaled() { return get() * scale; }
    }
}