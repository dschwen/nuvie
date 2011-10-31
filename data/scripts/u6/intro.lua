g_img_tbl = {}

function poll_for_esc()
	input = input_poll()
	if input ~= nil and input == 27 then
		canvas_hide()
		return true
	end
	
	return false
end

function load_images(filename, num_images)
	--FIXME create a C function to do this.
	local i
	g_img_tbl = {}
	for i=0,num_images do
	g_img_tbl[i] = image_load(filename, i)
	end
end

g_clock_tbl = {}

function load_clock()
	g_clock_tbl["h1"] = sprite_new(nil, 0xdd, 0x14, true)
	g_clock_tbl["h2"] = sprite_new(nil, 0xdd+4, 0x14, true)
	g_clock_tbl["m1"] = sprite_new(nil, 0xdd+0xa, 0x14, true)
	g_clock_tbl["m2"] = sprite_new(nil, 0xdd+0xe, 0x14, true)
end

function display_clock()
	local t = os.date("*t")
	local hour = t.hour
	local minute = t.min
	
	if hour > 12 then
		hour = hour - 12
	end
	
	if hour < 10 then
		g_clock_tbl["h1"].image = g_img_tbl[12]
	else
		g_clock_tbl["h1"].image = g_img_tbl[3]
	end
	
	if hour >= 10 then
		hour = hour - 10
	end

	g_clock_tbl["h2"].image = g_img_tbl[hour+2]

	g_clock_tbl["m1"].image = g_img_tbl[math.floor(minute / 10) + 2]
	g_clock_tbl["m2"].image = g_img_tbl[(minute % 10) + 2]
		
end

g_lounge_tbl = {}

function load_lounge()

	g_lounge_tbl["bg0"] = sprite_new(g_img_tbl[15], 210, 0, true)
	g_lounge_tbl["bg"] = sprite_new(g_img_tbl[0], 0, 0, true)
	g_lounge_tbl["bg1"] = sprite_new(g_img_tbl[1], 320, 0, true)
	g_lounge_tbl["avatar"] = sprite_new(g_img_tbl[0xd], 0, 63, true)

	load_clock()
end

function display_lounge()
	display_clock()
end

function scroll_lounge()
g_lounge_tbl.bg.x = g_lounge_tbl.bg.x - 1
g_lounge_tbl.bg1.x = g_lounge_tbl.bg1.x - 1
g_lounge_tbl.avatar.x = g_lounge_tbl.avatar.x - 2

if g_lounge_tbl.bg1.x < 166 and g_lounge_tbl.bg1.x % 2 == 0 then
	g_lounge_tbl.bg0.x = g_lounge_tbl.bg0.x - 1
end

scroll_clock()
end

function scroll_clock()
g_clock_tbl["h1"].x = g_clock_tbl["h1"].x - 1
g_clock_tbl["h2"].x = g_clock_tbl["h2"].x - 1
g_clock_tbl["m1"].x = g_clock_tbl["m1"].x - 1
g_clock_tbl["m2"].x = g_clock_tbl["m2"].x - 1
end

function hide_lounge()

	g_lounge_tbl["bg0"].visible = false
	g_lounge_tbl["bg"].visible = false
	g_lounge_tbl["bg1"].visible = false
	g_lounge_tbl["avatar"].visible = false

end

