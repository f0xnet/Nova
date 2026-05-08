-- player.lua
-- Déplacement du joueur. Utilise EventBus pour réagir à des événements extérieurs.

local speed = 200  -- pixels/s, modifiable via EventBus

function init(entity)
    Log.info("Joueur initialisé (id=" .. entity.id .. ")")

    -- Un autre script peut changer la vitesse :
    --   EventBus.emit("player_set_speed", { value = 300 })
    Scene.listen("player_set_speed", function(data)
        if data and data.value then
            speed = data.value
            Log.info("Vitesse joueur → " .. speed .. " px/s")
        end
    end)
end

function update(entity, dt)
    if dialogueActive then return end

    local t = entity:getTransform()
    if not t then return end

    local dx, dy = 0, 0

    if Input.isKeyPressed("W") or Input.isKeyPressed("Up")    then dy = dy - 1 end
    if Input.isKeyPressed("S") or Input.isKeyPressed("Down")  then dy = dy + 1 end
    if Input.isKeyPressed("A") or Input.isKeyPressed("Left")  then dx = dx - 1 end
    if Input.isKeyPressed("D") or Input.isKeyPressed("Right") then dx = dx + 1 end

    -- Normalise le diagonal
    if dx ~= 0 and dy ~= 0 then
        local len = math.sqrt(dx * dx + dy * dy)
        dx, dy = dx / len, dy / len
    end

    if dx ~= 0 or dy ~= 0 then
        t.position.x = t.position.x + dx * speed * dt
        t.position.y = t.position.y + dy * speed * dt
        EventBus.emit("player_moved", { x = t.position.x, y = t.position.y })
    end
end
