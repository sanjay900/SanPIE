//
// Created by sanjay on 4/07/18.
//

#include "SER_MIDI.hpp"
#include "ControllerException.hpp"
#include <utility>
#include <fcntl.h>
#include <unistd.h>
#include <csignal>
#include <termio.h>
#include <termios.h>
#include <linux/serial.h>
#include <linux/ioctl.h>
#include <asm/ioctls.h>
#include "buttons_ref.h"
#include "Controller.hpp"

MIDISerial::MIDISerial(const std::string &lua_name, sol::table &lua_table): Controller(lua_name, "MIDI_SERIAL", lua_table) {
    this->lua_table = lua_table;
    this->lua_name = lua_name;
    this->sysname = lua_table["device"];
    this->debug = lua_table.get_or("debug",false);
    switch (lua_table.get_or("baudrate",115200)) {
        case 1200   :baudrate = B1200  ; break;
        case 2400   : baudrate = B2400  ; break;
        case 4800   : baudrate = B4800  ; break;
        case 9600   : baudrate = B9600  ; break;
        case 19200  : baudrate = B19200 ; break;
        case 38400  : baudrate = B38400 ; break;
        case 57600  : baudrate = B57600 ; break;
        case 115200 : baudrate = B115200; break;
        default     : throw new ControllerException("Unknown baudrate");
    }
    lua_table["name"] = lua_name;
    fd = open(sysname.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        throw ControllerException("Unable to open serial port");
    }
    struct serial_struct ser_info;
    /* save current serial port settings */
    tcgetattr(fd, &oldtio);

    /* clear struct for new port settings */
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = baudrate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME]    = 0;
    newtio.c_cc[VMIN]     = 1;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);
    ioctl(fd, TIOCGSERIAL, &ser_info);
	ser_info.flags |= ASYNC_LOW_LATENCY;
	ioctl(fd, TIOCSSERIAL, &ser_info);
	std::cout << "Waiting for midi control signal from " << lua_name << std::endl;
	do read(fd, buf, 1);
    while (buf[0] >> 7 == 0);
}

bool MIDISerial::try_to_use_device(struct udev * udev, struct udev_device * device, sol::state &lua) {
    return false;
}
bool MIDISerial::try_disconnect(const std::string &sysname,sol::state *lua) {
    return false;
}
MIDISerial::~MIDISerial() {
    if(isValid()) {
        tcsetattr(fd, TCSANOW, &oldtio);
        close(fd);
        fd = -1;
    }
}
void MIDISerial::parse_midi_command(char *buf, sol::state& lua)
{
    int operation, channel, param1, param2;

    operation = buf[0] & 0xF0;
    channel   = buf[0] & 0x0F;
    param1    = buf[1];
    param2    = buf[2];
    auto func = lua["midi_in"];
    if (func != nullptr) {
        func(lua_table, operation, channel, param1, param2);
    }
    switch (operation)
    {
        case 0x80:
            func = lua["midi_in_note_off"];
            if (func != nullptr) {
                func(lua_table, channel, param1, param2);
            }
            if (debug)
                printf("Serial  0x%x Note off           %03u %03u %03u\n", operation, channel, param1, param2);
            break;

        case 0x90:
            func = lua["midi_in_note_on"];
            if (func != nullptr) {
                func(lua_table, channel, param1, param2);
            }
            if (debug)
                printf("Serial  0x%x Note on            %03u %03u %03u\n", operation, channel, param1, param2);
            break;

        case 0xA0:
            func = lua["midi_in_pressure_change"];
            if (func != nullptr) {
                func(lua_table, channel, param1, param2);
            }
            if (debug)
                printf("Serial  0x%x Pressure change    %03u %03u %03u\n", operation, channel, param1, param2);
            break;

        case 0xB0:
            func = lua["midi_in_controller_change"];
            if (func != nullptr) {
                func(lua_table, channel, param1, param2);
            }
            if (debug)
                printf("Serial  0x%x Controller change  %03u %03u %03u\n", operation, channel, param1, param2);
            break;

        case 0xC0:
            func = lua["midi_in_program_change"];
            if (func != nullptr) {
                func(lua_table, channel, param1);
            }
            if (debug)
                printf("Serial  0x%x Program change     %03u %03u\n", operation, channel, param1);
            break;

        case 0xD0:
            func = lua["midi_in_channel_change"];
            if (func != nullptr) {
                func(lua_table, channel, param1);
            }
            if (debug)
                printf("Serial  0x%x Channel change     %03u %03u\n", operation, channel, param1);
            break;

        case 0xE0:
            param1 = (param1 & 0x7F) + ((param2 & 0x7F) << 7);
            func = lua["midi_in_pitch_bend"];
            if (func != nullptr) {
                func(lua_table, channel, param1);
            }
            if (debug)
                printf("Serial  0x%x Pitch bend         %03u %05i\n", operation, channel, param1);
            break;

            /* Not implementing system commands (0xF0) */

        default:
            if (debug)
                printf("0x%x Unknown MIDI cmd   %03u %03u %03u\n", operation, channel, param1, param2);
            break;
    }
}
int i = 1;
void MIDISerial::tick(sol::state& lua) {
    int msglen;
    if (!isValid()) return;

    while (i < 3) {
        if (read(fd, buf+i, 1) < 0) {
            return;
        }
        if (buf[i] >> 7 != 0) {
            /* Status byte received and will always be first bit!*/
            buf[0] = buf[i];
            i = 1;
        } else {
            /* Data byte received */
            if (i == 2) {
                /* It was 2nd data byte so we have a MIDI event
                   process! */
                i = 3;
            } else {
                /* Lets figure out are we done or should we read one more byte. */
                if ((buf[0] & 0xF0) == 0xC0 || (buf[0] & 0xF0) == 0xD0) {
                    i = 3;
                } else {
                    i = 2;
                }
            }
        }

    }
    i = 1;

    /* print comment message (the ones that start with 0xFF 0x00 0x00 */
    if (buf[0] == (char) 0xFF && buf[1] == (char) 0x00 && buf[2] == (char) 0x00)
    {
        read(fd, buf, 1);
        msglen = buf[0];
        if (msglen > MAX_MSG_SIZE-1) msglen = MAX_MSG_SIZE-1;

        read(fd, msg, msglen);

        if (!debug) return;

        /* make sure the string ends with a null character */
        msg[msglen] = 0;

        puts("0xFF Non-MIDI message: ");
        puts(msg);
        putchar('\n');
        fflush(stdout);
    }

        /* parse MIDI message */
    else parse_midi_command(buf, lua);
}