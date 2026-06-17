#ifndef WEB_SERVER_HPP
#define WEB_SERVER_HPP

#include <httplib.h>
#include <nlohmann/json.hpp>
#include "driver_registry.hpp"
#include "driver_base.hpp"
#include <mutex>
#include <memory>
#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <sstream>

class web_server
{
  public:
    void start(int port, const std::string& static_dir)
    {
        setup_routes(static_dir);
        std::cout << "Server listening on http://0.0.0.0:" << port << std::endl;
        svr_.listen("0.0.0.0", port);
    }

  private:
    httplib::Server svr_;
    std::unique_ptr<driver_base> active_driver_;
    int active_model_id_ = -1;
    std::mutex sim_mutex_;

    void setup_routes(const std::string& static_dir)
    {
        svr_.set_mount_point("/static", static_dir);

        svr_.Get("/", [static_dir](const httplib::Request&, httplib::Response& res) {
            std::ifstream file(static_dir + "/index.html");
            if (file.is_open())
            {
                std::ostringstream ss;
                ss << file.rdbuf();
                res.set_content(ss.str(), "text/html");
            }
            else
            {
                res.status = 500;
                res.set_content("index.html not found", "text/plain");
            }
        });

        svr_.Get("/api/drivers", [this](const httplib::Request&, httplib::Response& res) {
            handle_get_drivers(res);
        });

        svr_.Get(R"(/api/drivers/(\d+)/models)", [this](const httplib::Request& req, httplib::Response& res) {
            int driver_id = std::stoi(req.matches[1]);
            handle_get_models(driver_id, res);
        });

        svr_.Post("/api/simulation/start", [this](const httplib::Request& req, httplib::Response& res) {
            handle_start(req, res);
        });

        svr_.Post("/api/simulation/stop", [this](const httplib::Request&, httplib::Response& res) {
            handle_stop(res);
        });

        svr_.Get("/api/simulation/status", [this](const httplib::Request&, httplib::Response& res) {
            handle_status(res);
        });
    }

    void handle_get_drivers(httplib::Response& res)
    {
        auto drivers = driver_registry::instance().list_drivers();
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& [id, name] : drivers)
        {
            arr.push_back({{"id", id}, {"name", name}});
        }
        res.set_content(arr.dump(), "application/json");
    }

    void handle_get_models(int driver_id, httplib::Response& res)
    {
        auto models = driver_registry::instance().get_models(driver_id);
        if (models.empty())
        {
            auto desc = driver_registry::instance().find_descriptor(driver_id);
            if (!desc)
            {
                nlohmann::json err = {{"error", "Unknown driver ID"}};
                res.status = 404;
                res.set_content(err.dump(), "application/json");
                return;
            }
        }

        nlohmann::json arr = nlohmann::json::array();
        for (const auto& m : models)
        {
            arr.push_back({
                {"id", m.deviceId},
                {"name", m.Name},
                {"max_flow_rate", m.max_flow_rate},
                {"max_pressure", m.max_pressure},
                {"power", m.power},
                {"device_type", static_cast<int>(m.deviceType)},
                {"support_eth", m.SupportEth},
                {"support_rs485", m.SupportRS485},
                {"eth_protocol", static_cast<int>(m.EthProtocol)},
                {"serial_protocol", static_cast<int>(m.SerialProtocol)}
            });
        }
        res.set_content(arr.dump(), "application/json");
    }

    void handle_start(const httplib::Request& req, httplib::Response& res)
    {
        nlohmann::json body;
        try
        {
            body = nlohmann::json::parse(req.body);
        }
        catch (...)
        {
            nlohmann::json err = {{"error", "Invalid JSON body"}};
            res.status = 400;
            res.set_content(err.dump(), "application/json");
            return;
        }

        if (!body.contains("driver_id") || !body.contains("model_id"))
        {
            nlohmann::json err = {{"error", "Missing driver_id or model_id"}};
            res.status = 400;
            res.set_content(err.dump(), "application/json");
            return;
        }

        int driver_id = body["driver_id"].get<int>();
        int model_id = body["model_id"].get<int>();
        std::string iface = "ethernet";
        if (body.contains("interface"))
        {
            iface = body["interface"].get<std::string>();
        }

        std::lock_guard<std::mutex> lock(sim_mutex_);

        if (active_driver_ && active_driver_->is_running())
        {
            active_driver_->stop_simulation();
        }

        active_driver_ = driver_registry::instance().create_driver(driver_id);
        if (!active_driver_)
        {
            nlohmann::json err = {{"error", "Unknown driver ID"}};
            res.status = 404;
            res.set_content(err.dump(), "application/json");
            return;
        }

        active_driver_->set_devices();

        if (!active_driver_->select_device_by_index(model_id))
        {
            active_driver_.reset();
            nlohmann::json err = {{"error", "Invalid model ID"}};
            res.status = 400;
            res.set_content(err.dump(), "application/json");
            return;
        }

        if (iface == "rs485")
        {
            auto models = driver_registry::instance().get_models(driver_id);
            if (model_id >= 0 && model_id < static_cast<int>(models.size()) && models[model_id].SupportRS485)
            {
                std::string device = "/dev/ttyUSB0";
                int baud = 9600;
                if (body.contains("device")) device = body["device"].get<std::string>();
                if (body.contains("baud"))   baud   = body["baud"].get<int>();
                active_driver_->configureRtu(device, baud);
            }
            else
            {
                active_driver_.reset();
                nlohmann::json err = {{"error", "Selected model does not support RS485"}};
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }
        }

        active_model_id_ = model_id;
        active_driver_->start_simulation();

        nlohmann::json ok = {{"status", "ok"}};
        res.set_content(ok.dump(), "application/json");
    }

    void handle_stop(httplib::Response& res)
    {
        std::lock_guard<std::mutex> lock(sim_mutex_);

        if (active_driver_ && active_driver_->is_running())
        {
            active_driver_->stop_simulation();
        }

        nlohmann::json ok = {{"status", "ok"}};
        res.set_content(ok.dump(), "application/json");
    }

    void handle_status(httplib::Response& res)
    {
        std::lock_guard<std::mutex> lock(sim_mutex_);

        nlohmann::json status;

        if (!active_driver_)
        {
            status = {
                {"running", false},
                {"flow_rate", 0},
                {"pressure", 0},
                {"pump_power", 0},
                {"water_level", 0},
                {"pump_run_time", 0},
                {"model_id", -1}
            };
        }
        else
        {
            std::lock_guard<std::mutex> data_lock(active_driver_->get_mutex());
            const auto& p = active_driver_->get_pump_data();
            status = {
                {"running", active_driver_->is_running()},
                {"flow_rate", p.flow_rate},
                {"pressure", p.pressure},
                {"pump_power", p.pump_power},
                {"water_level", p.water_level},
                {"pump_run_time", p.pump_run_time},
                {"model_id", active_model_id_}
            };
        }

        res.set_content(status.dump(), "application/json");
    }
};

#endif
