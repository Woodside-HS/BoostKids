# Chapter 5 Follow the Numbers

from random import randint


WIDTH = 400
HEIGHT = 400

dots = []
lines = []

next_dot = 0
num_dots = 10

for dot in range(0, num_dots):
    actor = Actor("dot")
    actor.pos = randint(20, WIDTH-20),randint(20, HEIGHT-20)
    dots.append(actor)

def draw():
    screen.fill("black")
    number = 1
    for dot in dots:
        screen.draw.text(str(number), (dot.pos[0],dot.pos[1] + 12))
        dot.draw()
        number = number + 1

    for line in lines:
        screen.draw.line(line[0], line[1], (100, 0, 0))

def on_mouse_down(pos):
    global next_dot, lines
    if next_dot < num_dots and dots[next_dot].collidepoint(pos):
        if next_dot:
            lines.append((dots[next_dot-1].pos, dots[next_dot].pos))
        next_dot = next_dot + 1
    else:
        lines = []
        next_dot = 0

