gpio.mode(9, gpio.INPUT, gpio.PULLUP)
config_mode = gpio.read(9) == gpio.LOW
uart.setup(0, 9600, 8, uart.PARITY_NONE, uart.STOPBITS_1)
tm1637 = require('tm1637')
tm1637.init(4, 3)
tm1637.set_brightness(7)
m = mqtt.Client(node.chipid(), 120)
function handler(client, topic, data)
	if data == olddata then
		return
	end
	olddata = data
	tm1637.clear()
	if data ~= nil then
		tm1637.write_string(data)
	end
end
m:on("message", handler)
m:on("offline", handler)
function sub(T)
	m:connect(T.gateway, 1883, 0, function(client)
		client:subscribe("kms", 0)
	end)
end
function boot_with_config()
	_, _, gw = wifi.sta.getip()
	if gw ~= nil then
		sub({ gateway = gw })
	else
		wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, sub)
	end
end
if config_mode then
	wifi.sta.clearconfig()
	tm1637.write_string("8.8.8.8")
	enduser_setup.start(function()
		wifi.sta.autoconnect(1)
		boot_with_config()
	end)
else
	boot_with_config()
end
