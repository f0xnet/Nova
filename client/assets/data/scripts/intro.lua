-- intro.lua
-- Séquence d'amorçage terminal Nova — Protocole Zéro.
--
-- Variables d'ajustement (timing) :
--   CHAR_DELAY : secondes entre chaque caractère (vitesse de frappe)
--   LINE_DELAY : pause entre la fin d'une ligne et le début de la suivante
--   SEP_DELAY  : pause après l'affichage d'un séparateur
--   HOLD_END   : pause finale avant la transition vers le menu
--   FADE_FINAL : durée du fondu au noir de fin d'intro
--
-- Variables d'ajustement (révélation du menu) :
--   TERMINAL_X        : position X (world) du centre du terminal dans menu.scene
--   TERMINAL_Y        : position Y (world) du centre du terminal dans menu.scene
--   TERMINAL_ZOOM     : zoom de départ — assez fort pour ne voir que l'écran noir
--   ZOOM_OUT_DURATION : durée du dezoom révélateur (secondes)
--   ZOOM_OUT_EASE     : easing du dezoom ("easeOut", "easeInOut", "linear"…)

local CHAR_DELAY = 0.015
local LINE_DELAY = 0.15
local SEP_DELAY  = 0.08
local HOLD_END   = 1.8
local FADE_FINAL = 1.2

local TERMINAL_X        = 2725       -- centre X du terminal dans menu.scene (world)
local TERMINAL_Y        = 1265       -- centre Y du terminal dans menu.scene (world)
local TERMINAL_ZOOM     = 8.0        -- zoom suffisant pour ne voir que l'écran noir du terminal
local ZOOM_OUT_DURATION = 2.5        -- durée du dezoom révélateur
local ZOOM_OUT_EASE     = "easeInOut"
-- Position caméra cible en fin de dezoom : centre logique du canvas (3840/2, 2160/2).
-- Garantit que l'entité monde (TERMINAL_X, TERMINAL_Y) coïncide avec l'élément UI
-- placé aux mêmes coordonnées dans le canvas 3840×2160 (même système de référence).
local MENU_CAM_X = 1920
local MENU_CAM_Y = 1080

-- Overlay CRT — ajuster selon le goût
local CRT_SCAN_STEP    = 6     -- px entre chaque paire de scanlines
local CRT_SCAN_HEIGHT  = 2     -- hauteur (px) de chaque ligne sombre
local CRT_SCAN_ALPHA   = 30    -- opacité des scanlines (0–255)
local CRT_VIG_PASSES   = 4     -- passes de vignettage par côté
local CRT_VIG_ALPHA    = 45    -- opacité max du vignettage
local CRT_VIG_W        = 0.18  -- largeur relative des bandes latérales
local CRT_VIG_H        = 0.14  -- hauteur relative des bandes haut/bas
local CRT_CORNER_ALPHA = 75    -- assombrissement supplémentaire aux coins

