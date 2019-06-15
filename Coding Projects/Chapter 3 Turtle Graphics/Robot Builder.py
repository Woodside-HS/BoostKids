import turtle

def rectangle(horizontal, vertical, color):
    turtle.pendown()
    turtle.pensize(1)
    turtle.color(color)
    turtle.begin_fill()
    for counter in range(1, 3):
        turtle.forward(horizontal)
        turtle.right(90)
        turtle.forward(vertical)
        turtle.right(90)
    turtle.end_fill()
    turtle.penup()

turtle.penup()
turtle.speed("slow")
turtle.bgcolor("Dodger blue")

#feet
turtle.goto(-100, -150)
rectangle(50, 20, "blue")
turtle.goto(-30, -150)
rectangle(50, 20, "blue") 

#legs
turtle.goto(-25, -50)
rectangle(15, 100, "grey")
turtle.goto(-55, -50)
rectangle(-15, 100, "grey") 

#body
turtle.goto(-90, 100)
rectangle(100, 150, "red")

#arms
turtle.goto(-150, 70)
rectangle(60, 15, "grey")
turtle.goto(-150, 110)
rectangle(15, 40, "grey")

turtle.goto(10, 70)
rectangle(60, 15, "grey")
turtle.goto(55, 110)
rectangle(15, 40, "grey") 

#neck
turtle.goto(-50, 120)
rectangle(15, 20, "grey") 

#head
turtle.goto(-85, 170)
rectangle(80, 50, "red") 

#eyes
turtle.goto(-60, 160)
rectangle(30, 10, "white")
turtle.goto(-55, 155)
rectangle(5, 5, "black")
turtle.goto(-40, 155)
rectangle(5, 5, "black") 

#mouth
turtle.goto(-65, 135)
rectangle(40, 5, "black")

turtle.hideturtle()
