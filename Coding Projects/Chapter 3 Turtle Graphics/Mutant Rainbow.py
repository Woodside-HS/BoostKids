import random
import turtle

def get_line_length():
    choice = input("Enter line length (long, medium, short): ")
    if choice == "long":
        line_length = 250
    elif choice == "medium":
        line_length = 200
    else:
        line_length = 100
    return line_length

def get_line_width():
    choice = input("Enter line width (superthick, thick, thin): ")
    if choice == "superthick":
        line_width = 40
    elif choice == "thick":
        line_width = 25
    else:
        line_width = 10
    return line_width

def inside_window():
    left_limit = (-turtle.window_width() / 2) + 100
    right_limit = (turtle.window_width() / 2) - 100
    upper_limit = (turtle.window_height() / 2) - 100
    lower_limit = (-turtle.window_height() / 2) + 100
    (x, y) = turtle.pos()
    inside = left_limit < x < right_limit and lower_limit < y < upper_limit
    return inside

def move_turtle(line_length):
    pen_colors = ["red", "orange", "yellow", "green", "blue", "purple"]
    turtle.pencolor(random.choice(pen_colors))
    if inside_window():
        angle = random.randint(0, 180)
        turtle.right(angle)
        turtle.forward(line_length)
    else:
        turtle.backward(line_length)

line_length = get_line_length()
line_width = get_line_width()

turtle.shape("turtle")
turtle.fillcolor("green")
turtle.bgcolor("black")
turtle.speed("fastest")
turtle.pensize(line_width)

while True:
    move_turtle(line_length)