function init()
    Sound.playMusic("data/music/intro.ogg", false)
    UI.load("data/ui/json/intro.json")

    -- Masquer tous les éléments — révélés un à un par les timers
    local all_ids = {
        "ln_h1", "ln_h2", "ln_sep1",
        "ln_sys1", "ln_sys2", "ln_sys3", "ln_sys4", "ln_sep2",
        "ln_pre1", "ln_pre2", "ln_pre3", "ln_pre4", "ln_pre5", "ln_sep3",
        "ln_i1", "ln_i2", "ln_i3", "ln_i4",
        "ln_i5a", "ln_i5b", "ln_i6", "ln_sep4",
        "ln_a1", "ln_a2", "ln_a3", "ln_a4", "ln_a5", "ln_sep5",
        "ln_c1", "ln_c2", "ln_c3", "ln_c4"
    }
    for _, id in ipairs(all_ids) do
        UI.setVisible(id, false)
    end

    local cursor = 0  -- temps cumulé (quand la prochaine action peut commencer)

    local scope = Scene.scope()

    -- Affiche un élément caractère par caractère, comme une frappe au clavier
    local function typewrite(id, text)
        local start = cursor
        Timer.after(start, function()
            UI.setVisible(id, true)
            UI.setText(id, "")
        end, scope)
        for i = 1, #text do
            local partial = text:sub(1, i)
            Timer.after(start + i * CHAR_DELAY, function()
                UI.setText(id, partial)
            end, scope)
        end
        cursor = start + #text * CHAR_DELAY + LINE_DELAY
    end

    -- Affiche un séparateur instantanément, puis marque une petite pause
    local function sep(id)
        Timer.after(cursor, function() UI.setVisible(id, true) end, scope)
        cursor = cursor + SEP_DELAY
    end

    -- ── En-tête programme ─────────────────────────────────────────────
    typewrite("ln_h1",   "PORT-LUMIEL PROGRAM")
    typewrite("ln_h2",   "DIVISION DE SIMULATION COGNITIVE  //  PROTOCOLE ZERO")
    sep("ln_sep1")

    -- ── Identification système ────────────────────────────────────────
    typewrite("ln_sys1", "NOVA SYSTEM v0.1.0")
    typewrite("ln_sys2", "PREMIERE TENTATIVE D'INTEGRATION")
    typewrite("ln_sys3", "SESSION ID : DSC-NNY-0001  //  DATE : [REDACTED]")
    typewrite("ln_sys4", "CLASSIFICATION : DSC-SECRET  //  HOTE : DSC-CORE-01")
    sep("ln_sep2")

    -- ── Vérification des modules ──────────────────────────────────────
    typewrite("ln_pre1", "> VERIFICATION DES MODULES SYSTEME")
    typewrite("ln_pre2", "  MODULES COGNITIFS ............... [CHARGES]")
    typewrite("ln_pre3", "  REGISTRES MNEMIQUES ............. [VIDES]")
    typewrite("ln_pre4", "  INTERFACE SIMULATION ............ [EN ATTENTE]")
    typewrite("ln_pre5", "  ETAT GENERAL .................... [PRET]")
    sep("ln_sep3")

    -- ── Initialisation ────────────────────────────────────────────────
    typewrite("ln_i1",  "> CHARGEMENT SUBSTRAT NEURONAL ....... [OK]")
    typewrite("ln_i2",  "> INITIALISATION SIMULATION COGNITIVE . [OK]")
    typewrite("ln_i3",  "> VERIFICATION EMPREINTE MEMOIRE ...... [OK]")
    typewrite("ln_i4",  "> CALIBRAGE CANAUX DE COMMUNICATION ... [OK]")
    typewrite("ln_i5a", "> STABILITE CONSCIENCE .............. [INSTABLE]")

    -- Nova corrige sans commentaire : la ligne rouge est remplacée par la version corrigée
    local t_correct = cursor + 0.6
    Timer.after(t_correct, function()
        UI.setVisible("ln_i5a", false)
        UI.setVisible("ln_i5b", true)
    end, scope)
    cursor = t_correct + LINE_DELAY

    typewrite("ln_i6",  "> AJUSTEMENT AUTOMATIQUE — TENTATIVE 1/1 .. [OK]")
    sep("ln_sep4")

    -- ── Anomalie (rouge) — même ton que le reste, aucune emphase ──────
    typewrite("ln_a1", "> ANOMALIE DETECTEE — ENTITE NON REPERTORIEE")
    typewrite("ln_a2", "  CLASSIFICATION .......... [ RIN ]")
    typewrite("ln_a3", "  PRESENCE ................ [ SUBSTRAT INTERNE ]")
    typewrite("ln_a4", "  EVALUATION MENACE ........ [ NON CRITIQUE ]")
    typewrite("ln_a5", "  DECISION ................. [ IGNORER — CONTINUER PROTOCOLE ]")
    sep("ln_sep5")

    -- ── Lancement ─────────────────────────────────────────────────────
    typewrite("ln_c1", "PROTOCOLE ZERO — ACTIF")
    typewrite("ln_c2", "CIBLE    : PLUIE")
    typewrite("ln_c3", "OBJECTIF : EXTRACTION EMPREINTE MEMOIRE STABLE")
    typewrite("ln_c4", "INTEGRATION EN COURS...")

    -- Fin d'intro : coupe directe sur menu.scene zoomé sur le terminal, puis dezoom.
    -- Pas de fondu — l'intro et le terminal sont tous les deux noirs : la coupure est invisible.
    Timer.after(cursor + HOLD_END, function()
        -- Fade out de la musique sur toute la durée du dezoom
        Sound.fadeMusic(0, FADE_FINAL + ZOOM_OUT_DURATION)

        UI.removeUI("intro_ui")
        Scene.load("data/scenes/menu.scene", "menu")
        _G._fromIntro = true
        Camera.setPosition(TERMINAL_X, TERMINAL_Y)
        Camera.setZoom(TERMINAL_ZOOM)
        Scene.setActive("menu")   -- déclenche menu.lua:init() qui voit _fromIntro

        -- Dezoom progressif + panoramique simultané vers le centre de la vue menu.
        -- Les deux tweens sont indépendants et s'exécutent en parallèle.
        Camera.zoomTo(1.0, ZOOM_OUT_DURATION, ZOOM_OUT_EASE)
        Camera.moveTo(MENU_CAM_X, MENU_CAM_Y, ZOOM_OUT_DURATION, ZOOM_OUT_EASE)

        -- Quand le dezoom est terminé, finaliser le menu (musique, UI, boutons)
        Timer.after(ZOOM_OUT_DURATION + 0.2, function()
            _G._fromIntro = nil
            if _G._menuFinalize then
                _G._menuFinalize()
                _G._menuFinalize = nil
            end
        end)
    end, scope)
end

function update(dt)
    local ws = Viewport.getWindowSize()
    local W, H = ws.x, ws.y

    -- Scanlines horizontales
    local y = 0
    while y < H do
        Debug.fillRect(0, y, W, CRT_SCAN_HEIGHT, Color.new(0, 0, 0, CRT_SCAN_ALPHA), 0)
        y = y + CRT_SCAN_STEP
    end

    -- Vignettage latéral et vertical (passes progressives)
    local vw = W * CRT_VIG_W
    local vh = H * CRT_VIG_H
    for i = 1, CRT_VIG_PASSES do
        local a  = math.floor(CRT_VIG_ALPHA * i / CRT_VIG_PASSES)
        local fw = vw * i / CRT_VIG_PASSES
        local fh = vh * i / CRT_VIG_PASSES
        local c  = Color.new(0, 0, 0, a)
        Debug.fillRect(0,      0,      fw, H,  c, 0)  -- gauche
        Debug.fillRect(W - fw, 0,      fw, H,  c, 0)  -- droite
        Debug.fillRect(0,      0,      W,  fh, c, 0)  -- haut
        Debug.fillRect(0,      H - fh, W,  fh, c, 0)  -- bas
    end

    -- Coins sombres — suggère la courbure bombée
    local cx = W * 0.12
    local cy = H * 0.10
    local cc = Color.new(0, 0, 0, CRT_CORNER_ALPHA)
    Debug.fillRect(0,      0,      cx, cy, cc, 0)
    Debug.fillRect(W - cx, 0,      cx, cy, cc, 0)
    Debug.fillRect(0,      H - cy, cx, cy, cc, 0)
    Debug.fillRect(W - cx, H - cy, cx, cy, cc, 0)
end
