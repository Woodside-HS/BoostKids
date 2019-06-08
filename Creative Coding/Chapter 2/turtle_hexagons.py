# Creative coding chapter two step 4

import turtle

colors = ['red', 'yellow', 'blue', 'orange', 'green', 'red']

shelly = turtle.Turtle()

turtle.bgcolor('black') # turn background black

shelly.speed('fastest')

# draw 36 colored hexagons
for i in range(36):
    for i in range(6):
        shelly.color(colors[i])
        shelly.forward(100)
        shelly.left(60)
    #add a turn before the next hexagon
    shelly.right(10)

# draw 36 white circles
shelly.penup()
shelly.color('white')
for i in range(36):
    shelly.forward(220)
    shelly.pendown()
    shelly.circle(5)
    shelly.penup()
    shelly.backward(220)
    shelly.right(10)

shelly.hideturtle()

