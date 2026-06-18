#ifndef DRIVER_REGISTRY_HPP
#define DRIVER_REGISTRY_HPP

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <utility>
#include "model_info.hpp"

class driver_base;

struct driver_descriptor
{
    const char*                            name;
    DriverId                               id;
    void                                 (*add_model)(std::vector<ModelInfo>&);
    std::unique_ptr<driver_base>         (*new_instance)();
    void                                 (*add_to_test)();
};

class driver_registry
{
  public:
    struct register_t
    {
        explicit register_t(driver_descriptor desc)
        {
            driver_registry::instance().register_driver(std::move(desc));
        }
    };

    static driver_registry& instance()
    {
        static driver_registry reg;
        return reg;
    }

    void register_driver(driver_descriptor desc)
    {
        int id_int = static_cast<int>(desc.id);
        descriptors_[id_int] = std::move(desc);
    }

    std::unique_ptr<driver_base> create_driver(int id) const
    {
        auto it = descriptors_.find(id);
        if (it != descriptors_.end() && it->second.new_instance)
        {
            return it->second.new_instance();
        }
        return nullptr;
    }

    std::vector<ModelInfo> get_models(int id) const
    {
        auto it = descriptors_.find(id);
        if (it != descriptors_.end() && it->second.add_model)
        {
            std::vector<ModelInfo> models;
            it->second.add_model(models);
            return models;
        }
        return {};
    }

    const driver_descriptor* find_descriptor(int id) const
    {
        auto it = descriptors_.find(id);
        return (it != descriptors_.end()) ? &it->second : nullptr;
    }

    std::vector<std::pair<int, std::string>> list_drivers() const
    {
        std::vector<std::pair<int, std::string>> result;
        for (const auto& [id, desc] : descriptors_)
        {
            result.emplace_back(id, desc.name);
        }
        return result;
    }

  private:
    driver_registry() = default;
    driver_registry(const driver_registry&) = delete;
    driver_registry& operator=(const driver_registry&) = delete;

    std::map<int, driver_descriptor> descriptors_;
};

#endif
