
#include <iostream>
#include "pump_factory.hpp"
#include <string>
#include <thread>
#include <mutex>
#include "third_party/httplib.h"
#include "third_party/json.hpp"

static std::mutex driver_mutex;
static std::unique_ptr<driver_base> active_driver;
static int current_driver_id = 0;
static int current_model_id = 0;
static pump_factory factory;

int main()
{
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_file_content("static/index.html", "text/html");
    });

    svr.Post("/api/simulation/start", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto data = nlohmann::json::parse(req.body);
            int driver_id = data["driver_id"];
            int model_id  = data["model_id"];

            std::lock_guard<std::mutex> lock(driver_mutex);

            if (active_driver && active_driver->is_running())
            {
                active_driver->stop_simulation();
            }

            active_driver = factory.create_pump_driver(driver_id);
            if (!active_driver)
            {
                res.status = 400;
                res.set_content(R"({"error":"Invalid driver_id"})", "application/json");
                return;
            }

            active_driver->set_devices();

            if (!active_driver->device_selection_by_index(model_id))
            {
                active_driver.reset();
                res.status = 400;
                res.set_content(R"({"error":"Invalid model_id"})", "application/json");
                return;
            }

            current_driver_id = driver_id;
            current_model_id  = model_id;

            active_driver->start_simulation();

            res.set_content(R"({"status":"started"})", "application/json");
        }
        catch (const std::exception& e) {
            res.status = 400;
            res.set_content(nlohmann::json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    svr.Post("/api/simulation/stop", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(driver_mutex);

        if (!active_driver || !active_driver->is_running())
        {
            res.set_content(R"({"status":"not_running"})", "application/json");
            return;
        }

        active_driver->stop_simulation();

        res.set_content(R"({"status":"stopped"})", "application/json");
    });

    svr.Get("/api/simulation/status", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(driver_mutex);

        nlohmann::json status;

        if (!active_driver)
        {
            status["running"]   = false;
            status["driver_id"] = 0;
            status["model_id"]  = 0;
            res.set_content(status.dump(), "application/json");
            return;
        }

        pumpProto snapshot = active_driver->get_pump_snapshot();

        status["running"]       = active_driver->is_running();
        status["driver_id"]     = current_driver_id;
        status["model_id"]      = current_model_id;
        status["pump_on"]       = snapshot.pump_on;
        status["flow_rate"]     = snapshot.flow_rate;
        status["pressure"]      = snapshot.pressure;
        status["pump_power"]    = snapshot.pump_power;
        status["water_level"]   = snapshot.water_level;
        status["pump_run_time"] = snapshot.pump_run_time;

        res.set_content(status.dump(), "application/json");
    });

    svr.Get("/api/drivers/:driver_id/models", [](const httplib::Request& req, httplib::Response& res) {
        int driver_id = std::stoi(req.path_params.at("driver_id"));

        pump_factory temp_factory;
        auto temp_driver = temp_factory.create_pump_driver(driver_id);

        if (!temp_driver)
        {
            res.status = 400;
            res.set_content(R"({"error":"Invalid driver_id"})", "application/json");
            return;
        }

        temp_driver->set_devices();
        const auto& devices = temp_driver->get_device_list();

        nlohmann::json models = nlohmann::json::array();
        for (int i = 0; i < static_cast<int>(devices.size()); ++i)
        {
            models.push_back({
                {"id", i},
                {"name", devices[i].device_name},
                {"max_flow_rate", devices[i].max_flow_rate},
                {"max_pressure", devices[i].max_pressure},
                {"power", devices[i].power}
            });
        }

        res.set_content(models.dump(), "application/json");
    });

    svr.Get("/api/hello", [](const httplib::Request&, httplib::Response& res) {
        nlohmann::json j;
        j["message"] = "Hello, World!";
        res.set_content(j.dump(), "application/json");
    });

    svr.Get("/api/drivers", [](const httplib::Request&, httplib::Response& res) {
        nlohmann::json drivers = nlohmann::json::array();
        drivers.push_back({{"id", 1}, {"name", "Grundfos"}});
        drivers.push_back({{"id", 2}, {"name", "Wilo"}});
        res.set_content(drivers.dump(), "application/json");
    });

    std::cout << "Server starting on http://localhost:18080" << std::endl;

    std::thread server_thread([&svr]() {
        svr.listen("0.0.0.0", 18080);
    });

    server_thread.join();

    return 0;
}