function lounge_sequence()

	load_lounge()

	canvas_set_palette("palettes.int", 1)

	--for i=0,0xff do
		--display_lounge()
		--canvas_update()
	--end
	local scroll_img = image_load("blocks.shp", 1)
	local scroll = sprite_new(scroll_img, 1, 0x9f, true)

	local x, y = image_print(scroll_img, "Upon your world, five seasons have passed since your ", 8, 312, 8, 10, 0x3e)
	image_print(scroll_img, "triumphant homecoming from Britannia.", 8, 312, x, y, 0x3e)

	local input = input_poll()

	while input == nil do
		display_lounge()
		canvas_update()
		input = input_poll()
	end

	scroll_img = image_load("blocks.shp", 2)
	x, y = image_print(scroll_img, "You have traded the Avatar's life of peril and adventure ", 7, 310, 7, 8, 0x3e)
	x, y = image_print(scroll_img, "for the lonely serenity of a world at peace. But ", 7, 310, x, y, 0x3e)
	x, y = image_print(scroll_img, "television supermen cannot take the place of friends ", 7, 310, x, y, 0x3e)
	image_print(scroll_img, "who died at your side!", 7, 310, x, y, 0x3e)

	scroll.image = scroll_img
	scroll.x = 1
	scroll.y = 0x98

	input = input_poll()

	while input == nil do
		display_lounge()
		canvas_update()
		input = input_poll()
	end

	scroll_img = image_load("blocks.shp", 0)
	image_print(scroll_img, "Outside, a chill wind rises...", 39, 200, 39, 8, 0x3e)

	scroll.image = scroll_img
	scroll.x = 0x21
	scroll.y = 0x9d

	for i=0,319 do
		scroll_lounge()
		display_lounge()
		canvas_update()
	end

	input = input_poll()
	while input == nil do
		canvas_update()
		input = input_poll()
	end

	hide_lounge()
	scroll.visible = false
end

g_window_tbl = {}
function load_window()
	local rand = math.random
	g_window_tbl["cloud_x"] = -400
	
	g_window_tbl["sky"] = sprite_new(g_img_tbl[1], 0, 0, true)
	g_window_tbl["cloud1"] = sprite_new(g_img_tbl[0], g_window_tbl["cloud_x"], -5, true)
	g_window_tbl["cloud2"] = sprite_new(g_img_tbl[0], g_window_tbl["cloud_x"], -5, true)
	g_window_tbl["clouds"] = {}
	
	local i
	for i=1,5 do
		table.insert(g_window_tbl["clouds"], sprite_new(g_img_tbl[rand(2,3)], rand(0,320) - 260, rand(0, 30), true))
	end
	
	g_window_tbl["lightning"] = sprite_new(nil, 0, 0, false)
	g_window_tbl["ground"] = sprite_new(g_img_tbl[10], 0, 0x4c, true)
	g_window_tbl["trees"] = sprite_new(g_img_tbl[8], 0, 0, true)

	g_window_tbl["strike"] = sprite_new(g_img_tbl[rand(19,23)], 158, 114, false)
	

	
	--FIXME rain here.
	local rain = {}
	local i
	for i = 0,100 do
		rain[i] = sprite_new(g_img_tbl[math.random(4,7)], math.random(0,320), math.random(0,200), false)
	end
	g_window_tbl["rain"] = rain
	
	g_window_tbl["frame"] = sprite_new(g_img_tbl[28], 0, 0, true)	
	g_window_tbl["window"] = sprite_new(g_img_tbl[26], 0x39, 0, true)
	g_window_tbl["door_left"] = sprite_new(g_img_tbl[24], 320, 0, true)
	g_window_tbl["door_right"] = sprite_new(g_img_tbl[25], 573, 0, true)
		
	g_window_tbl["flash"] = 0
	g_window_tbl["drops"] = 0
	g_window_tbl["rain_delay"] = 20
	g_window_tbl["lightning_counter"] = 0
	
end

