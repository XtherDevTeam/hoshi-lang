def loop():
    x = 1.0
    i = 0
    while i < 99999999:
        x = (i + i + 2 * i + 1 - 0.379) / x
        i += 1
    return x

if __name__ == '__main__':
    print(loop())