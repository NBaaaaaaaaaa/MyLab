import random

# Определяем диапазоны ASCII-кодов
ranges = list(range(0x23, 0x2C)) + list(range(0x3C, 0x41))

# Функция генерации случайного символа
def random_char():
    return chr(random.choice(ranges))

# Функция генерации одной секции (7 случайных символов)
def generate_section():
    return ''.join(random_char() for _ in range(7))

# Генерируем полный ключ в формате xxxxxxx-xxxxxxx-xxxxxxx-xxxxxxx-xxxxxxx
key = '-'.join(generate_section() for _ in range(5))

print(key, end='')