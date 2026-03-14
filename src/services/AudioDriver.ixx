module;
#include <atomic>
#include <cmath>
#include <iostream>
#include <format>
#include <vector>
#include <optional>
#include "miniaudio.h"
export module Crystal_Service.AudioDriver;
import Crystal_Log;
import Crystal_Core.Foundation;
import Crystal_Core.AudioNode;
using namespace Crystal;

export namespace Crystal::Service
{
	//Utility struct
	struct Device_Descriptor
	{
		ma_device_info info;
		ma_device_type type;
	};
	using Device_List = std::vector<Device_Descriptor>;

	//Utility func
	State_Code Enum_Audio_Devices(Device_List& device_list)
	{
		ma_context context;
		if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
			std::cout << "Failed to initialize context" << std::endl;
			return State_Code::Fail;
		}

		ma_device_info* pPlaybackDeviceInfos;
		ma_uint32 playbackDeviceCount;
		ma_device_info* pCaptureDeviceInfos;
		ma_uint32 captureDeviceCount;

		if (ma_context_get_devices(&context, &pPlaybackDeviceInfos, &playbackDeviceCount,
			&pCaptureDeviceInfos, &captureDeviceCount) != MA_SUCCESS) {
			std::cout << "Failed to retrieve device information." << std::endl;
			ma_context_uninit(&context);
			return State_Code::Fail;
		}

		device_list.reserve(playbackDeviceCount + captureDeviceCount);
		for (ma_uint32 id = 0; id < playbackDeviceCount; id++)
		{
			Device_Descriptor config;
			config.info = pPlaybackDeviceInfos[id];
			config.type = ma_device_type_playback;
			device_list.push_back(config);
		}
		for (ma_uint32 id = 0; id < captureDeviceCount; id++)
		{
			Device_Descriptor config;
			config.info = pCaptureDeviceInfos[id];
			config.type = ma_device_type_capture;
			device_list.push_back(config);
		}

		ma_context_uninit(&context);
		return State_Code::Success;
	}
	State_Code Show_Audio_Devices(Device_List& device_list)
	{
		if (device_list.empty()) {
			return State_Code::Invalid_Args;
		}
		for (size_t id = 0; id < device_list.size(); id++) {
			std::string device_typename;
			switch (device_list[id].type)
			{
			default:
				device_typename = "Unknown";
				break;
			case ma_device_type_playback:
				device_typename = "Playback";
				break;
			case ma_device_type_capture:
				device_typename = "Capture";
				break;
			}
			std::cout << std::format("{:<8} Device [{}]: {:<30} ", device_typename, id, device_list[id].info.name) << std::endl;
		}
		return State_Code::Success;
	}

	class Audio_Device
	{
	public:
		Audio_Device(Device_Descriptor device_descriptor)
			: device_property(device_descriptor) {
		}
		~Audio_Device() {
			Uninit();
		}

		State_Code Init(Audio_Descriptor& setting)
		{
			if (initialized.load() == true) {
				Crystal_Debug_Log("check here");
				return State_Code::Success;
			}
			if (setting.sample_rate == 0) {
				Crystal_Debug_Log("check here");
				return State_Code::Invalid_Args;
			}

			current_setting = setting;

			if (Check_If_Setting_Supported() != State_Code::Success) {
				Crystal_Debug_Log("check here");
				return State_Code::Not_Ready;
			}

			ma_device_config config = ma_device_config_init(device_property.type);
			config.sampleRate = current_setting.sample_rate;
			switch (device_property.type)
			{
			default:
				break;
			case ma_device_type_playback:
				config.playback.format = current_setting.format;
				config.playback.channels = current_setting.channels;
				break;

			case ma_device_type_capture:
				config.capture.format = current_setting.format;
				config.capture.channels = current_setting.channels;
				break;
			}
			config.dataCallback = data_callback_func;
			config.pUserData = this;

			if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
				Crystal_Debug_Log("check here");
				return State_Code::Device_Error;
			}

			initialized.store(true);
			return State_Code::Success;
		}
		State_Code Change_Setting(Audio_Descriptor& new_setting)
		{
			State_Code result;
			if ((result = Stop()) != State_Code::Success) {
				Crystal_Debug_Log("check here");
				return result;
			}
			if ((result = Uninit()) != State_Code::Success) {
				Crystal_Debug_Log("check here");
				return result;
			}

			return Init(new_setting);
		}
		State_Code Run()
		{
			if (ma_device_start(&device) != MA_SUCCESS) {
				Crystal_Debug_Log("check here");
				return State_Code::Fail;
			}
			return State_Code::Success;
		}
		State_Code Stop()
		{
			if (ma_device_stop(&device) != MA_SUCCESS) {
				Crystal_Debug_Log("check here");
				return State_Code::Device_Busy;
			}
			return State_Code::Success;
		}
		State_Code Uninit()
		{
			ma_device_uninit(&device);

			current_setting.sample_rate = 0;
			initialized.store(false);
			
			return State_Code::Success;
		}
		 
		State_Code Set_Bus(std::shared_ptr<Audio_Bus> bus)
		{
			master_bus = bus;
			return State_Code::Success;
		}
		static void data_callback_func(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
		{
			Audio_Device* device = static_cast<Audio_Device*>(pDevice->pUserData);
			if (!device or !device->master_bus or !pOutput) {
				Crystal_Debug_Log("check here");
				return;
			}

			//pOutput is held by device_buffer to get rid of extra memcpy cost
			Audio_Buffer device_buffer{ .data = pOutput, .spec = device->current_setting, .frame_count = frameCount };
			device->master_bus.get()->Process(device_buffer);
		}

		Device_Descriptor	device_property;
		Audio_Descriptor	current_setting;

	private:
		std::atomic<bool> initialized = false;
		ma_device device;
		std::shared_ptr<Audio_Bus> master_bus;

		//Utility func to check if audio_spec from current audio_descriptor is supported by hardware
		State_Code Check_If_Setting_Supported()
		{
			if (current_setting.sample_rate == 0) {
				return State_Code::Invalid_Args;
			}

			const auto& info = device_property.info;
			const auto& target = current_setting;

			if (info.nativeDataFormatCount == 0) {
				return State_Code::Success;
			}

			for (u32 i = 0; i < info.nativeDataFormatCount; ++i)
			{
				const auto& native = info.nativeDataFormats[i];
				if (native.sampleRate == target.sample_rate and 
					native.channels == target.channels and 
					native.format == target.format) {
					return State_Code::Success;
				}
			}
			return State_Code::Fail;
		}
	};
}
