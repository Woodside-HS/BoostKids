import pygame
import time

TITLE = 'Pong'
WIDTH = 700
HEIGHT = 400

# CONSTANTS
PADDLE_SPEED = 4
BALL_SPEED = 4
WIN_AT = 3

# blue paddle
blue = Actor("paddleblu")
blue.angle = 90
blue.center = (blue.width/2, HEIGHT/2)

# red paddle
red = Actor("paddlered")
red.angle = 90
red.center = (WIDTH-red.width/2, HEIGHT/2)

# ball
ball = Actor("ballgrey")
ball.center = (WIDTH/2, HEIGHT/2)
ball.xspeed = BALL_SPEED
ball.yspeed = BALL_SPEED

# game variables
blue_score = 0
red_score = 0
game_running = True

# resets the positions of game objects
def reset_game():
    blue.center = (blue.width/2, HEIGHT/2)
    red.center = (WIDTH-red.width/2, HEIGHT/2)
    ball.center = (WIDTH/2, HEIGHT/2)

# tells the computer to clear the screen and draw each image to the screen
def draw():
    screen.clear()

    blue.draw()
    red.draw()
    ball.draw()

    display_score()

    if blue_score >= WIN_AT:
        display_blue_win()
    elif red_score >= WIN_AT:
        display_red_win()

def display_score():
    screen.draw.text("SCORE:\nBlue: " + str(blue_score) + "  Red: " + str(red_score), center=(WIDTH/2, 20))

# calls the update functions that move the objects
def update():
    global game_running
    global red_score, blue_score
    if game_running == True:
        update_paddles()
        update_ball()

        # checks for contact with the walls and adds to score
        if ball.right >= WIDTH:
            blue_score += 1
            display_score()
            time.sleep(2)
            reset_game()
        if ball.left <= 0:
            red_score += 1
            display_score()
            time.sleep(2)
            reset_game()

        # ends the game once someone passes the win condition
        if blue_score >= WIN_AT or red_score >= WIN_AT:
            game_running = False

def display_blue_win():
    screen.draw.text("BLUE WINS!!", center=(WIDTH/2, HEIGHT/2), fontsize=50)

def display_red_win():
    screen.draw.text("RED WINS!!", center=(WIDTH/2, HEIGHT/2), fontsize=50)

def update_paddles():

    # update blue paddle position based on keyboard w and s keys
    if keyboard.w and blue.top >= 0:
        blue.y -= PADDLE_SPEED
    elif keyboard.s and blue.bottom <= HEIGHT:
        blue.y += PADDLE_SPEED

    # update red paddle position based on keyboard up and down arrows
    if keyboard.up and red.top >= 0:
        red.y -= PADDLE_SPEED
    elif keyboard.down and red.bottom <= HEIGHT:
        red.y += PADDLE_SPEED

def update_ball():

    # move ball
    ball.y += ball.yspeed
    ball.x += ball.xspeed

    # screen bouncing
    if ball.top <= 0 or ball.bottom >= HEIGHT:
        ball.yspeed = -ball.yspeed

    # red paddle bouncing
    if ball.bottom >= red.top and ball.top <= red.bottom and ball.x >= red.left and ball.x <= red.right:
        ball.yspeed = -ball.yspeed
    elif ball.right >= red.left and ball.left <= red.right and ball.y >= red.top and ball.y <= red.bottom:
        ball.xspeed = -ball.xspeed

    # blue paddle bouncing
    if ball.bottom >= blue.top and ball.top <= blue.bottom and ball.x >= blue.left and ball.x <= blue.right:
        ball.yspeed = -ball.yspeed
    elif ball.right >= blue.left and ball.left <= blue.right and ball.y >= blue.top and ball.y <= blue.bottom:
        ball.xspeed = -ball.xspeed

# Lab 1:
# Display text

# screen.draw.text("Hello World, center=(WIDTH/2, HEIGHT/2), fontsize = 50)

# Lab 2:
# Pick any picture for your actor, display it in the center, and make it move using arrow keys

# WIDTH = 700
# HEIGHT = 400

# lucas = Actor("lucas")
# lucas.pos = (WIDTH/2, HEIGHT/2)

# def draw():
#     screen.clear()
#     lucas.draw()

# def update():
#     if keyboard.up:
#         lucas.y -= 5
#     elif keyboard.down:
#         lucas.y += 5

#     if keyboard.left:
#         lucas.x -= 5
#     elif keyboard.right:
#         lucas.x += 5

# Lab 3:
# Pick any picture for your actor, make it move and bounce off the walls
# Then add another actor somewhere and make the first actor bounce off the second on all 4 sides

# WIDTH = 700
# HEIGHT = 400

# first = Actor("lucas")
# first.topleft = (1,1)
# first.xspeed = 5
# first.yspeed = 5

# second = Actor("lucas")
# second.pos = (WIDTH/2, HEIGHT/2)

# def draw():
#     screen.clear()

#     first.draw()
#     second.draw()

# def update():
#     first.x += first.xspeed
#     first.y += first.yspeed

#     if first.right >= WIDTH:
#         first.xspeed = -5
#     if first.left <= 0:
#         first.xspeed = 5
#     if first.top <= 0:
#         first.yspeed = 5
#     if first.bottom >= HEIGHT:
#         first.yspeed = -5

#     if first.bottom >= second.top and first.bottom <= second.bottom and first.x >= second.left and first.x <= second.right:
#         first.yspeed = -5
#     elif first.top <= second.bottom and first.top >= second.bottom and first.x >= second.left and first.x <= second.right:
#         first.yspeed = 5
#     if first.right >= second.left and first.right <= second.right and first.y >= second.top and first.y <= second.bottom:
#         first.xspeed = -5
#     elif first.left <= second.right and first.left >= second.left and first.y >= second.top and first.y <= second.bottom:
#         first.xspeed = 5


# LINK TO FILES USED: https://www.kenney.nl/assets/puzzle-pack
# remember to rename files to lowercase for Mu compatibility