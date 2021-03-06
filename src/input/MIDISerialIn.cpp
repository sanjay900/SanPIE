//
// Created by sanjay on 4/07/18.
//

#include "MIDISerialIn.hpp"
#include "src/DeviceException.hpp"
#include <utility>
#include <fcntl.h>
#include <unistd.h>
#include <csignal>
#include <termio.h>
#include <termios.h>
#include <linux/serial.h>
#include <linux/ioctl.h>
#include <asm/ioctls.h>
#include <libudev.h>
#include "Input.hpp"

MIDISerialIn::MIDISerialIn(const std::string &lua_name, sol::table &lua_table): Input(lua_name, "MIDISerialIn", lua_table),SerialIn(lua_name, lua_table), CoreMIDIIn(lua_name, lua_table) {}

bool MIDISerialIn::try_to_use_device(struct udev *udev, struct udev_device *device, sol::state &lua) {
    std::string sysname = std::string("/dev/")+udev_device_get_sysname(device);
    if (sysname != Serial::sysname) {
        return false;
    }
    Serial::try_to_use_device(udev, device, lua);
    std::cout << "Waiting for midi control signal from " << lua_name << std::endl;
    unsigned char buf[1];
    do read(fd, buf, 1);
    while (buf[0] >> 7u == 0);
    return false;
}

void MIDISerialIn::tick(sol::state& lua) {
    if (!isValid()) return;

    while (i < 3) {
        if (read(fd, buf+i, 1) < 0) {
            return;
        }
        if (buf[i] >> 7u != 0) {
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
                if ((buf[0] & 0xF0u) == 0xC0 || (buf[0] & 0xF0u) == 0xD0) {
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
        size_t msglen = buf[0];
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
