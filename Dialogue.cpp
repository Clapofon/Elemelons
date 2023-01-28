#include "Dialogue.h"

Dialogue::Dialogue()
{

}

Dialogue::~Dialogue()
{

}

void Dialogue::init(const std::vector<std::string>& lines, const std::vector<std::string>& audio, const std::vector<float>& times)
{
	m_lines = lines;
	m_dialogueAudio = audio;
	m_lineDisplayTimes = times;

	m_audioPlayed.resize(m_lines.size());
	m_audioChannelIds.resize(m_lines.size());
	for (uint32_t i = 0; i < m_lines.size(); i++)
	{
		m_audioPlayed[i] = false;
	}
}

std::string Dialogue::getLine(uint32_t lineNumber)
{
	if (lineNumber <= m_lines.size())
	{
		return m_lines[lineNumber];
	}
}

float Dialogue::getDisplayTime(uint32_t lineNumber)
{
	if (lineNumber <= m_lineDisplayTimes.size())
	{
		return m_lineDisplayTimes[lineNumber];
	}
}

bool Dialogue::wasAudioPlayed(uint32_t lineNumber)
{
	if (lineNumber <= m_audioPlayed.size())
	{
		return m_audioPlayed[lineNumber];
	}
}

std::string Dialogue::getAudio(uint32_t lineNumber)
{
	if (lineNumber <= m_dialogueAudio.size())
	{
		return m_dialogueAudio[lineNumber];
	}
}

void Dialogue::playAudio(Engine::AudioEngine& audioEngine, uint32_t lineNumber, const glm::vec3& pos, float volume)
{
	m_audioChannelIds[lineNumber] = audioEngine.playSound(m_dialogueAudio[lineNumber], { pos.x, pos.y, pos.z }, volume);
}

void Dialogue::setAudioPlayed(uint32_t lineNumber)
{
	if (lineNumber <= m_audioPlayed.size())
	{
		m_audioPlayed[lineNumber] = true;
	}
}

void Dialogue::reset()
{
	for (uint32_t i = 0; i < m_audioPlayed.size(); i++)
	{
		m_audioPlayed[i] = false;
	}
}

void Dialogue::skipAudio(uint32_t lineNumber, Engine::AudioEngine& audioEngine)
{
	for (uint32_t i = 0; i < m_audioChannelIds.size(); i++)
	{
		audioEngine.StopChannel(m_audioChannelIds[lineNumber]);
	}
}

