import pygame
import sys
import random
import math

# 初始化 Pygame
pygame.init()

# 屏幕设置
WIDTH, HEIGHT = 800, 600
SCREEN = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("圆环射击游戏")

# 颜色定义
WHITE = (255, 255, 255)
BLUE = (0, 0, 255)
RED = (255, 0, 0)
BLACK = (0, 0, 0)
YELLOW = (255, 255, 0)

# 时钟
CLOCK = pygame.time.Clock()
FPS = 60

# 字体
FONT = pygame.font.SysFont("arial", 30)

# 游戏状态
MENU, PLAY, GAME_OVER = "menu", "play", "game_over"

# 难度设置
DIFFICULTIES = {
    "Easy": 1,
    "Medium": 2,
    "Hard": 3
}

# 圆环参数
RING_RADIUS = 200
RING_CENTER = (WIDTH // 2, HEIGHT // 2)

# 玩家参数
PLAYER_RADIUS = 15
PLAYER_COLOR = BLUE
PLAYER_SPEED = 2  # 旋转速度

# 子弹参数
BULLET_COLOR = (0, 255, 0)
BULLET_SPEED = 5
BULLET_SIZE = 10

# 敌人参数
ENEMY_SIZE = 30
ENEMY_COLOR = RED
ENEMY_SPEED_BASE = 1

# 爆炸参数
EXPLOSION_DURATION = 30  # 帧数

# 定义玩家类
class Player:
    def __init__(self):
        self.angle = 0  # 以度为单位
        self.direction = 1  # 1 为顺时针，-1 为逆时针

    def update(self):
        self.angle = (self.angle + PLAYER_SPEED * self.direction) % 360

    def draw(self, surface):
        x = RING_CENTER[0] + RING_RADIUS * math.cos(math.radians(self.angle))
        y = RING_CENTER[1] + RING_RADIUS * math.sin(math.radians(self.angle))
        pygame.draw.circle(surface, PLAYER_COLOR, (int(x), int(y)), PLAYER_RADIUS)

    def shoot(self):
        angle_rad = math.radians(self.angle)
        x = RING_CENTER[0] + RING_RADIUS * math.cos(angle_rad)
        y = RING_CENTER[1] + RING_RADIUS * math.sin(angle_rad)
        # 子弹初始位置在玩家位置
        bullet = Bullet(x, y, self.angle)
        return bullet

# 定义子弹类
class Bullet:
    def __init__(self, x, y, angle):
        self.x = x
        self.y = y
        self.angle = angle
        # 计算速度分量
        self.dx = BULLET_SPEED * math.cos(math.radians(self.angle))
        self.dy = BULLET_SPEED * math.sin(math.radians(self.angle))
        # 定义三角形子弹的顶点
        self.size = BULLET_SIZE

    def update(self):
        self.x += self.dx
        self.y += self.dy

    def draw(self, surface):
        # 计算三角形的三个顶点
        point1 = (self.x + self.size * math.cos(math.radians(self.angle)),
                  self.y + self.size * math.sin(math.radians(self.angle)))
        angle_left = self.angle + 135
        angle_right = self.angle - 135
        point2 = (self.x + self.size * 0.5 * math.cos(math.radians(angle_left)),
                  self.y + self.size * 0.5 * math.sin(math.radians(angle_left)))
        point3 = (self.x + self.size * 0.5 * math.cos(math.radians(angle_right)),
                  self.y + self.size * 0.5 * math.sin(math.radians(angle_right)))
        pygame.draw.polygon(surface, BULLET_COLOR, [point1, point2, point3])

    def off_screen(self):
        return not (0 <= self.x <= WIDTH and 0 <= self.y <= HEIGHT)

    def get_rect(self):
        return pygame.Rect(self.x - self.size, self.y - self.size, self.size*2, self.size*2)

# 定义敌人类
class Enemy:
    def __init__(self, speed):
        # 随机选择一个边缘位置
        self.size = ENEMY_SIZE
        self.speed = speed
        side = random.choice(['top', 'bottom', 'left', 'right'])
        if side == 'top':
            self.x = random.randint(0, WIDTH)
            self.y = -self.size
        elif side == 'bottom':
            self.x = random.randint(0, WIDTH)
            self.y = HEIGHT + self.size
        elif side == 'left':
            self.x = -self.size
            self.y = random.randint(0, HEIGHT)
        else:  # right
            self.x = WIDTH + self.size
            self.y = random.randint(0, HEIGHT)
        # 计算移动方向指向圆环中心
        dx = RING_CENTER[0] - self.x
        dy = RING_CENTER[1] - self.y
        distance = math.hypot(dx, dy)
        self.dx = (dx / distance) * self.speed
        self.dy = (dy / distance) * self.speed
        self.rect = pygame.Rect(self.x - self.size//2, self.y - self.size//2, self.size, self.size)

    def update(self):
        self.x += self.dx
        self.y += self.dy
        self.rect.x = self.x - self.size//2
        self.rect.y = self.y - self.size//2

    def draw(self, surface):
        pygame.draw.rect(surface, ENEMY_COLOR, self.rect)

    def reached_ring(self):
        # 检查敌人是否接近圆环
        distance = math.hypot(self.x - RING_CENTER[0], self.y - RING_CENTER[1])
        return distance <= RING_RADIUS + self.size//2

# 定义爆炸效果类
class Explosion:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.frame = 0

    def update(self):
        self.frame += 1

    def draw(self, surface):
        # 简单的扩展圆圈作为爆炸效果
        radius = self.frame * 2
        if radius > 50:
            radius = 50
        pygame.draw.circle(surface, YELLOW, (int(self.x), int(self.y)), radius, 2)

    def is_finished(self):
        return self.frame > EXPLOSION_DURATION

# 菜单界面
def draw_menu(surface, selected_difficulty):
    surface.fill(BLACK)
    title_text = FONT.render("圆环射击游戏", True, WHITE)
    surface.blit(title_text, (WIDTH//2 - title_text.get_width()//2, 100))

    start_text = FONT.render("按空格键开始游戏", True, WHITE)
    surface.blit(start_text, (WIDTH//2 - start_text.get_width()//2, 200))

    difficulty_text = FONT.render("选择难度:", True, WHITE)
    surface.blit(difficulty_text, (WIDTH//2 - difficulty_text.get_width()//2, 300))

    for idx, (diff, _) in enumerate(DIFFICULTIES.items()):
        color = YELLOW if diff == selected_difficulty else WHITE
        diff_text = FONT.render(diff, True, color)
        surface.blit(diff_text, (WIDTH//2 - diff_text.get_width()//2, 350 + idx * 40))

    pygame.display.flip()

# 游戏结束界面
def draw_game_over(surface, score):
    surface.fill(BLACK)
    over_text = FONT.render("游戏失败!", True, WHITE)
    surface.blit(over_text, (WIDTH//2 - over_text.get_width()//2, 200))

    score_text = FONT.render(f"击败敌人数量: {score}", True, WHITE)
    surface.blit(score_text, (WIDTH//2 - score_text.get_width()//2, 300))

    retry_text = FONT.render("按R键重新开始 或 按Q键退出", True, WHITE)
    surface.blit(retry_text, (WIDTH//2 - retry_text.get_width()//2, 400))

    pygame.display.flip()

# 主游戏函数
def main():
    state = MENU
    selected_difficulty = "Easy"
    difficulty_level = DIFFICULTIES[selected_difficulty]

    player = Player()
    bullets = []
    enemies = []
    explosions = []
    enemy_spawn_timer = 0
    score = 0

    while True:
        CLOCK.tick(FPS)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()

            if state == MENU:
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_SPACE:
                        state = PLAY
                        # 初始化游戏变量
                        player = Player()
                        bullets = []
                        enemies = []
                        explosions = []
                        enemy_spawn_timer = 0
                        score = 0
                        difficulty_level = DIFFICULTIES[selected_difficulty]
                    elif event.key == pygame.K_UP or event.key == pygame.K_DOWN:
                        # 更改难度选择
                        keys = list(DIFFICULTIES.keys())
                        current_index = keys.index(selected_difficulty)
                        if event.key == pygame.K_UP:
                            selected_difficulty = keys[(current_index - 1) % len(keys)]
                        else:
                            selected_difficulty = keys[(current_index + 1) % len(keys)]

            elif state == PLAY:
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_LEFT:
                        player.direction = -1
                    elif event.key == pygame.K_RIGHT:
                        player.direction = 1
                    elif event.key == pygame.K_SPACE:
                        bullets.append(player.shoot())

            elif state == GAME_OVER:
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_r:
                        state = MENU
                        selected_difficulty = "Easy"
                    elif event.key == pygame.K_q:
                        pygame.quit()
                        sys.exit()

        if state == MENU:
            draw_menu(SCREEN, selected_difficulty)

        elif state == PLAY:
            SCREEN.fill(BLACK)
            # 绘制圆环
            pygame.draw.circle(SCREEN, WHITE, RING_CENTER, RING_RADIUS, 2)

            # 更新和绘制玩家
            player.update()
            player.draw(SCREEN)

            # 更新和绘制子弹
            for bullet in bullets[:]:
                bullet.update()
                bullet.draw(SCREEN)
                if bullet.off_screen():
                    bullets.remove(bullet)

            # 生成敌人
            enemy_spawn_timer += 1
            spawn_delay = max(60 // difficulty_level, 15)  # 难度越高，生成越快
            if enemy_spawn_timer >= spawn_delay:
                enemy_spawn_timer = 0
                enemies.append(Enemy(ENEMY_SPEED_BASE + difficulty_level * 0.5))

            # 更新和绘制敌人
            for enemy in enemies[:]:
                enemy.update()
                enemy.draw(SCREEN)
                if enemy.reached_ring():
                    state = GAME_OVER
                # 检查与子弹的碰撞
                for bullet in bullets[:]:
                    if enemy.rect.colliderect(bullet.get_rect()):
                        explosions.append(Explosion(enemy.x, enemy.y))
                        enemies.remove(enemy)
                        bullets.remove(bullet)
                        score += 1
                        break

            # 更新和绘制爆炸效果
            for explosion in explosions[:]:
                explosion.update()
                explosion.draw(SCREEN)
                if explosion.is_finished():
                    explosions.remove(explosion)

            # 显示得分
            score_text = FONT.render(f"得分: {score}", True, WHITE)
            SCREEN.blit(score_text, (10, 10))

            pygame.display.flip()

        elif state == GAME_OVER:
            draw_game_over(SCREEN, score)

# 运行游戏
if __name__ == "__main__":
    main()
