math.randomseed(os.time())

dexeclInvoke("dexecl/config.dexl")

local CELL = Dexecl.getVar("cell_size", 3.0)
local WALL_H = Dexecl.getVar("wall_height", 3.0)
local WALL_T = Dexecl.getVar("wall_thickness", 0.15)

local p = {
    x = 0.5, y = 1.6, z = 0.5,
    yaw = 0, pitch = 0,
    speed = Dexecl.getVar("player_speed", 6.0),
    sens = Dexecl.getVar("player_sensitivity", 0.002),
    radius = Dexecl.getVar("player_radius", 0.3),
    hp = Dexecl.getVar("player_max_hp", 100),
    maxHp = Dexecl.getVar("player_max_hp", 100)
}

local cells = {}
local walls = {}
local exitX, exitZ = 0, 0
local floorEntity = nil
local ceilingEntity = nil
local mouseLocked = true
local won = false
local startTime = 0
local level = 1
local numCells = 36
local AREA_W = 0
local AREA_D = 0

local ladderEntities = {}

local hpAlpha = 0
local hpTarget = 0
local hpTimer = 0
local lastHp = 100

local darkLevel = false
local wallGreen = 1

local enemies = {}

local TRANS_INACTIVE = -1
local TRANS_BLACK = 0
local transState = TRANS_INACTIVE
local transTimer = 0

local winTimer = -1

function lerp(a, b, t)
    return a + (b - a) * t
end

Console.addCommand("spawn_enemy", "do", "", "Spawn an enemy near the player", function(args)
    spawnEnemyAtPlayer()
end)

Console.addCommand("goto_level", "do", "<level>", "Jump to a specific sublevel", function(args)
    local target = tonumber(args)
    if target and target >= 1 then
        level = target
        transState = TRANS_BLACK
        transTimer = 0
    end
end)

function onStart()
    Renderer.clearColor(0.02, 0.02, 0.02, 1)
    Camera.setFOV(75)
    Camera.setMouseLock(true)
    Engine.setWindowTitle("Sublevel " .. level)
    Engine.setWindowIcon("assets/icon.png")
    Renderer.setPostEffect("none")
    Renderer.setPostIntensity(0.5)
    newLevel()
    for _, e in ipairs(ladderEntities) do
        ECS.entitySetActive(e, true)
    end
end

function darkLevelChance()
    local base = Dexecl.getVar("dark_level_chance_base", 0.07)
    local growth = Dexecl.getVar("dark_level_chance_growth", 0.005)
    return math.random() < base * (1 + level * growth)
end

function getEnemyCount()
    local n = 1
    if math.random(2) == 1 then n = n + 1 end
    if math.random(4) == 1 then n = n + 1 end
    if math.random(6) == 1 then n = n + 1 end
    if math.random(8) == 1 then n = n + 1 end
    return n
end

function enemyChance()
    local denom = Dexecl.getVar("enemy_spawn_chance_denom", 7)
    return math.random() < (1 / denom) * (1 + level / 100)
end

