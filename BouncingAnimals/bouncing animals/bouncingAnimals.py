

# set window size
WIDTH = 700
HEIGHT = 700

# create animal
animal = Actor('giraffe')
# set animal position
animal.pos = (200, 200)
# set animal horizontal (left & right) speed
speedx = 3
# set animal vertical (up & down) speed
speedy = 5

def update():
    # tell program that we are using speedx and speedy from above
    global speedx, speedy
    # move animal horizontally (left & right)
    animal.x += speedx
    # move animal vertically (up & down)
    animal.y += speedy
    # make animal bounce off screen edges
    if animal.x <= 0 or animal.x >= WIDTH:
        speedx *= -1
    if animal.y <= 0 or animal.y >= HEIGHT:
        speedy *= -1

def draw():
    # change background color from default black to blue (draws over previous screen)
    screen.fill((0, 255, 255))
    # draw animal
    animal.draw()

# use mouse position
def on_mouse_down(pos):
    if animal.collidepoint(pos):
    # check if mouse position is the same as the animal position
        # change from first animal image to second
        if animal.image == "giraffe":
            animal.image = "dog"
        # change from second animal image to first
        elif animal.image == "dog":
            animal.image = "giraffe"