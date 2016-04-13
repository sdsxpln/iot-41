dofile("config.lua")
wifi.setmode(wifi.STATION)
wifi.setphymode(wifi.PHYMODE_N)
wifi.sta.config(wifi_ap_name, wifi_password)

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
	h1000, t100 = bme280.humi()
	if h1000 ~= nil then
		m:publish(topicbase .. "humi/stat", (h1000 / 1000) .. "." .. (h1000 % 1000), 0, 0)
		m:publish(topicbase .. "temp/stat", (t100 / 100) .. "." .. (t100 % 100), 0, 0)
	end
	p1000 = bme280.baro()
	if p1000 ~= nil then
		m:publish(topicbase .. "baro/stat", (p1000 / 1000) .. "." .. (p1000 % 1000), 0, 0)
	end
	m:publish(topicbase .. "adc/stat", adc.read(0), 0, 0)
	m:publish(topicbase .. "heap/stat", node.heap(), 0, 0)
	m:publish(topicbase .. "df/stat", file.fsinfo(), 0, 0)
end

m:on("connect", function(c)
	conn = true
	tick()
end)

m:on("offline", function(c)
	m:close()
	conn = false
end)

uart.setup(0, 9600, 8, uart.PARITY_NONE, uart.STOPBITS_1, 1)
-- sda, scl
bme280.init(3, 4);
tmr.alarm(0, 2000, tmr.ALARM_SINGLE, tick)
tmr.alarm(1, 60000, tmr.ALARM_AUTO, tick)
