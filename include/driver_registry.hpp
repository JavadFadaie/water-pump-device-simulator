#ifndef DRIVER_REGISTRY_HPP
#define DRIVER_REGISTRY_HPP

#include <map>
#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <utility>

class driver_base;

struct driver_entry
{
    int id;
    std::string name;
    std::function<std::unique_ptr<driver_base>()> factory_fn;
};

class driver_registry
{
  public:
    static driver_registry& instance()
    {
        static driver_registry reg;
        return reg;
    }

    bool register_driver(int id, const std::string& name,
                         std::function<std::unique_ptr<driver_base>()> fn)
    {
        drivers_[id] = {id, name, std::move(fn)};
        return true;
    }

    std::unique_ptr<driver_base> create_driver(int id) const
    {
        auto it = drivers_.find(id);
        if (it != drivers_.end())
        {
            return it->second.factory_fn();
        }
        return nullptr;
    }

    std::vector<std::pair<int, std::string>> list_drivers() const
    {
        std::vector<std::pair<int, std::string>> result;
        for (const auto& [id, entry] : drivers_)
        {
            result.emplace_back(id, entry.name);
        }
        return result;
    }

  private:
    driver_registry() = default;
    driver_registry(const driver_registry&) = delete;
    driver_registry& operator=(const driver_registry&) = delete;

    std::map<int, driver_entry> drivers_;
};

#define REGISTER_PUMP_DRIVER(id, name, ClassName)                          \
    namespace {                                                            \
        const bool ClassName##_registered = []() {                         \
            driver_registry::instance().register_driver(                   \
                id, name,                                                  \
                []() -> std::unique_ptr<driver_base> {                     \
                    return std::make_unique<ClassName>();                   \
                });                                                        \
            return true;                                                   \
        }();                                                               \
    }

#endif
