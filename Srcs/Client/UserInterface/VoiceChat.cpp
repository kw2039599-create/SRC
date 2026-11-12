#include "stdafx.h"
#ifdef ENABLE_VOICE_CHAT
#include "VoiceChat.h"
#include "PythonApplication.h"
#include "PythonNetworkStream.h"
#include <cmath>

#if defined(_DEBUG) || defined(__ANDROID__)
static constexpr auto DISABLE_ACTIVE_WINDOW_CHECK_FOR_TALKING = true;
static constexpr auto DISABLE_ACTIVE_WINDOW_CHECK_FOR_HEARING = true;
#else
#define USE_COMPRESSION
static constexpr auto DISABLE_ACTIVE_WINDOW_CHECK_FOR_TALKING = false;
static constexpr auto DISABLE_ACTIVE_WINDOW_CHECK_FOR_HEARING = false;
#endif

#ifdef __ANDROID__
extern "C" {
	bool MobileVoiceChat_Init(uint32_t sampleRate, uint8_t channelCount);
	void MobileVoiceChat_Destroy();
	bool MobileVoiceChat_RequestPermission();
	bool MobileVoiceChat_StartRecording();
	void MobileVoiceChat_StopRecording();
	void MobileVoiceChat_AddPlaybackSamples(const int16_t* samples, size_t count, float volumePercent);
	void MobileVoiceChat_PollCapturedSamples(VoiceChat* pVoiceChat);
}
#endif

static constexpr uint32_t SAMPLE_RATE = 32000;
static constexpr uint8_t CHANNEL_COUNT = 1;

enum EVoiceRecorderStates
{
	RECORDER_STATE_OFF,
	RECORDER_STATE_PUSH_TO_TALK,
	RECORDER_STATE_ALWAYS_ON,
};

VoiceChat::VoiceChat()
{
	Initialize();
}

VoiceChat::~VoiceChat()
{
	Destroy();
}

void VoiceChat::Initialize()
{
	if (m_IsInitialized)
		return;

	// Defaults
	m_ChatType = VOICE_CHAT_TYPE_LOCAL;
	m_RecorderState = RECORDER_STATE_OFF;
	m_Disabled = false;
	m_IsRecording = false;
	m_Volume = 100.0f;

	// Init
#ifndef __ANDROID__
	amun::VoiceChat::setPlaybackDelay(50);
	amun::VoiceChat::init(SAMPLE_RATE, CHANNEL_COUNT);
#else
	MobileVoiceChat_Init(SAMPLE_RATE, CHANNEL_COUNT);
#endif
	m_IsInitialized = true;
}

void VoiceChat::Destroy()
{
	StopRecording();
#ifndef __ANDROID__
	amun::VoiceChat::destroy();
#else
	MobileVoiceChat_Destroy();
#endif
	m_IsInitialized = false;
}

bool VoiceChat::IsRecording() const
{
	return m_IsRecording;
}

bool VoiceChat::StartRecording()
{
#ifndef __ANDROID__
	m_IsRecording = amun::VoiceChat::startRecording();
#else
	m_IsRecording = MobileVoiceChat_StartRecording();
#endif
	return m_IsRecording;
}

void VoiceChat::StopRecording()
{
#ifndef __ANDROID__
	amun::VoiceChat::stopRecording();
#else
	MobileVoiceChat_StopRecording();
#endif
	m_IsRecording = false;
}

uint32_t VoiceChat::GetSampleRate() const
{
#ifndef __ANDROID__
	return amun::VoiceChat::getSampleRate();
#else
	return SAMPLE_RATE;
#endif
}

uint8_t VoiceChat::GetChannelCount() const
{
#ifndef __ANDROID__
	return amun::VoiceChat::getChannelCount();
#else
	return CHANNEL_COUNT;
#endif
}

void VoiceChat::SetChatType(uint8_t chatType)
{
	m_ChatType = chatType;
}

