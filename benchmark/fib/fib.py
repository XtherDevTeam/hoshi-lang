def fib(n):
    if n <= 1:
        return n
    else:
        return fib(n-1) + fib(n-2)
    
if __name__ == '__main__':
    fib(40)