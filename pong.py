import pygame
import time

TITLE = 'Pong'
WIDTH = 700
HEIGHT = 400

# CONSTANTS
PADDLE_SPEED = 3
BALL_SPEED = 4
WIN_AT = 3

# blue paddle
blue = Actor("paddleblu")
blue.angle = 90
blue.center = (blue.width/2 + 50, HEIGHT/2)

# red paddle
red = Actor("paddlered")
red.angle = 90
red.center = (WIDTH-red.width/2 - 50, HEIGHT/2)

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
    blue.center = (blue.width/2 + 50, HEIGHT/2)
    red.center = (WIDTH-red.width/2 - 50, HEIGHT/2)
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
            time.sleep(2)
            reset_game()
        if ball.left <= 0:
            red_score += 1
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

    # screen bouncing
    if (ball.top + ball.yspeed) <= 0 or (ball.bottom + ball.yspeed) >= HEIGHT:
        ball.yspeed = -ball.yspeed

    # if (ball.right + ball.xspeed) >= WIDTH or (ball.left + ball.xspeed) <= 0:
#         ball.xspeed = -ball.xspeed

    # red paddle bouncing
    if (ball.bottom + ball.yspeed) >= red.top and (ball.top + ball.yspeed) <= red.bottom and ball.x >= red.left and ball.x <= red.right:
        ball.yspeed = -ball.yspeed
    if (ball.right + ball.xspeed) >= red.left and (ball.left + ball.xspeed) <= red.right and ball.y >= red.top and ball.y <= red.bottom:
        ball.xspeed = -ball.xspeed

    # blue paddle bouncing
    if (ball.bottom + ball.yspeed) >= blue.top and (ball.top + ball.yspeed) <= blue.bottom and ball.x >= blue.left and ball.x <= blue.right:
        ball.yspeed = -ball.yspeed
    if (ball.right + ball.xspeed) >= blue.left and(ball.left + ball.xspeed) <= blue.right and ball.y >= blue.top and ball.y <= blue.bottom:
        ball.xspeed = -ball.xspeed

    # move ball
    ball.y += ball.yspeed
    ball.x += ball.xspeed


# LINK TO FILES USED: https://www.kenney.nl/assets/puzzle-pack
# remember to rename files to lowercase for Mu compatibility