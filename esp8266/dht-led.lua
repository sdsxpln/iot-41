dofile("config.lua")
wifi.setmode(wifi.STATION)
wifi.setphymode(wifi.PHYMODE_N)
wifi.sta.config(wifi_ap_name, wifi_password)

gpio.mode(ledpin, gpio.OUTPUT)
gpio.write(ledpin, gpio.LOW)

id = node.chipid()
m = mqtt.Client(id, 120, "", "")
conn = false
topicbase = "device/esp_" .. id .. "_"

function tick()
	if wifi.sta.status() ~= 5 then
		conn = false
		return
	end
	if not conn then
		m:connect(mqtt_ip, 1883)
		return
	end
	s, t, h, td, hd = dht.read(dhtpin)
	if s == dht.OK then
		m:publish(topicbase .. "temp/stat", t .. "." .. td, 0, 0)
		m:publish(topicbase .. "humi/stat", h .. "." .. hd, 0, 0)
	end
	m:publish(topicbase .. "adc/stat", adc.read(0), 0, 0)
	m:publish(topicbase .. "heap/stat", node.heap(), 0, 0)
	m:publish(topicbase .. "df/stat", file.fsinfo(), 0, 0)
end

m:on("connect", function(c)
	conn = true
	m:subscribe(topicbase .. "led/cmd", 0)
	tick()
end)

m:on("offline", function(c)
	m:close()
	conn = false
end)

m:on("message", function(c, topic, data)
	gpio.write(ledpin, (data == "ON" and gpio.HIGH or gpio.LOW))
	m:publish(topicbase .. "led/stat", (gpio.read(ledpin) ~= gpio.LOW and "ON" or "OFF"), 0, 0)
end)

tmr.alarm(0, 2000, tmr.ALARM_SINGLE, tick)
tmr.alarm(1, 60000, tmr.ALARM_AUTO, tick)
