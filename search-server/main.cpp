// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь:
#include <iostream>
#include <string>

int main() {
	int numbers=0;
	for (i = 1; i <= 1000; i++) {
		if (i.itos().count('3')) {
			++numbers;
		}
}
	cout << numbers << endl;
	return 0;
}


// Закомитьте изменения и отправьте их в свой репозиторий.
