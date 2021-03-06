devices = {
    drums0 = {
        type = "wii",
        extension_type = "Drums"
    },
    -- guitar0 = {
    --     type = "wii",
    --     extension_type = "Guitar",
    -- },
    -- guitar1 = {
    --     type = "wii",
    --     extension_type = "Guitar",
    -- },
    -- guitar2 = {
    --     type = "wii",
    --     extension_type = "Guitar",
    -- },
    --    keys0 = {
    --        type = "midi",
    --        device = "Serial->MIDI:RtMidi Output 129:0",
    --        debug = true
    --    },
    -- serial0 = {
    --     type = "serial",
    --     device = "/dev/ttyACM0",
    --     baudrate = 1000000,
    -- },
    --    keys0 = {
    --        type = "midi_serial",
    --        device = "/dev/ttyACM1",
    --        baudrate = 115200,
    --        debug = false
    --    },
    --    nxt = {
    --        type = "serial",
    --        device = "/dev/rfcomm0",
    --        baudrate = 38400,
    --    }
}
v_devices = {
    --    key = {
    --        type="keyboard",
    --    },
    --    midi = {
    --        type="midi",
    --        device = "/dev/midi3"
    --    },
    -- vguitar0 = {
    --     buttons = 11,
    --     axes = 8
    -- },
    -- vguitar1 = {
    --     buttons = 11,
    --     axes = 6
    -- },
    -- vguitar2 = {
    --     buttons = 11,
    --     axes = 6
    -- },
    -- vguitar3 = {
    --     buttons = 11,
    --     axes = 6
    -- },
    vdrums0 = {
        buttons = 11,
        axes = 8,
        guitar = false
    },
    --    serial = {
    --        type = "serial",
    --        device = "/dev/ttyACM0",
    --        baudrate = 115200,
    --    },
    --    serial1 = {
    --        type = "serial",
    --        device = "/dev/ttyACM1",
    --        baudrate = 115200,
    --    },
    --    nxt = {
    --        type = "serial",
    --        device = "/dev/rfcomm0",
    --        baudrate = 57600,
    --    }
}
count = 0
function tick(usec)
    count = count + usec
    if count > 100000 then
        for k, v in pairs(v_devices) do
            if string.starts(k, "vdrums") then
                for i = 0,v.buttons do
                    v.send_button(i,false)
                end
            end
        end
        count = 0
        --        v_devices.nxt.write({0x03, 0x00, 0x00, 0x06,0x01})
    end
end
current = -1;
function disconnect_event(device)
    local name = device.name;
    local vDev = v_devices["v"..name];
    if string.starts(name,"guitar") then
        vDev.send_button(7,true)
        vDev.send_button(7,false)
    end
end

drum_mappings_a = {0,3,4,1,2,5}
function axis_event(device, axis, value)
    local name = device.name;
    local vDev = v_devices["v"..name];
    if string.starts(name,"guitar") then
        if device.type == "Accelerometer" then
            if axis == 0 then
                vDev.send_axis(4,math.max(math.pow((10000 - value),1.25),32768))
            end
        elseif axis == 0 then
            if value > 32767 / 2 then
                vDev.send_axis(6,32767)
            elseif value < -32767 / 2 then
                vDev.send_axis(6,-32767)
            else
                vDev.send_axis(6,0)
            end
        elseif axis == 1 then
            if value > 32767 / 2 then
                vDev.send_axis(7,-32767)
            elseif value < -32767 / 2 then
                vDev.send_axis(7,32767)
            else
                vDev.send_axis(7,0)
            end
        elseif axis == 3 then
            if value <= -32767 then
                vDev.send_axis(3,-32767)
            else
                vDev.send_axis(3,value - 32767/2)
            end
        end
    elseif string.starts(name,"drums") then
        if device.type == "Drums" then
            -- local vel = math.scale(value,-32767,0, 0, 127)
            -- vel = math.min(127,vel)
            -- --            v_devices.midi.note(1,50+axis,vel)
            -- if axis == 7 then
            --     v_devices.vguitar0.send_button(8,value > 0)
            -- end
            if vDev.guitar then
                if axis < 2 then
                    if value < -1000 then
                        vDev.send_button(axis+10,true)
                        vDev.send_button(axis+12,false)
                    elseif value > 1000 then
                        vDev.send_button(axis+12,true)
                        vDev.send_button(axis+10,false)
                    else
                        vDev.send_button(axis+10,false)
                        vDev.send_button(axis+12,false)
                    end
                elseif value > -32767 then
                    vDev.send_button(axis,true)
                    count = 0
                else
                    vDev.send_button(axis,false)
                end
            else
                if axis == 0 then
                    if value > 32767 / 2 then
                        vDev.send_axis(6,32767)
                    elseif value < -32767 / 2 then
                        vDev.send_axis(6,-32767)
                    else
                        vDev.send_axis(6,0)
                    end
                elseif axis == 1 then
                    if value > 32767 / 2 then
                        vDev.send_axis(7,-32767)
                    elseif value < -32767 / 2 then
                        vDev.send_axis(7,32767)
                    else
                        vDev.send_axis(7,0)
                    end
                else
                    vDev.send_button(drum_mappings_a[axis-1],value > -32767)
                end
            end
        end
    end
end

guitar_mappings = {0,1,3,2,4,6,7}

function button_event(device, button, value)
    local name = device.name;
    local vDev = v_devices["v"..name];
    if string.starts(name,"guitar") then
        if button == 7 then
            vDev.send_axis(7, value and -32768 or 0)
        elseif button == 8 then
            vDev.send_axis(7, value and 32768 or 0)
        else
            vDev.send_button(guitar_mappings[button+1], value)
        end
    elseif string.starts(name,"drums") then
        vDev.send_button(7-button, value)
    end
end
