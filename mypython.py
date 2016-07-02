import random
import string

for _ in range(3):
    temp_string = ''.join(random.choice(string.ascii_lowercase) for _ in range(10))
    f = open(temp_string + '.txt', 'w')
    f.write(temp_string)
    f.write('\n')
    f.close()
    print temp_string

num1 = random.randint(1, 43)
num2 = random.randint(1,43)

print num1, " * ", num2, " = ",  num1 * num2
