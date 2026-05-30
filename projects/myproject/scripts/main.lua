-- Planet Engine Demo
Map.load("maps/demo.map")

function onStart()
    for i = -4, 4 do
        local wall = Map.createObject("wall", i * 3.0, 0, -4)
    end

    for i = -4, 4, 2 do
        Map.createObject("pillar", i * 3.0, 0, -4)
    end

    Map.createObject("sphere", -2, 1.5, 2)
    Map.createObject("sphere", 2, 1.5, 2)
end

function onUpdate(dt)
end

function onDraw()
end
