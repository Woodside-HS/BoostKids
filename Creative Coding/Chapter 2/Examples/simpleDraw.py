# from turtle import *

import turtle
colors = ['red', 'yellow', 'blue', 'orange', 'green', 'red']

turt = turtle.Turtle()
turt.color('red')
turt.shape('turtle')

turt.pensize(5)

for i in range(4):
    beginfill()
    turt.forward(200)
    turt.left(90)
    endfill()




