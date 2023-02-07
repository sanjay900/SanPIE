//
// Created by sanjay on 18/07/18.
//

#include "Output.hpp"
#include "VKeyboard.hpp"
#include "SerialOut.h"
#include "VJoy.hpp"

Output* Output::create(std::string &name, sol::table &lua_dev, sol::state &lua) {
    sol::optional<std::string> typeOpt = lua_dev["type"];
    std::string type;
    if (typeOpt != sol::nullopt) {
        type = typeOpt.value();
    }
    Output* out;
    if (type == "keyboard") {
        out = new VKeyboard(name, lua_dev, lua);
    } else if(type == "serial"){
        out = new SerialOut(name, lua_dev);
    } else {
        out = new VJoy(name, lua_dev);
    }
    lua_dev["dev"] = out;
    return out;
}


