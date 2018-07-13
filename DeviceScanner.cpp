//
// Created by sanjay on 13/07/18.
//

#include <libudev.h>
#include <input/Controller.hpp>
#include <input/InputFactory.hpp>
#include <output/OutputFactory.hpp>
#include "DeviceScanner.hpp"
#include "ControllerException.hpp"

void DeviceScanner::scan_devices(sol::state &lua) {
    sol::table devices = lua["devices"];
    for (auto &device : devices) {
        auto name = device.first.as<std::string>();
        sol::table lua_dev = device.second;
        lua_dev["name"] = name;
        lua_dev["dev"] = InputFactory::create(name,lua_dev);
    }
    for (auto &device : devices) {
        sol::table lua_dev = device.second;
        Controller &c = lua_dev["dev"];
        this->devices.push_back(&c);
    }
    std::sort(this->devices.begin(), this->devices.end());
}
bool compareDev(const std::pair<std::string, sol::table>& a, const std::pair<std::string, sol::table>& b) {
    return a.first < b.first;

}
void DeviceScanner::scan_v_devices(sol::state &lua) {
    sol::table v_devices = lua["v_devices"];
    std::vector<std::pair<std::string,sol::table>> v_devices_sorted;
    for (auto &device : v_devices) {
        auto name = device.first.as<std::string>();
        v_devices_sorted.emplace_back(name, device.second);
    }
    std::sort(v_devices_sorted.begin(), v_devices_sorted.end(), compareDev);
    for (auto &device: v_devices_sorted) {
        auto name = device.first;
        sol::table lua_dev = device.second;
        OutputFactory::create(name, lua_dev, lua);
    }
}

void DeviceScanner::scan_connected_devices(sol::state &lua)  {
    struct udev *udev;
    struct udev_device *dev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *device_list, *dev_list_entry;

    /* create udev object */
    udev = udev_new();
    if (!udev) {
        throw ControllerException("Cannot create udev context.");
    }
    /* create enumerate object */
    enumerate = udev_enumerate_new(udev);
    if (!enumerate) {
        throw ControllerException("Cannot create enumerate context.");
    }

    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);

    /* fillup device list */
    device_list = udev_enumerate_get_list_entry(enumerate);
    if (!device_list) {
        throw ControllerException("Failed to get device list.");
    }

    udev_list_entry_foreach(dev_list_entry, device_list) {
        const char *path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        if (dev == nullptr) continue;
        std::string dev_name = udev_device_get_sysname(dev);
        if (dev_name.find("event") != std::string::npos) {
            for (auto &c : devices) {
                if (c->try_to_use_device(udev, dev,lua)) {
                    break;
                }
            }
        }
        udev_device_unref(dev);

    }
    /* free enumerate */
    udev_enumerate_unref(enumerate);
    /* free udev */
    udev_unref(udev);
}

DeviceScanner::DeviceScanner(sol::state &lua) {
    scan_devices(lua);
    scan_v_devices(lua);
    scan_connected_devices(lua);
}
