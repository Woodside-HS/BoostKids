# from turtle import *

import turtle
colors = ['red', 'yellow', 'blue', 'orange', 'green', 'red']

turt = turtle.Turtle()
turtle.bgcolor('black')


for i in range(500): # this "for" loop will repeat these functions 500 times
    turt.color(colors[i%6])
    turt.pensize(5)
    turt.forward(i+100)
    turt.left(95)
