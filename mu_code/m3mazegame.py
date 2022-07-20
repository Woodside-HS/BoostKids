#Amazing Maze
TILE_SIZE = 140
WIDTH = TILE_SIZE*6
HEIGHT = TILE_SIZE*6

tiles = ['zebra', 'panda', 'parrot']
maze = [
        [1, 1, 1, 1, 1, 1],
        [1, 0, 0, 0, 1, 2 ],
        [1, 0, 0, 0, 1, 2 ],
        [0, 0, 0, 0, 1, 2],
        [1, 2, 2, 0, 1, 2],
        [1, 0, 0, 0, 1, 2],

]

player = Actor("pig", anchor = (0,0), pos = (1*TILE_SIZE, 1*TILE_SIZE))
enemy = Actor("sloth", anchor=(0, 0), pos=(3 * TILE_SIZE, 6 * TILE_SIZE))
enemy.yv = -1

def draw():
    screen.clear()
    for row in range(len(maze)):
        for col in range(len(maze[row])):
            x = col*TILE_SIZE
            y = row*TILE_SIZE
            tile = tiles[maze[row][col]]
            screen.blit(tile, (x, y))
    player.draw()
    enemy.draw()

def on_key_down(key):
    row = int(player.y / TILE_SIZE)
    column = int(player.x / TILE_SIZE)
    if key == keys.UP:
        row = row - 1
    if key == keys.DOWN:
        row = row + 1
    if key == keys.LEFT:
        column = column - 1
    if key == keys.RIGHT:
        column = column + 1
    tile = tiles[maze[row][column]]
    if tile == 'empty':
        x = column * TILE_SIZE
        y = row * TILE_SIZE
        animate(player, duration=0.1, pos=(x, y))
    elif tile == 'goal':
        print("Well done")
        exit()

    # enemy movement
    row = int(enemy.y / TILE_SIZE)
    column = int(enemy.x / TILE_SIZE)
    row = row + enemy.yv
    tile = tiles[maze[row][column]]
    if not tile == 'wall':
        x = column * TILE_SIZE
        y = row * TILE_SIZE
        animate(enemy, duration=0.1, pos=(x, y))
    else:
        enemy.yv = enemy.yv * -1

    if enemy.colliderect(player):
        print("You died")
        exit()
