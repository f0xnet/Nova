-- nova/notify.lua
-- Notifications écran et textes flottants au-dessus des entités.
-- Disponible sans require : Notify est un global auto-chargé.
--
-- API :
--   Notify.show(text, opts)                    -- notification en haut de l'écran
--   Notify.floatingText(entity, text, opts)    -- texte flottant au-dessus d'une entité
--   Notify.clear()                             -- demande l'effacement des notifications
--
-- opts (tous optionnels) :
--   duration  number  -- secondes d'affichage (défaut: 3.0 / 1.5 pour floating)
--   color     string  -- couleur ("white", "gold", "red", "#ff8800"…)
--   size      number  -- taille relative (défaut: 1.0)
--
-- EventBus events émis (brancher l'UI dessus) :
--   "ui_notification"      { text, duration, color, size }
--   "floating_text"        { entityId, text, duration, color, size, x, y }
--   "ui_notification_clear" {}
--
-- Exemple (brancher dans le script HUD) :
--   EventBus.on("ui_notification", function(data)
--       UI.setText("hud_notif", data.text)
--       UI.showGroup("hud_notif")
--       Timer.after(data.duration, function() UI.hideGroup("hud_notif") end)
--   end)
--
--   -- Utilisation :
--   Notify.show("Quête complétée !", { color = "gold", duration = 4.0 })
--   Notify.floatingText(entity, "+50 XP", { color = "lime" })
--   Notify.show("Vie critique !", { color = "red", size = 1.4 })

local Notify = {}

local function toId(v)
    return (type(v) == "userdata") and v.id or v
end

function Notify.show(text, opts)
    opts = opts or {}
    EventBus.emit("ui_notification", {
        text     = tostring(text),
        duration = opts.duration or 3.0,
        color    = opts.color    or "white",
        size     = opts.size     or 1.0,
    })
end

function Notify.floatingText(entityOrId, text, opts)
    opts = opts or {}

    -- Récupère la position avant toId (on a besoin de l'entité userdata)
    local x, y = 0, 0
    if type(entityOrId) == "userdata" then
        local pos = entityOrId:getPosition()
        if pos then x, y = pos.x, pos.y end
    else
        local e = Registry:getEntity(entityOrId)
        if e then
            local pos = e:getPosition()
            if pos then x, y = pos.x, pos.y end
        end
    end

    EventBus.emit("floating_text", {
        entityId = toId(entityOrId),
        text     = tostring(text),
        duration = opts.duration or 1.5,
        color    = opts.color    or "white",
        size     = opts.size     or 1.0,
        x        = x,
        y        = y,
    })
end

function Notify.clear()
    EventBus.emit("ui_notification_clear", {})
end

return Notify