void VoiceChat::SetRecorderState(uint8_t newRecorderState)
{
	if (m_RecorderState == RECORDER_STATE_ALWAYS_ON || newRecorderState == RECORDER_STATE_OFF)
	{
		StopRecording();
	}
	m_RecorderState = newRecorderState;
#ifdef __ANDROID__
	if (newRecorderState != RECORDER_STATE_OFF)
	{
		MobileVoiceChat_RequestPermission();
		if (newRecorderState == RECORDER_STATE_ALWAYS_ON)
		{
			StartRecording();
		}
	}
#endif
}

void VoiceChat::StartTalking()
{
	bool isActive = CPythonApplication::Instance().IsActive() || DISABLE_ACTIVE_WINDOW_CHECK_FOR_TALKING;
	if (!m_Disabled && isActive && m_RecorderState == RECORDER_STATE_PUSH_TO_TALK)
	{
		StartRecording();
	}
}

void VoiceChat::StopTalking()
{
	if (!m_Disabled && m_RecorderState == RECORDER_STATE_PUSH_TO_TALK)
	{
		StopRecording();
	}
}

void VoiceChat::SetDisabled(bool flag)
{
	m_Disabled = flag;
	if (m_Disabled && IsRecording())
	{
		StopRecording();
	}
}

void VoiceChat::SetVolume(float volume)
{
	m_Volume = volume;
}

void VoiceChat::SetSpeakerVolume(const std::string& name, float volume)
{
	m_SpeakerVolumes[name] = volume;
}

void VoiceChat::OnRecvVoiceChatPacket(const TVoiceChatPacket& packet, const char* data)
{
#undef min
	if (CPythonApplication::Instance().IsActive() || DISABLE_ACTIVE_WINDOW_CHECK_FOR_HEARING)
	{
		float spkVol = 100.0f;
		auto it = m_SpeakerVolumes.find(packet.name);
		if (it != m_SpeakerVolumes.end())
			spkVol = it->second;

		float volume = m_Volume * spkVol / 100.0f;
		if (packet.type == VOICE_CHAT_TYPE_LOCAL)
		{
			const float distanceToVolumeReduction = 100.0f - std::min<float>(packet.distance / 100.0f * 1.9f, 100.0f);
			volume = volume * distanceToVolumeReduction / 100.0f;
		}

#ifndef __ANDROID__
		amun::VoiceChat::setVolume(packet.name, volume);
		if (packet.isEncoded)
		{
			const auto& decodedSamples = amun::VoiceChat::decode(data, packet.dataSize);
			amun::VoiceChat::addSamples(packet.name, decodedSamples.data(), decodedSamples.size());
		}
		else
		{
			amun::VoiceChat::addSamples(packet.name, reinterpret_cast<const int16_t*>(data), packet.dataSize / sizeof(int16_t));
		}
#else
		if (!packet.isEncoded && data && packet.dataSize >= sizeof(int16_t))
		{
			MobileVoiceChat_AddPlaybackSamples(reinterpret_cast<const int16_t*>(data), packet.dataSize / sizeof(int16_t), volume);
		}
#endif
	}
}

void VoiceChat::onCaptureSamples(const int16_t* samples, uint32_t sampleCount)
{
	static auto& rkNet = CPythonNetworkStream::Instance();
	uint32_t dataSize = sampleCount * sizeof(int16_t);

	if (!samples || !sampleCount || (sampleCount > GetSampleRate() / 10))
	{
		TraceError("Invalid samples. Count %u", sampleCount);
		return;
	}

	static std::vector<int16_t> samplesVec(sampleCount);
	samplesVec.clear();
	samplesVec.insert(samplesVec.end(), samples, samples + sampleCount);
	if (!processVoice(samplesVec.data(), samplesVec.size()))
		return;

#if defined(USE_COMPRESSION) && !defined(__ANDROID__)
	const auto& encodedSamples = amun::VoiceChat::encode(samplesVec.data(), samplesVec.size());
	dataSize = encodedSamples.size();
	if (!dataSize || dataSize > sampleCount * sizeof(int16_t))
	{
		TraceError("Invalid data size %u, sampleCount %u", dataSize, sampleCount);
		return;
	}
#endif

	TVoiceChatPacket packet{};
	packet.header = HEADER_CG_VOICE_CHAT;
	packet.type = m_ChatType;
	packet.dataSize = dataSize;
	packet.size = sizeof(packet) + dataSize;
#if defined(USE_COMPRESSION) && !defined(__ANDROID__)
	packet.isEncoded = true;
#else
	packet.isEncoded = false;
#endif
	if (rkNet.HasSpaceToSend(packet.size))
	{
		rkNet.Send(sizeof(TVoiceChatPacket), &packet);
#if defined(USE_COMPRESSION) && !defined(__ANDROID__)
		rkNet.Send(dataSize, encodedSamples.data());
#else
		rkNet.Send(dataSize, samplesVec.data());
#endif
	}
}

