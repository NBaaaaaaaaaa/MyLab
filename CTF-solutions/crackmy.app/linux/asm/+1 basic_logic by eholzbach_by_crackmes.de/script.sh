#!/bin/bash

# Выполняем команду ps и извлекаем PID процесса, который соответствует "logic"
PID=$(ps a | grep "./logic" | grep -v "grep" | grep "S+" | awk '{print $1}')
TIME_ADDR=0x8049504 
LENGTH=0x20         

# Проверяем, что процесс существует
if ! ps -p $PID > /dev/null; then
    echo "Процесс с PID $PID не найден."
    exit 1
fi

# Проверяем существование /proc/$PID/mem
if [[ ! -e "/proc/$PID/mem" ]]; then
    echo "Файл /proc/$PID/mem не существует."
    exit 1
fi

OFFSET=0            # Начальный смещение
PASSWORD="$PID"     # Строка пароля

# Заполняем PASSWORD результатом функции time 
while true; do
    # Считываем один байт из памяти по указанному адресу
    BYTE=$(xxd -p -s $(($TIME_ADDR + OFFSET)) -l 1 /proc/$PID/mem)

    # Проверяем на байт 0x00
    if [ $((16#$BYTE)) -eq 0 ]; then
        break
    fi

    # Добавляем цифру в строку 
    PASSWORD+="$((16#$BYTE - 0x30))"

    # Увеличиваем смещение
    OFFSET=$((OFFSET + 1))
done

echo "Correct password: $PASSWORD"
