#include "grundfos_pump.hpp"
#include "driver_registry.hpp"

enum grundfos_modbus_register
{
	REG_FLOW_RATE   = 100,
	REG_PRESSURE    = 102,
	REG_PUMP_POWER  = 104,
	REG_WATER_LEVEL = 106,
	REG_RUN_TIME    = 108,
	REG_PUMP_ON     = 110,
};

constexpr uint16_t PUMP_SENSOR_BLOCK_SIZE = 2;

static volatile driver_registry::register_t grundfos_reghelper({
	.name         = "Grundfos",
	.id           = DriverId::GRUNDFOS_PUMP,
	.add_model    = grundfos_pump::add_model,
	.new_instance = grundfos_pump::new_instance,
	.add_to_test  = nullptr
});

void grundfos_pump::add_model(std::vector<ModelInfo>& models)
{
	const std::vector<ModelInfo> grundfos_models =
	{
		{ .deviceId = 0, .Name = "Grundfos CRE 20", .max_flow_rate = 20.0f, .max_pressure = 2.0f, .power = 500.0f,
		  .deviceType = DeviceType::Pump, .SupportEth = true, .SupportRS485 = false,
		  .EthProtocol = CommProtocol::PROTOCOL_MODBUS_TCP },

		{ .deviceId = 1, .Name = "Grundfos CRE 30", .max_flow_rate = 30.0f, .max_pressure = 3.0f, .power = 600.0f,
		  .deviceType = DeviceType::Pump, .SupportEth = true, .SupportRS485 = true,
		  .EthProtocol = CommProtocol::PROTOCOL_MODBUS_TCP, .SerialProtocol = CommProtocol::PROTOCOL_MODBUS_RTU }
	};

	for (auto model : grundfos_models)
	{
		models.push_back(model);
	}
}

std::unique_ptr<driver_base> grundfos_pump::new_instance()
{
	return std::make_unique<grundfos_pump>();
}

grundfos_pump::grundfos_pump()
	: driver_base(grundfos_simulation_pump)
{
	mModbus = std::make_unique<ModbusTcp>(1);

	mModbus->mModbusCore->addRegisterBlock(RegisterType::InputRegister, REG_FLOW_RATE,   AccessType::ReadAccess, PUMP_SENSOR_BLOCK_SIZE);
	mModbus->mModbusCore->addRegisterBlock(RegisterType::InputRegister, REG_PRESSURE,    AccessType::ReadAccess, PUMP_SENSOR_BLOCK_SIZE);
	mModbus->mModbusCore->addRegisterBlock(RegisterType::InputRegister, REG_PUMP_POWER,  AccessType::ReadAccess, PUMP_SENSOR_BLOCK_SIZE);
	mModbus->mModbusCore->addRegisterBlock(RegisterType::InputRegister, REG_WATER_LEVEL, AccessType::ReadAccess, PUMP_SENSOR_BLOCK_SIZE);
	mModbus->mModbusCore->addRegisterBlock(RegisterType::InputRegister, REG_RUN_TIME,    AccessType::ReadAccess, PUMP_SENSOR_BLOCK_SIZE);
	mModbus->mModbusCore->addRegisterBlock(RegisterType::InputRegister, REG_PUMP_ON,     AccessType::ReadAccess, 1);

	pump_channels = std::make_unique<channel_device<RegisterType, uint16_t>>(*mModbus->mModbusCore);
	initChannels();
}

grundfos_pump::~grundfos_pump()
{}

void grundfos_pump::set_devices()
{
	device_list.clear();
	add_model(device_list);
}

void grundfos_pump::initChannels()
{
	pump_channels->addChannel("CH_FLOW_RATE",   RegisterType::InputRegister, REG_FLOW_RATE);
	pump_channels->addChannel("CH_PRESSURE",    RegisterType::InputRegister, REG_PRESSURE);
	pump_channels->addChannel("CH_PUMP_POWER",  RegisterType::InputRegister, REG_PUMP_POWER);
	pump_channels->addChannel("CH_WATER_LEVEL", RegisterType::InputRegister, REG_WATER_LEVEL);
	pump_channels->addChannel("CH_RUN_TIME",    RegisterType::InputRegister, REG_RUN_TIME);
	pump_channels->addChannel("CH_PUMP_ON",     RegisterType::InputRegister, REG_PUMP_ON);
}

void grundfos_pump::writeSimulationToRegisters()
{
	if (!pump_channels) return;

	pump_channels->setRegisterValueFloat("CH_FLOW_RATE",   pump.flow_rate,     1.0f);
	pump_channels->setRegisterValueFloat("CH_PRESSURE",    pump.pressure,      1.0f);
	pump_channels->setRegisterValueFloat("CH_PUMP_POWER",  pump.pump_power,    1.0f);
	pump_channels->setRegisterValueFloat("CH_WATER_LEVEL", pump.water_level,   1.0f);
	pump_channels->setRegisterValueFloat("CH_RUN_TIME",    pump.pump_run_time, 1.0f);
	pump_channels->setRegisterValue("CH_PUMP_ON", pump.pump_on ? 1 : 0);
}

void grundfos_pump::update_driver_value()
{
	writeSimulationToRegisters();
}
