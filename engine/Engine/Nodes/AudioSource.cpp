#include "AudioSource.h"
#include <miniaudio.h>
#include "Engine/AssetManagement/AudioClip.h"
#include "Engine/Data/Memory.h"

#include "Engine/Utilities/Debug.h"
#include "Engine/Utilities/DebugMemory.h"




//This is called on its own thread?
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
	AudioSource::UserData* userData = (AudioSource::UserData*)pDevice->pUserData;
	ma_decoder* pDecoder = (ma_decoder*)(userData->decoder);
	if (pDecoder == NULL) {
		return;
	}

	userData->frame++;

	//Debug::Log("Data Callback");

	ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount);

	(void)pInput;

	//This is called every frame anyways
	if (userData->frame >= frameCount) {
		//Cannot call stop from in here
		//Need to use the events
		//ma_device_stop(pDevice);
		//ma_decoder_seek_to_pcm_frame(pDecoder, 0);
		//userData->frame= 0;
		userData->isPlaying = false;
		return;
	}

}

AudioSource::AudioSource()
{
	_type = "AudioSource";
	_isPlaying = DBG_NEW bool(false);
	_clip = nullptr;
	_decoder = DBG_NEW ma_decoder();
	_device = DBG_NEW ma_device();
	_userData = nullptr;
}

AudioSource::~AudioSource()
{
	stop();
	ma_device_uninit(_device);
	ma_decoder_uninit(_decoder);
	delete(_device);
	delete(_decoder);
	delete(_isPlaying);
}

bool AudioSource::setAudioClip(AudioClip* clip)
{
	if (clip == nullptr) { return false; }
	if(ma_device_is_started(_device)) { ma_device_stop(_device); }


	_clip = clip;
	ma_result result;

	//Going to need to load in the memory myself and then pass it to mini audio
	result = ma_decoder_init_memory(
		clip->getData()->data,
		clip->getData()->size, /* Only used for decoders that were opened against a block of memory. */
		NULL, _decoder);

	if (result != MA_SUCCESS) {
		Debug::Log("Failed to create decoder.");
		return false;
	}

	ma_device_config deviceConfig;
	deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.format = _decoder->outputFormat;
	deviceConfig.playback.channels = _decoder->outputChannels;
	deviceConfig.sampleRate = _decoder->outputSampleRate;
	deviceConfig.dataCallback = data_callback;

	if(_userData != nullptr) { delete(_userData); }
	_userData = DBG_NEW UserData{ _decoder, _isPlaying };

	deviceConfig.pUserData = _userData;

	if (ma_device_init(NULL, &deviceConfig, _device) != MA_SUCCESS) {
		Debug::Log("Failed to open playback device.");
		ma_decoder_uninit(_decoder);
		return false;
	}
	Debug::Log("Setup AudioSource");
	return true;
}

void AudioSource::play()
{
	if (_clip == nullptr) { return; }
	if (ma_device_start(_device) != MA_SUCCESS) {
		Debug::Log("Failed to start playback device.");
		Debug::Log("Stopping!");
		ma_device_stop(_device);
		ma_decoder_seek_to_pcm_frame(_decoder, 0);
	}
	else {
		Debug::Log("Tryplay");
	}
}

void AudioSource::stop()
{
	if (_clip == nullptr) { return;  }
	ma_device_stop(_device);
}

bool AudioSource::isPlaying()
{
	if (_userData == nullptr) { return false; }
	return _userData->isPlaying;
}
