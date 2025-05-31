// ================================================================================================
// File: Serializable.hpp
// Author: Luka Vukorepa (https://github.com/lukav1607)
// Created: May 25, 2025
// Description: Defines the Serializable class, which is a pure virtual base class for objects
//              that can be serialized to and deserialized from JSON.
// ================================================================================================
// License: MIT License
// Copyright (c) 2025 Luka Vukorepa
// ================================================================================================

#pragma once

#include <string>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#elif defined(_MSC_VER)
#pragma warning(push, 0)
#endif

#include "../external/json.hpp"

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

using json = nlohmann::json;

class Serializable
{
public:
	virtual ~Serializable() = default;

	// Serialize the object to JSON
	virtual void serialize(json& j) const = 0;
	// Deserialize the object from JSON
	virtual void deserialize(const json& j) = 0;
	// Get the type of the object as a string (for identification purposes)
	virtual std::string getType() const = 0;
};