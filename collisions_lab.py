WIDTH = 800
HEIGHT = 800

# creating an actor and putting it in the top left corner
first = Actor("horse")
first.pos = (first.width/2,first.height/2)

# creating variables for the speed of the bouncing actor
xspeed = 5
yspeed = 5

second = Actor("giraffe")
# putting second image in the center (half the width/height)
second.pos = (WIDTH/2, HEIGHT/2)

# draw function to display the images on the screen
def draw():
    screen.clear()

    first.draw()
    second.draw()

# called 60 times per second to change the positions of the actors and handle the bouncing behavior
def update():
    global xspeed, yspeed

    # bounce if x position is off screen
    if first.right + xspeed >= WIDTH or first.left + xspeed <= 0:
        xspeed = -xspeed
    # bounce if the y position is off the screen
    if first.bottom + yspeed >= HEIGHT or first.top + yspeed <= 0:
        yspeed = -yspeed

    # bounce if first actor hits the second actor on the sides
    if (first.right + xspeed) >= second.left and (first.left + xspeed) <= second.right and first.top <= second.bottom and first.bottom >= second.top:
        xspeed = -xspeed

    # bounce if first actor hits the second actor on the top or bottom
    if (first.bottom + yspeed) >= second.top and (first.top + yspeed) <= second.bottom and first.right >= second.left and first.left <= second.right:
        yspeed = -yspeed

    # updates the position of the images according to the speed variables
    first.x += xspeed
    first.y += yspeed