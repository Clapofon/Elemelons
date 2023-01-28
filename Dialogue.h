#pragma once

#include <vector>
#include <string>

#include <glm.hpp>

#include "../Engine/Timer.h"
#include "../Engine/AudioEngine.h"

enum class DialogueState
{
	DISPLAY,
	DISPLAY_UI,
	NONE
};

class Dialogue
{
public:
	Dialogue();
	~Dialogue();

	void init(const std::vector<std::string>& lines, const std::vector<std::string>& audio, const std::vector<float>& times);

	std::string getLine(uint32_t lineNumber);
	float getDisplayTime(uint32_t lineNumber);
	bool wasAudioPlayed(uint32_t lineNumber);
	std::string getAudio(uint32_t lineNumber);
	void playAudio(Engine::AudioEngine& audioEngine, uint32_t lineNumber, const glm::vec3& pos, float volume);

	uint32_t getLineNumber() { return m_lines.size(); }

	void reset();

	void setAudioPlayed(uint32_t lineNumber);
	void skipAudio(uint32_t lineNumber, Engine::AudioEngine& audioEngine);

private:

	std::vector<std::string> m_lines;
	std::vector<float> m_lineDisplayTimes;
	std::vector<bool> m_audioPlayed;
	std::vector<std::string> m_dialogueAudio;
	std::vector<uint32_t> m_audioChannelIds;
};

