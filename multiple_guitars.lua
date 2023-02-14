devices = {
    guitar0 = {
        type = "wii",
        extension_type = "Guitar",
    },
    guitar1 = {
        type = "wii",
        extension_type = "Guitar",
    },
    guitar2 = {
        type = "wii",
        extension_type = "Guitar",
    },
    guitar3 = {
        type = "wii",
        extension_type = "Guitar",
    }
}
v_devices = {
    vguitar0 = {
        buttons = 11,
        axes = 8
    },
    vguitar1 = {
        buttons = 11,
        axes = 8
    },
    vguitar2 = {
        buttons = 11,
        axes = 8
    },
    vguitar3 = {
        buttons = 11,
        axes = 8
    }
}
function disconnect_event(device)
    local name = device.name;
    local vDev = v_devices["v"..name];
    if string.starts(name,"guitar") then
        vDev.send_button(7,true)
        vDev.send_button(7,false)
    end
end

function axis_event(device, axis, value)
    local name = device.name;
    local vDev = v_devices["v"..name];
    if string.starts(name,"guitar") then
        if device.type == "Accelerometer" then
            if axis == 1 then
                vDev.send_axis(4,value)
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
                vDev.send_axis(3,0)
            else
                vDev.send_axis(3,value + 32767)
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
    end
end
