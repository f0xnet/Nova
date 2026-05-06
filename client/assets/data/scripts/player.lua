-- player.lua
-- Gère le déplacement du joueur via WASD / flèches directionnelles.
-- Appelé par ScriptSystem chaque frame : update(entity, dt)

local speed = 200  -- pixels par seconde

function init(entity)
    Log.info("Joueur initialisé (id=" .. entity.id .. ", vitesse=" .. speed .. "px/s)")
end

function update(entity, dt)
    -- Le mouvement est bloqué pendant les dialogues
    if dialogueActive then return end

    local t = entity:getTransform()
    if not t then return end

    local dx, dy = 0, 0

    if Input.isKeyPressed("W") or Input.isKeyPressed("Up")    then dy = dy - 1 end
    if Input.isKeyPressed("S") or Input.isKeyPressed("Down")  then dy = dy + 1 end
    if Input.isKeyPressed("A") or Input.isKeyPressed("Left")  then dx = dx - 1 end
    if Input.isKeyPressed("D") or Input.isKeyPressed("Right") then dx = dx + 1 end

    -- Normalise le mouvement diagonal pour éviter d'aller plus vite en diagonale
    if dx ~= 0 and dy ~= 0 then
        local len = math.sqrt(dx * dx + dy * dy)
        dx = dx / len
        dy = dy / len
    end

    t.position.x = t.position.x + dx * speed * dt
    t.position.y = t.position.y + dy * speed * dt
end