function findDeadEnds()
    local deads = {}
    for i, c in ipairs(cells) do
        local count = 0
        for _ in pairs(c.connections) do count = count + 1 end
        if count == 1 then
            deads[#deads + 1] = i
        end
    end
    return deads
end

function getDeadEndDirection(cellIdx)
    local c = cells[cellIdx]
    for nbId in pairs(c.connections) do
        local dx = cells[nbId].x - c.x
        local dz = cells[nbId].z - c.z
        return math.atan2(dx, -dz)
    end
    return 0
end

function newLevel()
    won = false
    enemies = {}
    walls = {}
    p.hp = p.maxHp
    lastHp = p.maxHp
    Renderer.clearPointLights()

    local startCells = Dexecl.getVar("level_start_cells", 36)
    local perLevel = Dexecl.getVar("level_cells_per_level", 2)
    local minCells = Dexecl.getVar("level_min_cells", 36)
    local maxCells = Dexecl.getVar("level_max_cells", 400)
    numCells = math.floor(startCells + level * perLevel)
    numCells = math.max(minCells, math.min(maxCells, numCells))

    local c = CELL
    local cols = math.ceil(math.sqrt(numCells))
    local rows = math.ceil(numCells / cols)
    AREA_W = cols * c
    AREA_D = rows * c

    darkLevel = darkLevelChance()
    wallGreen = 1

    local t = math.min(1, level / 100)
    local ambient_r = lerp(Dexecl.getVar("ambient_r_start", 0.35), Dexecl.getVar("ambient_r_end", 0.06), t)
    local ambient_g = lerp(Dexecl.getVar("ambient_g_start", 0.38), Dexecl.getVar("ambient_g_end", 0.06), t)
    local ambient_b = lerp(Dexecl.getVar("ambient_b_start", 0.45), Dexecl.getVar("ambient_b_end", 0.08), t)
    local light_r = lerp(Dexecl.getVar("light_r_start", 1.0), Dexecl.getVar("light_r_end", 0.15), t)
    local light_g = lerp(Dexecl.getVar("light_g_start", 0.95), Dexecl.getVar("light_g_end", 0.12), t)
    local light_b = lerp(Dexecl.getVar("light_b_start", 0.85), Dexecl.getVar("light_b_end", 0.10), t)
    local fog_density = lerp(Dexecl.getVar("fog_density_start", 0), Dexecl.getVar("fog_density_end", 0.06), t)

    if darkLevel then
        wallGreen = math.min(1, math.sqrt(level) * 50 / 255)
        ambient_r = math.max(ambient_r, 0.08)
        ambient_g = math.max(ambient_g, 0.08)
        ambient_b = math.max(ambient_b, 0.10)
        Renderer.setFogColor(0, 0, 0)
        Renderer.setFogDensity(fog_density)
        Renderer.setFogEnabled(true)
    else
        if fog_density > 0.01 then
            Renderer.setFogColor(0.02, 0.02, 0.02)
            Renderer.setFogDensity(fog_density)
            Renderer.setFogEnabled(true)
        else
            Renderer.setFogEnabled(false)
        end
    end

    Renderer.setAmbientColor(ambient_r, ambient_g, ambient_b)
    Renderer.setLightColor(light_r, light_g, light_b)
    Renderer.setLightDir(0.5, -0.8, -0.3)

    local spawnCellIdx, exitCellIdx
    for attempt = 1, 50 do
        walls = {}
        generate(c)

        local deadEnds = findDeadEnds()
        if #deadEnds < 2 then
            deadEnds = {}
            for i = 2, #cells do
                deadEnds[#deadEnds + 1] = i
            end
        end

        local exIdx = math.random(#deadEnds)
        exitCellIdx = deadEnds[exIdx]
        table.remove(deadEnds, exIdx)

        spawnCellIdx = deadEnds[math.random(#deadEnds)]

        local visited = {}
        local queue = {spawnCellIdx}
        visited[spawnCellIdx] = true
        while #queue > 0 do
            local cur = table.remove(queue, 1)
            for nb in pairs(cells[cur].connections) do
                if not visited[nb] then
                    visited[nb] = true
                    queue[#queue + 1] = nb
                end
            end
        end

        if visited[exitCellIdx] then break end
        if attempt == 50 then Engine.log("Warning: maze unsolvable after 50 attempts") end
    end

    exitX = cells[exitCellIdx].x
    exitZ = cells[exitCellIdx].z

    p.x = cells[spawnCellIdx].x
    p.z = cells[spawnCellIdx].z
    p.pitch = 0
    p.yaw = getDeadEndDirection(spawnCellIdx)

    buildWorld(c)
    for _, e in ipairs(ladderEntities) do
        ECS.entitySetActive(e, false)
    end

    if enemyChance() then
        local count = getEnemyCount()
        for i = 1, count do
            local cx = math.random() * AREA_W
            local cz = math.random() * AREA_D
            cx = math.max(WALL_T, math.min(AREA_W - WALL_T, cx))
            cz = math.max(WALL_T, math.min(AREA_D - WALL_T, cz))
            spawnEnemy(cx, cz)
        end
    end

    startTime = Engine.getElapsedTime()
    Engine.log("Sublevel " .. level .. " (" .. #cells .. " cells)" .. (darkLevel and " [DARK]" or ""))
end

function spawnEnemy(ex, ez)
    local e = ECS.createEntity("enemy")
    ECS.entitySetPosition(e, ex, 1.0, ez)
    ECS.addMeshWallQuad(e, 2, 2)
    ECS.entitySetTexture(e, "enemy.png")
    ECS.entitySetColor(e, 1, 1, 1)
    ECS.entitySetUnlit(e, true)
    local lightIdx = Renderer.addPointLight(ex, 1.0, ez, 1, 0.2, 0.2, 3.0)
    enemies[#enemies + 1] = {entity = e, x = ex, z = ez, hitCooldown = 0, lightIdx = lightIdx}
end

function spawnEnemyAtPlayer()
    local angle = math.random() * math.pi * 2
    local dist = 2 + math.random() * 3
    local ex = p.x + math.cos(angle) * dist
    local ez = p.z + math.sin(angle) * dist
    ex = math.max(WALL_T, math.min(AREA_W - WALL_T, ex))
    ez = math.max(WALL_T, math.min(AREA_D - WALL_T, ez))
    spawnEnemy(ex, ez)
    Engine.log("Spawned enemy near player")
end

function generate(c)
    local cols = math.ceil(math.sqrt(numCells))
    local rows = math.ceil(numCells / cols)

    cells = {}
    local idx = 1
    for r = 1, rows do
        for c2 = 1, cols do
            if idx > numCells then break end
            cells[idx] = {
                x = (c2 - 0.5) * c,
                z = (r - 0.5) * c,
                connections = {}
            }
            idx = idx + 1
        end
    end

    local edges = {}
    for i = 1, #cells do
        for j = i + 1, #cells do
            local dx = math.abs(cells[i].x - cells[j].x)
            local dz = math.abs(cells[i].z - cells[j].z)
            if math.max(dx, dz) < c * 1.1 and math.min(dx, dz) < 0.1 then
                edges[#edges + 1] = {i, j, math.random()}
            end
        end
    end

    table.sort(edges, function(a, b) return a[3] < b[3] end)

    local parent = {}
    for i = 1, #cells do parent[i] = i end
    local function find(x)
        while parent[x] ~= x do
            parent[x] = parent[parent[x]]
            x = parent[x]
        end
        return x
    end

    for _, e in ipairs(edges) do
        local ra, rb = find(e[1]), find(e[2])
        if ra ~= rb then
            parent[ra] = rb
            cells[e[1]].connections[e[2]] = true
            cells[e[2]].connections[e[1]] = true
        end
    end

    for _, e in ipairs(edges) do
        if not cells[e[1]].connections[e[2]] then
            local a, b = cells[e[1]], cells[e[2]]
            local mx = (a.x + b.x) * 0.5
            local mz = (a.z + b.z) * 0.5
            local dw = math.abs(a.x - b.x)
            local dd = math.abs(a.z - b.z)
            if dd > dw then
                addWall("w" .. _ .. "_h", mx, mz, c, WALL_H, WALL_T)
            else
                addWall("w" .. _ .. "_v", mx, mz, WALL_T, WALL_H, c)
            end
        end
    end

    local overlap = 0.05
    local inset = 0.01
    local perimW = c + overlap
    local perimD = c + overlap
    for _, cell in ipairs(cells) do
        local hasN, hasS, hasW, hasE = false, false, false, false
        for _, other in ipairs(cells) do
            local dx, dz = other.x - cell.x, other.z - cell.z
            if math.abs(dx) < 0.1 and math.abs(dz + c) < 0.1 then hasN = true end
            if math.abs(dx) < 0.1 and math.abs(dz - c) < 0.1 then hasS = true end
            if math.abs(dz) < 0.1 and math.abs(dx + c) < 0.1 then hasW = true end
            if math.abs(dz) < 0.1 and math.abs(dx - c) < 0.1 then hasE = true end
        end
        if not hasN then addWall("bn_" .. _, cell.x, cell.z - c * 0.5 + inset, perimW, WALL_H, WALL_T) end
        if not hasS then addWall("bs_" .. _, cell.x, cell.z + c * 0.5 - inset, perimW, WALL_H, WALL_T) end
        if not hasW then addWall("bw_" .. _, cell.x - c * 0.5 + inset, cell.z, WALL_T, WALL_H, perimD) end
        if not hasE then addWall("be_" .. _, cell.x + c * 0.5 - inset, cell.z, WALL_T, WALL_H, perimD) end
    end
end

function buildWorld(c)
    local planeSize = 200

    floorEntity = ECS.createEntity("floor")
    ECS.addMeshPlane(floorEntity, planeSize, planeSize)
    ECS.entitySetColor(floorEntity, 0, 0, 0)
    ECS.entitySetUnlit(floorEntity, true)

    ceilingEntity = ECS.createEntity("ceiling")
    ECS.addMeshPlane(ceilingEntity, planeSize, planeSize)
    ECS.entitySetColor(ceilingEntity, 0, 0, 0)
    ECS.entitySetUnlit(ceilingEntity, true)

    ladderEntities = {}
    local ladderW = 0.8
    local ladderH = WALL_H * 0.85
    local railT = 0.06
    local rungT = 0.04
    local rungGap = 0.3
    local metal = {0.6, 0.6, 0.65}

    local lr = ECS.createEntity("ladder_l")
    ECS.entitySetPosition(lr, exitX - ladderW * 0.5 + railT * 0.5, ladderH * 0.5, exitZ)
    ECS.addMeshBoxEx(lr, railT, ladderH, railT)
    ECS.entitySetColor(lr, metal[1], metal[2], metal[3])
    ECS.entitySetUnlit(lr, not darkLevel)
    ladderEntities[#ladderEntities + 1] = lr

    local rr = ECS.createEntity("ladder_r")
    ECS.entitySetPosition(rr, exitX + ladderW * 0.5 - railT * 0.5, ladderH * 0.5, exitZ)
    ECS.addMeshBoxEx(rr, railT, ladderH, railT)
    ECS.entitySetColor(rr, metal[1], metal[2], metal[3])
    ECS.entitySetUnlit(rr, not darkLevel)
    ladderEntities[#ladderEntities + 1] = rr

    local num = math.floor(ladderH / rungGap)
    for i = 1, num do
        local ry = i * rungGap
        local rung = ECS.createEntity("rung_" .. i)
        ECS.entitySetPosition(rung, exitX, ry, exitZ)
        ECS.addMeshBoxEx(rung, ladderW - railT * 2, rungT, railT)
        ECS.entitySetColor(rung, metal[1], metal[2], metal[3])
        ECS.entitySetUnlit(rung, not darkLevel)
        ladderEntities[#ladderEntities + 1] = rung
    end

    Engine.log("Built " .. #walls .. " walls, " .. #cells .. " cells")
end

local wallDist = {
    {lvl=0,   wall=100, laminated=0,  dark=0,  blue=0, sad=0},
    {lvl=25,  wall=80,  laminated=15, dark=0,  blue=0, sad=0},
    {lvl=50,  wall=60,  laminated=20, dark=2,  blue=0, sad=10},
    {lvl=75,  wall=40,  laminated=25, dark=3.5, blue=2.5, sad=25},
    {lvl=100, wall=20,  laminated=20, dark=5,  blue=5, sad=50},
}

function pickWallTexture()
    if darkLevel then return "wall_dark" end
    local keys = wallDist
    local t = math.min(100, level)
    local i = 1
    while i < #keys and keys[i+1].lvl <= t do i = i + 1 end
    local a, b = keys[i], keys[#keys]
    if i < #keys then b = keys[i+1] end
    local f = (b.lvl == a.lvl) and 0 or (t - a.lvl) / (b.lvl - a.lvl)
    local function lerpA(key) return a[key] + (b[key] - a[key]) * f end
    local wallW = lerpA("wall")
    local lamW  = lerpA("laminated")
    local darkW = lerpA("dark")
    local blueW = lerpA("blue")
    local sadW  = lerpA("sad")
    local total = wallW + lamW + darkW + blueW + sadW
    local r = math.random() * total
    if r < wallW then return "wall" end
    r = r - wallW
    if r < lamW then return "wall_laminated" end
    r = r - lamW
    if r < darkW then return "wall_dark" end
    r = r - darkW
    if r < blueW then return "wall_blue" end
    return "wall_sad"
end

function addWall(name, cx, cz, w, h, d)
    local e = ECS.createEntity(name)
    ECS.entitySetPosition(e, cx, h * 0.5, cz)
    local visW = math.max(math.max(w, d), h)
    ECS.addMeshWallQuad(e, visW, h)
    if d > w then
        ECS.entitySetRotation(e, 0, 90, 0)
    end
    local tex = pickWallTexture()
    ECS.entitySetTexture(e, tex .. ".png")
    if darkLevel then
        ECS.entitySetColor(e, wallGreen, wallGreen, wallGreen)
    else
        ECS.entitySetColor(e, 1, 1, 1)
    end
    ECS.entitySetUnlit(e, true)

    local hw = w * 0.5
    local hd = d * 0.5
    local isIllusion = tex == "wall_dark" and not darkLevel
    if not isIllusion then
        walls[#walls + 1] = {x1 = cx - hw, z1 = cz - hd, x2 = cx + hw, z2 = cz + hd}
    end
end

function collides(nx, nz, r)
    local radius = r or p.radius
    for _, w in ipairs(walls) do
        if nx + radius > w.x1 and nx - radius < w.x2 and nz + radius > w.z1 and nz - radius < w.z2 then
            return true
        end
    end
    if nx - radius < -WALL_T or nx + radius > AREA_W + WALL_T or
       nz - radius < -WALL_T or nz + radius > AREA_D + WALL_T then
        return true
    end
    return false
end

function onUpdate(dt)
    if Input.getKeyDown("escape") then Engine.quit(); return end

    if Input.getKeyDown("tab") then
        mouseLocked = not mouseLocked
        Camera.setMouseLock(mouseLocked)
    end

    if winTimer >= 0 then
        winTimer = winTimer + dt
        if winTimer > 4.0 then
            Engine.quit()
        end
        return
    end

    if transState ~= TRANS_INACTIVE then
        transTimer = transTimer + dt

        if transTimer >= 2.0 then
            Scene.clear()
            newLevel()
            Camera.setPosition(p.x, p.y, p.z)
            local lx = p.x + math.sin(p.yaw) * math.cos(p.pitch) * 10
            local ly = p.y + math.sin(p.pitch) * 10
            local lz = p.z - math.cos(p.yaw) * math.cos(p.pitch) * 10
            Camera.setTarget(lx, ly, lz)
            for _, e in ipairs(ladderEntities) do
                ECS.entitySetActive(e, true)
            end
            transState = TRANS_INACTIVE
            transTimer = 0
        end

        return
    end

    if floorEntity then
        ECS.entitySetPosition(floorEntity, p.x, -0.05, p.z)
        ECS.entitySetPosition(ceilingEntity, p.x, WALL_H + 0.05, p.z)
    end

    if mouseLocked then
        local dx, _ = Input.getMouseDelta()
        p.yaw = p.yaw + dx * p.sens
    end

    local spd = p.speed

    local forward_x = math.sin(p.yaw)
    local forward_z = -math.cos(p.yaw)
    local right_x = math.cos(p.yaw)
    local right_z = math.sin(p.yaw)

    local mx, mz = 0, 0
    if Input.getKey("w") then mx = mx + forward_x; mz = mz + forward_z end
    if Input.getKey("s") then mx = mx - forward_x; mz = mz - forward_z end
    if Input.getKey("a") then mx = mx - right_x; mz = mz - right_z end
    if Input.getKey("d") then mx = mx + right_x; mz = mz + right_z end

    local len = math.sqrt(mx * mx + mz * mz)
    if len > 0 and not Engine.getNoclip() then
        mx = mx / len * spd * dt
        mz = mz / len * spd * dt
        if not collides(p.x + mx, p.z) then p.x = p.x + mx end
        if not collides(p.x, p.z + mz) then p.z = p.z + mz end
    elseif len > 0 then
        p.x = p.x + mx / len * spd * dt
        p.z = p.z + mz / len * spd * dt
    end

    if Engine.getNoclip() then
        p.y = p.y + Engine.getNoclipVerticalDelta(dt, spd)
    elseif p.y ~= 1.6 then
        p.y = 1.6
    end

    local enemySpeed = p.speed * Dexecl.getVar("enemy_speed_mult", 0.5)

    for i = #enemies, 1, -1 do
        local e = enemies[i]
        local dx = p.x - e.x
        local dz = p.z - e.z
        local dist = math.sqrt(dx * dx + dz * dz)

        if dist > 0.1 then
            local mx = dx / dist * enemySpeed * dt
            local mz = dz / dist * enemySpeed * dt
            if not collides(e.x + mx, e.z, 0.3) then e.x = e.x + mx end
            if not collides(e.x, e.z + mz, 0.3) then e.z = e.z + mz end
            ECS.entitySetPosition(e.entity, e.x, 1.0, e.z)
            Renderer.setPointLightPosition(e.lightIdx, e.x, 1.0, e.z)
        end

        local dx = p.x - e.x
        local dy = p.y - 1.0
        local dz = p.z - e.z
        local hdist = math.sqrt(dx * dx + dz * dz)
        local pitch = -math.atan2(dy, hdist)
        local yaw = math.atan2(dx, dz)
        ECS.entitySetRotation(e.entity, pitch * 180 / math.pi, yaw * 180 / math.pi, 0)

        local hitRange = Dexecl.getVar("enemy_hit_range", 0.8)
        local hitDmg = Dexecl.getVar("enemy_hit_damage", 15)
        local hitCd = Dexecl.getVar("enemy_hit_cooldown", 0.3)
        if dist < hitRange and e.hitCooldown <= 0 then
            p.hp = p.hp - hitDmg
            e.hitCooldown = hitCd
        end
        if e.hitCooldown > 0 then
            e.hitCooldown = e.hitCooldown - dt
        end
    end

    if p.hp <= 0 then
        Engine.quit()
        return
    end

    local lookX = p.x + math.sin(p.yaw) * math.cos(p.pitch) * 10
    local lookY = p.y + math.sin(p.pitch) * 10
    local lookZ = p.z - math.cos(p.yaw) * math.cos(p.pitch) * 10
    Camera.setPosition(p.x, p.y, p.z)
    Camera.setTarget(lookX, lookY, lookZ)

    if p.hp < p.maxHp then
        local regen = Dexecl.getVar("player_hp_regen", 8)
        p.hp = math.min(p.maxHp, p.hp + regen * dt)
    end

    if math.abs(p.hp - lastHp) > 0.5 then
        hpTarget = 1
        hpTimer = 0
        lastHp = p.hp
    end
    if p.hp >= p.maxHp and hpAlpha >= 0.99 then
        hpTarget = 0
    end
    hpTimer = hpTimer + dt
    if hpTimer > 2.0 then
        hpTarget = 0
    end
    if hpTarget > hpAlpha then
        hpAlpha = math.min(1, hpAlpha + dt * 4)
    else
        hpAlpha = math.max(0, hpAlpha - dt * 2)
    end

    if not won then
        local dist = math.sqrt((p.x - exitX) ^ 2 + (p.z - exitZ) ^ 2)
        if dist < 0.6 then
            if level >= 100 then
                won = true
                winTimer = 0
            else
                won = true
                level = level + 1
                transState = TRANS_BLACK
                transTimer = 0
            end
        end
    end
end

function onDraw()
    if winTimer >= 0 then
        Renderer.drawRect(0, 0, 1280, 720, 0, 0, 0, 1)
        Renderer.drawText("YOU WIN", 1280 / 2 - 120, 720 / 2 - 30, 2.0, 0, 1, 0, 1)
        return
    end

    if hpAlpha > 0.01 then
        local bw = 400
        local bh = 20
        local bx = (1280 - bw) * 0.5
        local by = 720 - 40
        local ratio = p.hp / p.maxHp
        local r, g = 1 - ratio, ratio

        Renderer.drawRect(bx, by, bw, bh, 0.2, 0.2, 0.2, hpAlpha * 0.6)
        Renderer.drawRect(bx, by, bw * ratio, bh, r, g, 0.2, hpAlpha)
    end

    if transState == TRANS_BLACK then
        Renderer.drawRect(0, 0, 1280, 720, 0, 0, 0, 1)
    end

    Renderer.drawText("Sublevel " .. level, 10, 10, 0.5, 1, 1, 1, 0.6)
end
