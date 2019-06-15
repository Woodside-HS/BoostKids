import turtle
from random import randint, random

def draw_star(points, size, col, x, y):
    turtle.penup()
    turtle.goto(x, y)
    turtle.pendown
    angle = 180 - (180 / points)
    turtle.color(col)
    turtle.begin_fill()
    for i in range(points):
        turtle.forward(size)
        turtle.right(angle)
    turtle.end_fill()

#Main code
turtle.Screen().bgcolor("dark blue")

while True:
    ranPts = randint(2, 5) * 2 + 1
    ranSize = randint(10, 50)
    ranCol = (random(), random(), random())
    ranX = randint(-350, 300)
    ranY = randint(-250, 250)

    draw_star(ranPts, ranSize, ranCol, ranX, ranY) 
