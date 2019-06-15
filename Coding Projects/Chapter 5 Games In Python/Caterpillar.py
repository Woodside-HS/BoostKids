import random
import turtle

turtle.bgcolor("yellow")

caterpillar = turtle.Turtle()
caterpillar.shape("square")
caterpillar.color("red")
caterpillar.speed(0)
caterpillar.penup()
caterpillar.hideturtle()

leaf = turtle.Turtle()
leaf_shape = ((0, 0), (14, 2), (18, 6), (20, 20), (6, 18), (2, 14))
turtle.register_shape("leaf", leaf_shape)
leaf.shape("leaf")
leaf.color("green")
leaf.penup()
leaf.hideturtle()
leaf.speed(0)

game_started = False
text_turtle = turtle.Turtle()
text_turtle.write("Press SPACE to start", align="center", font=("Arial", 16, "bold"))
text_turtle.hideturtle()

score_turtle = turtle.Turtle()
score_turtle.hideturtle()
score_turtle.speed(0)

def outside_window():
    left_wall = -turtle.window_width() / 2
    right_wall = turtle.window_width() / 2
    top_wall = turtle.window_height() / 2
    bottom_wall = -turtle.window_height() / 2
    (x, y) = caterpillar.pos()
    outside = \
            x< left_wall or \
            x> right_wall or \
            y< bottom_wall or \
            y> top_wall
    return outside

def game_over():
    caterpillar.color("yellow")
    leaf.color("yellow")
    turtle.penup()
    turtle.hideturtle()
    turtle.write("GAME OVER!", align="center", font=("Arial", 30, "normal"))

def display_score(current_score):
    score_turtle.clear()
    score_turtle.penup()
    x = (turtle.window_width() / 2) - 50
    y = (turtle.window_height() / 2) - 50
    score_turtle.setpos(x, y)
    score_turtle.write(str(current_score), align="right", font=("Arial", 40, "bold"))

def place_leaf():
    leaf.ht()
    leaf.setx(random.randint(-200, 200))
    leaf.sety(random.randint(-200, 200))
    leaf.st()

def start_game():
    global game_started
    if game_started:
        return
    game_started = True

    score = 0
    text_turtle.clear()

    caterpillar_speed = 2
    caterpillar_length = 3
    caterpillar.shapesize(1, caterpillar_length, 1)
    caterpillar.showturtle()
    display_score(score)
    place_leaf()

    while True:
        caterpillar.forward(caterpillar_speed)
        if caterpillar.distance(leaf) < 20:
            place_leaf()
            caterpillar_length += 1
            caterpillar.shapesize(1, caterpillar_length, 1)
            caterpillar_speed += 1
            score += 10
            display_score(score)
        if outside_window:
            game_over()
            break

def move_up():
    if caterpillar.heading() == 0 or caterpillar.heading() == 180:
        caterpillar.setheading(90)

def move_down():
    if caterpillar.heading() == 0 or caterpillar.heading() == 180:
        caterpillar.setheading(270)

def move_left():
    if caterpillar.heading() == 90 or caterpillar.heading() == 270:
        caterpillar.setheading(180)

def move_right():
    if caterpillar.heading() == 90 or caterpillar.heading() == 270:
        caterpillar.setheading(0)

turtle.onkey(start_game, "space")
turtle.onkey(move_up, "Up")
turtle.onkey(move_right, "Right")
turtle.onkey(move_down, "Down")
turtle.onkey(move_left, "Left")
turtle.listen()
turtle.mainloop()
