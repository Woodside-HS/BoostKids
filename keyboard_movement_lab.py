WIDTH = 700
HEIGHT = 400

lucas = Actor("lucas")
lucas.pos = (WIDTH/2, HEIGHT/2)

def draw():
    screen.clear()
    lucas.draw()

def update():
    if (keyboard.up or keyboard.w) and lucas.top >= 0:
        lucas.y -= 5
    elif (keyboard.down or keyboard.s) and lucas.bottom <= HEIGHT:
        lucas.y += 5

    if (keyboard.left or keyboard.a) and lucas.left >= 0:
        lucas.x -= 5
    elif (keyboard.right or keyboard.d) and lucas.right <= WIDTH:
        lucas.x += 5

