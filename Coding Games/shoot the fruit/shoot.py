# Chapter 3  Shoot the Fruit

from random import randint

apple = Actor("apple")


def draw():
    screen.clear()
    apple.draw()


def place_apple():
    apple.x = randint(10, 800)
    apple.y = randint(10, 600)


def on_mouse_down(pos):
    if apple.collidepoint(pos):
        print("good shot")
        place_apple()
    else:
        print("you missed")
        quit()
