#ifndef GRUNDFOS_PUMP_HPP
#define GRUNDFOS_PUMP_HPP

#include "driver_base.hpp"
#include "channel_device.hpp"

class grundfos_pump : public driver_base
{
  public:
	explicit grundfos_pump();
	~grundfos_pump() override;

	static void                          add_model(std::vector<ModelInfo>& models);
	static std::unique_ptr<driver_base>  new_instance();

	void set_devices() override;
	void update_driver_value() override;

  private:
	void initChannels() override;
	void writeSimulationToRegisters() override;

	pumpProto grundfos_simulation_pump;
	std::unique_ptr<channel_device<RegisterType, uint16_t>> pump_channels;
};

#endif
