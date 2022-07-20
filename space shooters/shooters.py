
#window settings
TITLE = "Shooters"
WIDTH = 1200
HEIGHT = 600
#game settings
SHIP_SPEED = 3
LASER_SPEED = 14
COOLDOWN = 0.3
LIVES = 5
#allows the 2 ships to have separate cooldowns
left_cd = COOLDOWN
right_cd = COOLDOWN
running = True

#create player 1 (left side)
player_left = Actor("playership1_red.png")
player_left.angle = 270
player_left.center = (100, HEIGHT/2)
#list to hold/track left ship's lasers
lasers_left = []
left_life = LIVES
#set life tracker
left_icon = Actor("playerlife1_red.png")
left_icon.topleft = (5,5)

#right player settings (same as left, but flipped)
player_right = Actor("playership2_blue.png")
player_right.angle = 90
player_right.center = (WIDTH-100, HEIGHT/2)
lasers_right = []
right_life = LIVES
right_icon = Actor("playerlife2_blue.png")
right_icon.topright = (WIDTH-25, 5)

#dedicated function to ship mobility
def move_ships():

    if keyboard.w and player_left.top >= 0:
        player_left.y -= SHIP_SPEED
    elif keyboard.s and player_left.bottom <= 1000:
        player_left.y += SHIP_SPEED

    if keyboard.up and player_right.top >= 0:
        player_right.y -= SHIP_SPEED
    elif keyboard.down and player_right.bottom <= 1000:
        player_right.y += SHIP_SPEED

def draw():
    screen.clear()
    #background; can swap with another, but make sure it is at least the size of the canvas
    #screen.blit("space.png", (0,0))
    #draw all the sprites
    player_left.draw()
    left_icon.draw()
    player_right.draw()
    right_icon.draw()
    for shot in lasers_left:
        shot.draw()
    for shot in lasers_right:
        shot.draw()

    #draw the life trackers
    screen.draw.text(":"+ str(left_life),
        (left_icon.right,10), fontsize = 30)
    screen.draw.text(":"+ str(right_life),
        (right_icon.right,10), fontsize = 30)
    #if the game is over
    if(not running):
        screen.draw.text(("GAME OVER"), center=(WIDTH/2, HEIGHT/2-50), fontsize = 150)
        if(left_life>right_life):
            screen.draw.text(("LEFT PLAYER WINS"),
                center=(WIDTH/2, HEIGHT/2+50), fontsize = 120, color = "red")
        elif(right_life>left_life):
            screen.draw.text(("RIGHT PLAYER WINS"),
                center=(WIDTH/2, HEIGHT/2+50), fontsize = 120, color = "blue")
        else:
            screen.draw.text(("Wait, you tied? That can happen?!!?"),
                center=(WIDTH/2, HEIGHT/2+50), fontsize = 90)


def update(dt):
    global left_cd
    global right_cd
    global left_life
    global right_life
    global running

    #check if the game should end
    if(left_life == 0 or right_life==0):
        running = False
        #delete the currently active lasers
        for shot in lasers_left:
            lasers_left.remove(shot)
        for shot in lasers_right:
            lasers_right.remove(shot)

    #if the game has not ended
    if(running):
        move_ships()

        #left shoot
        if(keyboard.d):
            if(left_cd >= COOLDOWN):
                #reset cooldown
                left_cd = 0
                #make a laser actor and append it to the list
                laser = Actor("laserred03.png")
                laser.angle = 270
                laser.x = player_left.right
                laser.y = player_left.y
                lasers_left.append(laser)

        #update existing left lasers
        for shot in lasers_left:
            if shot.colliderect(player_right):
                right_life -= 1
                lasers_left.remove(shot)
            elif shot.right == WIDTH:
                lasers_left.remove(shot)
            else:
                shot.x += LASER_SPEED

        #right laser shooting/updating
        if(keyboard.left):
            if(right_cd >= COOLDOWN):
                right_cd = 0
                laser = Actor("laserblue03.png")
                laser.angle = 90
                laser.x = player_right.left
                laser.y = player_right.y
                lasers_right.append(laser)

        for shot in lasers_right:
            if(shot.colliderect(player_left)):
                left_life -= 1
                lasers_right.remove(shot)
            elif shot.left == WIDTH:
                lasers_right.remove(shot)
            else:
                shot.x -= LASER_SPEED
        #update laser cooldowns
        right_cd += dt
        left_cd += dt