function display_window()

	local i
	local rain = g_window_tbl["rain"]
	local rand = math.random
	local c_num

	--FIXME display clouds here.
	local cloud
	for i,cloud in ipairs(g_window_tbl["clouds"]) do
		if cloud.x > 320 then
			cloud.x = rand(0, 320) - 320
			cloud.y = rand(0, 30)
		end
		
		cloud.x = cloud.x + 2
	end

	g_window_tbl["cloud_x"] = g_window_tbl["cloud_x"] + 1
	if g_window_tbl["cloud_x"] == 320 then
		g_window_tbl["cloud_x"] = 0
	end
	
	g_window_tbl["cloud1"].x = g_window_tbl["cloud_x"]
	g_window_tbl["cloud2"].x = g_window_tbl["cloud_x"] - 320
	
	if rand(0, 6) == 0 and g_window_tbl["lightning_counter"] == 0 then --fixme var_1a, var_14
		g_window_tbl["lightning_counter"] = rand(1, 4)
		g_window_tbl["lightning"].image = g_img_tbl[rand(11,18)]
		g_window_tbl["lightning"].visible = true
		g_window_tbl["lightning_x"] = rand(-5,320)
		g_window_tbl["lightning_y"] = rand(-5,200)
	end

	if g_window_tbl["lightning_counter"] > 0 then
		g_window_tbl["lightning"].x = g_window_tbl["lightning_x"] + rand(0, 3)
		g_window_tbl["lightning"].y = g_window_tbl["lightning_y"] + rand(0, 3)
	end

	if (g_window_tbl["lightning_counter"] > 0 and rand(0,3) == 0) or g_window_tbl["strike"].visible == true then
		canvas_set_palette_entry(0x58, 0x40,0x94,0xfc)
		canvas_set_palette_entry(0x5a, 0x40,0x94,0xfc)
		canvas_set_palette_entry(0x5c, 0x40,0x94,0xfc)
	else
		canvas_set_palette_entry(0x58, 0x20,0xa4,0x80)
		canvas_set_palette_entry(0x5a, 0x14,0x8c,0x74)
		canvas_set_palette_entry(0x5c, 0x0c,0x74,0x68)
	end
	
	if rand(0,1) == 0 then
		g_window_tbl["strike"].image = g_img_tbl[rand(19,23)]
	end
	
	if g_window_tbl["flash"] > 0 then
		g_window_tbl["flash"] = g_window_tbl["flash"] - 1
	else
		if rand(0,5) == 0 or g_window_tbl["strike"].visible == true then
			g_window_tbl["window"].image = g_img_tbl[27]
			g_window_tbl["flash"] = rand(1, 5)
		else
			g_window_tbl["window"].image = g_img_tbl[26]
		end
	end

	if g_window_tbl["drops"] < 100 then
		if rand(0,g_window_tbl["rain_delay"]) == 0 then
			g_window_tbl["rain_delay"] = g_window_tbl["rain_delay"] - 2
			if g_window_tbl["rain_delay"] < 1 then
				g_window_tbl["rain_delay"] = 1
			end
			i = g_window_tbl["drops"]
			rain[i].visible = true
			rain[i].image = g_img_tbl[rand(4,7)]
			rain[i].y = -4
			rain[i].x = rand(0,320)
			g_window_tbl["drops"] = i + 1
		end
	end
	
	for i = 0,g_window_tbl["drops"] do

		if rain[i].visible == true then
			rain[i].x = rain[i].x + 2
			rain[i].y = rain[i].y + 8
			if rain[i].x > 320 or rain[i].y > 200 then
				rain[i].visible = false
			end
		else
			rain[i].visible = true
			rain[i].image = g_img_tbl[rand(4,7)]
			rain[i].y = -4
			rain[i].x = rand(0,320)
		end
	end
	
	if g_window_tbl["lightning_counter"] > 0 then
		g_window_tbl["lightning_counter"] = g_window_tbl["lightning_counter"] - 1
	end
end

