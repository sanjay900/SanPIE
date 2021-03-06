//
// Created by sanjay on 4/07/18.
//

#ifndef WEJOY_MIDIBASE_H
#define WEJOY_MIDIBASE_H


#include "Input.hpp"
#include <rtmidi/RtMidi.h>

class CoreMIDIIn: public virtual Input {
    static int padding;
    static std::map<unsigned char, std::string> func_map;
protected:
    bool debug;
    CoreMIDIIn(const std::string &lua_name, sol::table &dev);
public:
    void parse_midi_command(unsigned char *buf, sol::state &lua);
};


#endif