void VoiceChat::onError(uint8_t error)
{
	TraceError("VOICE CHAT ERROR: %s", m_Errors[error]);
}

bool VoiceChat::processVoice(int16_t* samples, std::size_t sampleCount)
{
	static int inactiveBypassLeft = 0;
	static float smallestSplValue = 100.0f;
	constexpr float targetSPL = 65.0f; // normalize to 65 dB SPL

#ifndef __ANDROID__
	float currentSPL = amun::VoiceChat::calculateSoundPressureLevel(samples, sampleCount);
#else
	double sumSq = 0.0;
	for (std::size_t i = 0; i < sampleCount; ++i) {
		double s = (double)samples[i];
		sumSq += s * s;
	}
	double rms = (sampleCount > 0) ? std::sqrt(sumSq / (double)sampleCount) : 0.0;
	float currentSPL = (rms > 1.0) ? (float)(20.0 * std::log10(rms / 32768.0) + 90.0) : 0.0f;
#endif

	if (currentSPL < 10.0f)
		return false;

	if (currentSPL < smallestSplValue)
	{
		smallestSplValue = currentSPL;
	}

	if (currentSPL > targetSPL)
	{
#ifndef __ANDROID__
		amun::VoiceChat::normalizeAmplitude(samples, sampleCount, currentSPL, targetSPL);
#else
		float gain = std::pow(10.0f, (targetSPL - currentSPL) / 20.0f);
		for (std::size_t i = 0; i < sampleCount; ++i) {
			int32_t v = (int32_t)(samples[i] * gain);
			if (v > 32767) v = 32767;
			else if (v < -32768) v = -32768;
			samples[i] = (int16_t)v;
		}
#endif
	}

	float requiredBasedOnSmallest = std::fmax(20.0f, static_cast<float>(smallestSplValue * 1.5f));
	if (currentSPL > requiredBasedOnSmallest)
	{
		inactiveBypassLeft = 4;
		return true;
	}
	else if (--inactiveBypassLeft > 0)
	{
		return true;
	}
	return false;
}

void VoiceChat::Process()
{
	if (m_Disabled || !m_IsInitialized)
		return;

#ifdef __ANDROID__
	MobileVoiceChat_PollCapturedSamples(this);
#endif

	bool isActive = CPythonApplication::Instance().IsActive() || DISABLE_ACTIVE_WINDOW_CHECK_FOR_TALKING;
	if (IsRecording() && !isActive)
	{
		StopRecording();
	}
#ifndef __ANDROID__
	else if (!IsRecording() && isActive && m_RecorderState == RECORDER_STATE_ALWAYS_ON && amun::VoiceChat::isReady())
	{
		StartRecording();
	}
#else
	else if (!IsRecording() && isActive && m_RecorderState == RECORDER_STATE_ALWAYS_ON)
	{
		static DWORD s_dwLastRetryMs = 0;
		DWORD nowMs = ELTimer_GetMSec();
		if (nowMs - s_dwLastRetryMs > 1500) {
			s_dwLastRetryMs = nowMs;
			StartRecording();
		}
	}
#endif
}
#endif