function window_sequence()
	load_images("intro_2.shp", 0x1c)
	
	load_window()
	
	canvas_set_palette("palettes.int", 2)
	
	local i = 0
	local input = input_poll()
	while input == nil and i < 20 do
		display_window()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			break
		end
		
		i = i + 1
		canvas_update()
	end
	
	local scroll_img = image_load("blocks.shp", 1)
	local scroll = sprite_new(scroll_img, 1, 0x98, true)
	
	local x, y = image_print(scroll_img, "...and in moments, the storm is upon you.", 8, 312, 36, 14, 0x3e)

	local input = input_poll()
	while input == nil do
		display_window()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			break
		end
		canvas_update()
	end
	
	scroll_img = image_load("blocks.shp", 1)
	x, y = image_print(scroll_img, "Tongues of lightning lash the sky, conducting an unceasing ", 8, 310, 8, 10, 0x3e)
	image_print(scroll_img, "crescendo of thunder....", 8, 310, x, y, 0x3e)
	scroll.image = scroll_img
	
	input = null
	while input == nil do
		display_window()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			break
		end
	canvas_update()
	end

	scroll_img = image_load("blocks.shp", 1)
	x, y = image_print(scroll_img, "In a cataclysm of sound and light, a bolt of searing ", 8, 310, 8, 10, 0x3e)
	image_print(scroll_img, "blue fire strikes the earth!", 8, 310, x, y, 0x3e)
	scroll.image = scroll_img
	
	g_window_tbl["strike"].visible = true
		
	input = null
	while input == nil do
		display_window()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			break
		end
	canvas_update()
	end
	
	scroll_img = image_load("blocks.shp", 1)
	x, y = image_print(scroll_img, "Lightning among the stones!", 8, 310, 73, 10, 0x3e)
	image_print(scroll_img, "Is this a sign from distant Britannia?", 8, 310, 41, 18, 0x3e)
	scroll.image = scroll_img
		
	--fixme scroll window here.
	input = null
	i = 0
	while input == nil and i < 320 do

		display_window()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			break
		end
	
		canvas_update()
		input = input_poll()
	
		g_window_tbl["window"].x = g_window_tbl["window"].x - 2
		g_window_tbl["frame"].x = g_window_tbl["frame"].x - 2
		g_window_tbl["door_left"].x = g_window_tbl["door_left"].x - 2
		g_window_tbl["door_right"].x = g_window_tbl["door_right"].x - 2
		i = i + 2
		if i > 160 and g_window_tbl["strike"].visible == true then
			g_window_tbl["strike"].visible = false
		end
	end
	
	input = null
	while input == nil do
		display_window()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			break
		end
		canvas_update()
	end
	
	scroll.visible = false
	input = null
	i = 0
	while input == nil and i < 68 do
	
		display_window()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			break
		end
	
		canvas_update()
		input = input_poll()
	
		g_window_tbl["door_left"].x = g_window_tbl["door_left"].x - 1
		g_window_tbl["door_right"].x = g_window_tbl["door_right"].x + 1
		i = i + 1
	end

	scroll_img = image_load("blocks.shp", 2)
	x, y = image_print(scroll_img, "You bolt from your house, stumbling, running blind in the", 7, 310, 8, 12, 0x3e)
	x, y = image_print(scroll_img, " storm. Into the forest, down the path, through the ", 7, 310, x, y, 0x3e)
	image_print(scroll_img, "rain... to the stones.", 7, 310, x, y, 0x3e)
	scroll.image = scroll_img
	scroll.visible = true

	input = null
	while input == nil do
		display_window()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			break
		end
		canvas_update()
	end

	
end

load_images("intro_1.shp", 0x46)
music_play("bootup.m")
--[ [
canvas_set_palette("palettes.int", 0)

local img = g_img_tbl[0x45]

local background = sprite_new(img)

background.x = 0
background.y = 0
background.opacity = 0
background.visible = true

music_play("bootup.m")
for i=0,0xff,3 do
	background.opacity = i
	if poll_for_esc() == true then return end
	canvas_update()
end

for i=0xff,0,-3 do
background.opacity = i
	if poll_for_esc() == true then return end

--if input ~= nil and input == ESCAPE_KEY then
--	return
--end
canvas_update()
end

img = g_img_tbl[0x46]

background.image = img

for i=0,0xff,3 do
	background.opacity = i
		if poll_for_esc() == true then return end

	canvas_update()
end

for i=0xff,0,-3 do
background.opacity = i
	if poll_for_esc() == true then return end

canvas_update()
end

background.visible = false



lounge_sequence()
--] ]
window_sequence()

canvas_hide()