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

def draw():
    screen.clear()
    for row in range(len(maze)):
        for col in range(len(maze[row])):
            x = col*TILE_SIZE
            y = row*TILE_SIZE
            tile = tiles[maze[row][col]]
            screen.blit(tile, (x, y))
    player.draw()