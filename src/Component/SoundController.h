#pragma once

#include <Siv3D.hpp>

class SoundController
{
public:
	SoundController();

	void Play(const String& asset_name, bool loop = false);

	void Stop(const String& asset_name);

	void StopAll();

	[[nodiscard]]
	bool IsPlaying(const String& asset_name) const;

	void SetVolume(const String& asset_name, double volume);
};
