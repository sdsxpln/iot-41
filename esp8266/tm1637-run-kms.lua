tm1637 = require('tm1637')
tm1637.init(4, 3)
tm1637.set_brightness(7)
m = mqtt.Client(node.chipid(), 120)
m:on("message", function(client, topic, data)
	if data == olddata then
		return
	end
	olddata = data
	tm1637.clear()
	if data ~= nil then
		tm1637.write_string(data)
	end
end)
wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, function(T)
	m:connect(T.gateway, 1883, 0, function(client)
		client:subscribe("kms", 0)
	end)
end)
