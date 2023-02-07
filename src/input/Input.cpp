//
// Created by sanjay on 4/07/18.
//
#include "Input.hpp"
#include "src/DeviceException.hpp"
#include "Wiimote.hpp"
#include <utility>
#include <libudev.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <fcntl.h>
#include <unistd.h>
#include "SerialIn.hpp"

Input::Input(const std::string &lua_name, const std::string &name, sol::table &lua_table) {
    this->lua_table = lua_table;
    this->lua_name = lua_name;
    this->name = name;
    lua_table["type"] = name;
    lua_table["name"] = lua_name;
}

const std::string &Input::getLua_name() const {
    return lua_name;
}

const std::string &Input::getName() const {
    return name;
}

Input *Input::create(std::string &name, sol::table &lua_table) {
    lua_table["name"] = name;
    sol::optional<std::string> typeOpt = lua_table["type"];
    if (typeOpt == sol::nullopt) {
        throw DeviceException("No type was defined.");
    }
    std::string type = typeOpt.value();
    if (type == "wii") {
        return new Wiimote(name, lua_table);
    }
    if (type == "serial") {
        return new SerialIn(name, lua_table);
    }
    return new Controller(name, type, lua_table);
}

const sol::table &Input::getLua_table() const {
    return lua_table;
}

const std::string &Input::getSysname() const {
    return sysname;
}
